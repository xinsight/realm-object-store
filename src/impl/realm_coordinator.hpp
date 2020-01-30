////////////////////////////////////////////////////////////////////////////
//
// Copyright 2015 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#ifndef REALM_COORDINATOR_HPP
#define REALM_COORDINATOR_HPP

#include "shared_realm.hpp"

#include <realm/version_id.hpp>

#include <condition_variable>
#include <mutex>

namespace realm {
class DB;
class Replication;
class Schema;
class StringData;
class SyncSession;
class Transaction;

namespace _impl {
class CollectionNotifier;
class ExternalCommitHelper;
class WeakRealmNotifier;

namespace partial_sync {
class WorkQueue;
}

// RealmCoordinator manages the weak cache of Realm instances and communication
// between per-thread Realm instances for a given file
class RealmCoordinator : public std::enable_shared_from_this<RealmCoordinator> {
public:
    // Get the coordinator for the given path, creating it if neccesary
    static std::shared_ptr<RealmCoordinator> get_coordinator(StringData path);
    // Get the coordinator for the given config, creating it if neccesary
    static std::shared_ptr<RealmCoordinator> get_coordinator(const Realm::Config&);
    // Get the coordinator for the given path, or null if there is none
    static std::shared_ptr<RealmCoordinator> get_existing_coordinator(StringData path);

    // Get a shared Realm with the given configuration
    // If the Realm is already opened on another thread, validate that the given
    // configuration is compatible with the existing one.
    // If no version is provided a live thread-confined Realm is returned.
    // Otherwise, a frozen Realm at the given version is returned. This
    // can be read from any thread.
    std::shared_ptr<Realm> get_realm(Realm::Config config, util::Optional<VersionID> version);
    std::shared_ptr<Realm> get_realm();
#if REALM_ENABLE_SYNC
    // Get a thread-local shared Realm with the given configuration
    // If the Realm is not already present, it will be fully downloaded before being returned.
    // If the Realm is already on disk, it will be fully synchronized before being returned.
    // Timeouts and interruptions are not handled by this method and must be handled by upper layers.
    std::shared_ptr<AsyncOpenTask> get_synchronized_realm(Realm::Config config);
    // Used from SyncSession constructor if config.validate_sync_history is set
    void open_with_config(Realm::Config config);

    // Creates the underlying sync session if it doesn't already exists.
    // This is also created as part of opening a Realm, so only use this
    // method if the session needs to exist before the Realm does.
    void create_session(const Realm::Config& config);
#endif

    // Get a Realm which is not bound to the current execution context
    ThreadSafeReference get_unbound_realm();

    // Bind an unbound Realm to a specific execution context. The Realm must
    // be managed by this coordinator.
    void bind_to_context(Realm& realm, AnyExecutionContextID);

    Realm::Config get_config() const { return m_config; }

    uint64_t get_schema_version() const noexcept { return m_schema_version; }
    const std::string& get_path() const noexcept { return m_config.path; }
    const std::vector<char>& get_encryption_key() const noexcept { return m_config.encryption_key; }
    bool is_in_memory() const noexcept { return m_config.in_memory; }
    // Returns the number of versions in the Realm file.
    uint_fast64_t get_number_of_versions() const { return m_db->get_number_of_versions(); };

    // To avoid having to re-read and validate the file's schema every time a
    // new read transaction is begun, RealmCoordinator maintains a cache of the
    // most recently seen file schema and the range of transaction versions
    // which it applies to. Note that this schema may not be identical to that
    // of any Realm instances managed by this coordinator, as individual Realms
    // may only be using a subset of it.

    // Get the latest cached schema and the transaction version which it applies
    // to. Returns false if there is no cached schema.
    bool get_cached_schema(Schema& schema, uint64_t& schema_version, uint64_t& transaction) const noexcept;

    // Cache the state of the schema at the given transaction version
    void cache_schema(Schema const& new_schema, uint64_t new_schema_version,
                      uint64_t transaction_version);
    // If there is a schema cached for transaction version `previous`, report
    // that it is still valid at transaction version `next`
    void advance_schema_cache(uint64_t previous, uint64_t next);
    void clear_schema_cache_and_set_schema_version(uint64_t new_schema_version);


    // Asynchronously call notify() on every Realm instance for this coordinator's
    // path, including those in other processes
    void send_commit_notifications(Realm&);
    void wake_up_notifier_worker();

    // Clear the weak Realm cache for all paths
    // Should only be called in test code, as continuing to use the previously
    // cached instances will have odd results
    static void clear_cache();

    // Clears all caches on existing coordinators
    static void clear_all_caches();

    // Verify that there are no Realms open for any paths
    static void assert_no_open_realms() noexcept;

    // Explicit constructor/destructor needed for the unique_ptrs to forward-declared types
    RealmCoordinator();
    ~RealmCoordinator();

    // Called by Realm's destructor to ensure the cache is cleaned up promptly
    // Do not call directly
    void unregister_realm(Realm* realm);

    // Called by m_notifier when there's a new commit to send notifications for
    void on_change();

    static void register_notifier(std::shared_ptr<CollectionNotifier> notifier);

    std::shared_ptr<Group> begin_read(VersionID version={}, bool frozen_transaction = false);

    // Check if advance_to_ready() would actually advance the Realm's read version
    bool can_advance(Realm& realm);

    // Advance the Realm to the most recent transaction version which all async
    // work is complete for
    void advance_to_ready(Realm& realm);

    // Advance the Realm to the most recent transaction version, blocking if
    // async notifiers are not yet ready for that version
    // returns whether it actually changed the version
    bool advance_to_latest(Realm& realm);

    // Deliver any notifications which are ready for the Realm's version
    void process_available_async(Realm& realm);

    // Register a function which is called whenever sync makes a write to the Realm
    void set_transaction_callback(std::function<void(VersionID, VersionID)>);

    // Deliver notifications for the Realm, blocking if some aren't ready yet
    // The calling Realm must be in a write transaction
    void promote_to_write(Realm& realm);

    // Commit a Realm's current write transaction and send notifications to all
    // other Realm instances for that path, including in other processes
    void commit_write(Realm& realm);

    void enable_wait_for_change();
    bool wait_for_change(std::shared_ptr<Transaction> tr);
    void wait_for_change_release();

    void close();
    bool compact();

    template<typename Pred>
    std::unique_lock<std::mutex> wait_for_notifiers(Pred&& wait_predicate);

#if REALM_ENABLE_SYNC
    // A work queue that can be used to perform background work related to partial sync.
    _impl::partial_sync::WorkQueue& partial_sync_work_queue();
#endif

    AuditInterface* audit_context() const noexcept { return m_audit_context.get(); }

private:
    friend Realm::Internal;
    Realm::Config m_config;
    std::unique_ptr<Replication> m_history;
    std::shared_ptr<DB> m_db;
    std::shared_ptr<Group> m_read_only_group;

    mutable std::mutex m_schema_cache_mutex;
    util::Optional<Schema> m_cached_schema;
    uint64_t m_schema_version = -1;
    uint64_t m_schema_transaction_version_min = 0;
    uint64_t m_schema_transaction_version_max = 0;

    std::mutex m_realm_mutex;
    std::vector<WeakRealmNotifier> m_weak_realm_notifiers;

    std::mutex m_notifier_mutex;
    std::condition_variable m_notifier_cv;
    std::vector<std::shared_ptr<_impl::CollectionNotifier>> m_new_notifiers;
    std::vector<std::shared_ptr<_impl::CollectionNotifier>> m_notifiers;
    VersionID m_notifier_skip_version = {0, 0};

    // Transaction used for actually running async notifiers
    // Will have a read transaction iff m_notifiers is non-empty
    std::shared_ptr<Transaction> m_notifier_sg;

    // Transaction used to advance notifiers in m_new_notifiers to the main shared
    // group's transaction version
    // Will have a read transaction iff m_new_notifiers is non-empty
    std::shared_ptr<Transaction> m_advancer_sg;
    std::exception_ptr m_async_error;

    std::unique_ptr<_impl::ExternalCommitHelper> m_notifier;
    std::mutex m_transaction_callback_mutex;
    std::function<void(VersionID, VersionID)> m_transaction_callback;

#if REALM_ENABLE_SYNC
    std::shared_ptr<SyncSession> m_sync_session;
    std::unique_ptr<partial_sync::WorkQueue> m_partial_sync_work_queue;
#endif

    std::shared_ptr<AuditInterface> m_audit_context;

    void open_db();

    // must be called with m_notifier_mutex locked
    void pin_version(VersionID version);

    void set_config(const Realm::Config&);
    void create_sync_session(bool force_client_resync, bool validate_sync_history);
    void do_get_realm(Realm::Config config, std::shared_ptr<Realm>& realm,
                      util::Optional<VersionID> version,
                      std::unique_lock<std::mutex>& realm_lock, bool bind_to_context=true);
    void run_async_notifiers();
    void advance_helper_shared_group_to_latest();
    void clean_up_dead_notifiers();

    std::vector<std::shared_ptr<_impl::CollectionNotifier>> notifiers_for_realm(Realm&);
};

void translate_file_exception(StringData path, bool immutable=false);

template<typename Pred>
std::unique_lock<std::mutex> RealmCoordinator::wait_for_notifiers(Pred&& wait_predicate)
{
    std::unique_lock<std::mutex> lock(m_notifier_mutex);
    bool first = true;
    m_notifier_cv.wait(lock, [&] {
        if (wait_predicate())
            return true;
        if (first) {
            wake_up_notifier_worker();
            first = false;
        }
        return false;
    });
    return lock;
}

} // namespace _impl
} // namespace realm

#endif /* REALM_COORDINATOR_HPP */
