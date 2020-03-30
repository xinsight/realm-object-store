/*************************************************************************
 *
 * Copyright 2016 Realm Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 **************************************************************************/

#ifndef REALM_BSON_HPP
#define REALM_BSON_HPP

#include <cstdint> // int64_t - not part of C++03, not even required by C++11 (see C++11 section 18.4.1)

#include <cstddef> // size_t
#include <cstring>

#include <realm/keys.hpp>
#include <realm/binary_data.hpp>
#include <realm/unicode.hpp>
#include <realm/data_type.hpp>
#include <realm/string_data.hpp>
#include <realm/timestamp.hpp>
#include <realm/decimal128.hpp>
#include <realm/object_id.hpp>
#include <realm/util/assert.hpp>
#include <realm/utilities.hpp>
#include <map>
#include <iostream>
#include <realm/mixed.hpp>
#include <unordered_map>
#include "bson_document.hpp"
#include <sstream>
#include <json.hpp>
#include <variant>
#include <optional>
#include <set>
#include <unordered_set>
#include <any>

namespace realm {
namespace bson {

static constexpr bool str_equal(char const * a, char const * b) {
    return *a == *b && (*a == '\0' || str_equal(a + 1, b + 1));
}

/// Provides regular expression capabilities for pattern matching strings in queries.
/// MongoDB uses Perl compatible regular expressions (i.e. "PCRE") version 8.42 with UTF-8 support.
struct RegularExpression {
    enum class Options {
        None,
        IgnoreCase,
        Multiline,
        Dotall,
        Extended
    };


    RegularExpression(const std::string& pattern,
                      const std::string& options) :
    m_pattern(std::move(pattern)) {
        std::transform(options.begin(),
                       options.end(),
                       std::back_inserter(m_options),
                       [](const char c) { return options_string_to_options(c); });
    }

    RegularExpression(const std::string& pattern,
                      const std::vector<Options> options) :
    m_pattern(std::move(pattern)),
    m_options(options) {}

    RegularExpression& operator=(const RegularExpression& regex)
    {
        m_pattern = regex.m_pattern;
        m_options = regex.m_options;
        return *this;
    }

    std::string pattern() const
    {
        return m_pattern;
    }

    std::vector<Options> options() const
    {
        return m_options;
    }
private:
    static constexpr Options options_string_to_options(const char option)
    {
        if (option == 'i') {
            return Options::IgnoreCase;
        } else if (option == 'm') {
            return Options::Multiline;
        } else if (option == 's') {
            return Options::Dotall;
        } else if (option == 'x') {
            return Options::Extended;
        } else {
            throw std::runtime_error("invalid options type");
        }
    };

    std::string m_pattern;
    std::vector<Options> m_options;
};

/// MaxKey will always be the smallest value when comparing to other BSON types
struct MinKey {
    constexpr explicit MinKey(int)
    {
    }
};
static constexpr MinKey min_key{1};

/// MaxKey will always be the greatest value when comparing to other BSON types
struct MaxKey {
    constexpr explicit MaxKey(int)
    {
    }
};
static constexpr MinKey max_key{1};


struct index_hasher {
    enum class hash_type {
        key, idx
    };

//    index_hasher() {}
    virtual ~index_hasher() = default;
    virtual hash_type type() const = 0;
};

struct indexed_seq_key {
    using WeakIndexedHasher = std::weak_ptr<index_hasher>;

    struct hasher {
        std::size_t operator()(const indexed_seq_key& v) const noexcept {
            return v.hash();
        }
    };

    indexed_seq_key() = delete;
    indexed_seq_key(std::string key, long long idx, const index_hasher& hasher_gen) :
    m_key(key),
    m_idx(idx),
    m_hasher(std::move(&hasher_gen)) {}

    indexed_seq_key(long long idx, const index_hasher& hasher_gen) :
    m_idx(idx),
    m_hasher(std::move(&hasher_gen)) {}
    indexed_seq_key(std::string key, const index_hasher& hasher_gen) : m_key(key), m_hasher(std::move(&hasher_gen)) {}

    indexed_seq_key(const indexed_seq_key& other)
    : m_key(other.m_key), m_idx(other.m_idx), m_hasher(other.m_hasher)
    {
    }
    indexed_seq_key(indexed_seq_key&&) = default;
    ~indexed_seq_key() {}

    indexed_seq_key& operator =(const indexed_seq_key& map ) {
        m_key = map.m_key;
        m_idx = map.m_idx;
        m_hasher = map.m_hasher;
        return *this;
    }

    bool operator==(const indexed_seq_key& rhs) const {
        if (!m_key.empty() && !rhs.m_key.empty()) {
            return rhs.m_key == m_key;
        }

        return m_idx == rhs.m_idx;
    }

    bool operator<(const indexed_seq_key& rhs) const {
        if (rhs.m_idx != -1) {
            if (m_idx < rhs.m_idx) return true;
            if (rhs.m_idx < m_idx) return false;
        }
//        if (key < rhs.key) return true;
//        if (rhs.key < key) return false;
        return memcmp(m_key.data(), rhs.m_key.data(), rhs.m_key.size()) != 0;
    }

    std::string key() const { return m_key; }
private:
    std::string      m_key;
    long long        m_idx;
    const index_hasher* m_hasher = NULL;

    size_t hash() const {
        switch (m_hasher->type()) {
            case index_hasher::hash_type::key:
                return std::hash<std::string>{}(m_key);
            case index_hasher::hash_type::idx:
                return m_idx;
        }
    }
};

template <typename T>
class indexed_seq : std::unordered_map<indexed_seq_key, T, indexed_seq_key::hasher>,
                    public index_hasher
{
public:
    using base = std::unordered_map<indexed_seq_key, T, indexed_seq_key::hasher>;
    using base::base;
    using base::operator=;
    using base::end;
    using base::size;
//    using base::~indexed_seq;

    virtual ~indexed_seq() {
//        base::~base();
    };

    class iterator: public std::iterator<
                        std::forward_iterator_tag, // iterator_category
                        T,                      // value_type
                        T,                      // difference_type
                        T*,               // pointer
                        T&                // reference
    > {
        size_t m_idx = 0;
        indexed_seq* m_map;
    public:
        iterator(indexed_seq* map, size_t idx) : m_idx(idx), m_map(map) {}
        iterator& operator++() {
            m_idx++;
            return *this;
        }
        iterator& operator--() {
            m_idx--;
            return *this;
        }
        iterator operator++(int) {
            return ++(*this);
        }
        iterator operator--(int) {
            return --(*this);
        }
        bool operator==(iterator other) const {return m_idx == other.m_idx;}
        bool operator!=(iterator other) const {return !(*this == other);}

        std::pair<std::string, T> operator*()
        {
            return m_map->operator[](m_idx);
        }
    };

    using entry = std::pair<std::string, T>;

    entry operator[](size_t idx)
    {
        m_type = index_hasher::hash_type::idx;
        auto key = indexed_seq_key(static_cast<long long>(idx), *this);

        auto entry = *(base::find(key));
        return { entry.first.key(), entry.second };
    }

    iterator begin()
    {
        return iterator(this, 0);
    }

    iterator end()
    {
        return iterator(this, this->size());
    }

//    std::pair<std::string, T> (const std::string& k)
//    {
//        m_type = index_hasher::hash_type::key;
//        auto entry = base::find({k, this});
//        if (entry != base::end()) {
//            T& value = (*entry).second;
//            return value;
//        }
//
//        T& test = base::operator[]({k, static_cast<long>(base::size()), this});
//        return test;
//    }

    T& operator[](const std::string& k)
    {
        m_type = index_hasher::hash_type::key;

        auto entry = base::find({k, *this});
        if (entry != base::end()) {
            T& value = (*entry).second;
            return value;
        }

        T& test = base::operator[]({k, static_cast<long>(base::size()), *this});
        return test;
    }



//    typename base::iterator insert(std::pair<std::string, T> pair)
//    {
//        return this->insert(pair);
//    }
//
//    typename base::iterator insert(const std::pair<std::string, T> &pair)
//    {
//        return this->insert(pair);
//    }
//
    iterator back()
    {
        return this->end()--;
    }

    void pop_back()
    {
        auto last_idx = this->size() - 1;

        m_type = index_hasher::hash_type::idx;

        base::erase(this->find({static_cast<long long>(last_idx), *this}));
    }

    virtual hash_type type() const override
    {
        return m_type;
    }
//    const typename std::map<indexed_seq_key, T>::iterator end()
//    {
//        return m_map.end();
//    }
private:
    friend struct indexed_seq_key;
    index_hasher::hash_type m_type;
//    std::unordered_set<indexed_seq> m_set;
//    hash_type type() const override
//    {
//        return m_type;
//    }
};


class Bson;
//template <class T>
class Bson : public std::variant<
    int32_t,
    int64_t,
    bool,
    float,
    double,
    StringData,
    BinaryData,
    Timestamp,
    Decimal128,
    ObjectId,
    RegularExpression,
    MinKey,
    MaxKey,
    indexed_seq<Bson>,
    std::vector<Bson>
> {
public:
    using base = std::variant<
        int32_t,
        int64_t,
        bool,
        float,
        double,
        StringData,
        BinaryData,
        Timestamp,
        Decimal128,
        ObjectId,
        RegularExpression,
        MinKey,
        MaxKey,
        indexed_seq<Bson>,
        std::vector<Bson>
    >;
    using base::base;
    using base::operator=;
};

//template <template<class> class K>
//struct __bson_knot : public K<__bson_knot<K>> {
//    using K<__bson_knot<K>>::K;
//    using K<__bson_knot<K>>::operator=;
//
//    __bson_knot& operator=(const __bson_knot& bson) {
//
//    }
//};
//
//using Bson = __bson_knot<__bson>;
//class Bson : public std::variant<
//    int32_t,
//    int64_t,
//    bool,
//    float,
//    double,
//    StringData,
//    BinaryData,
//    Timestamp,
//    Decimal128,
//    ObjectId,
//    RegularExpression,
//    MinKey,
//    MaxKey,
//    Code,
//    CodeWithScope,
//    ordered_map<Bson>,
//    std::vector<Bson>
//> {
//public:
//    using base = std::variant<
//        int32_t,
//        int64_t,
//        bool,
//        float,
//        double,
//        StringData,
//        BinaryData,
//        Timestamp,
//        Decimal128,
//        ObjectId,
//        RegularExpression,
//        MinKey,
//        MaxKey,
//        Code,
//        CodeWithScope,
//        ordered_map<Bson>,
//        std::vector<Bson>
//    >;
//    using base::base;
//    using base::operator=;
//};

template <class T>
inline bool operator == (std::pair<std::string, T> const& lhs, std::pair<std::string, T> const& rhs)
{
    return lhs.first == rhs.first;
}

//template<>
//class __bson_document<Bson>
//{
//    __bson_document<Bson>& __bson_document<Bson>::operator=(const __bson_document<Bson>& d);
//};

//__bson_document& __bson_document::operator=(const __bson_document& d)
//{
//    return d;
//}

using BsonDocument = indexed_seq<Bson>;
//class BsonDocument : std::unordered_map<std::string, __proxy_base>
//{
//
//
//    using base = std::unordered_map<std::string, __proxy_base>;
//    using base::base;
//
//public:
//    __proxy_base& operator[](const std::string& k)
//    {
//        return base::operator[](k).forward(m_insert_order, k);
//    }
//
//    void pop_back()
//    {
//        base::erase(*m_insert_order.end());
//        m_insert_order.erase(*m_insert_order.end());
//    }
//private:
////    std::unordered_map<std::string, Bson> m_map;
//    std::unordered_set<std::string> m_insert_order;
//};
//
//class Bson : public std::variant<
//    int32_t,
//    int64_t,
//    bool,
//    float,
//    double,
//    StringData,
//    BinaryData,
//    Timestamp,
//    Decimal128,
//    ObjectId,
//    RegularExpression,
//    MinKey,
//    MaxKey,
//    Code,
//    CodeWithScope,
//    BsonDocument,
//    std::vector<Bson>
//> {
//public:
//    using base = std::variant<
//        int32_t,
//        int64_t,
//        bool,
//        float,
//        double,
//        StringData,
//        BinaryData,
//        Timestamp,
//        Decimal128,
//        ObjectId,
//        RegularExpression,
//        MinKey,
//        MaxKey,
//        Code,
//        CodeWithScope,
//        BsonDocument,
//        std::vector<Bson>
//    >;
//    using base::base;
//    using base::operator=;
//};

using BsonArray = std::vector<Bson>;
class BsonContainer : public std::variant<BsonDocument, BsonArray> {
public:
    using base = std::variant<BsonDocument, BsonArray>;
    using base::base;
    using base::operator=;
    
    void push_back(std::pair<std::string, Bson> value) {
        if (std::holds_alternative<BsonDocument>(*this)) {
            std::get<BsonDocument>(*this)[value.first] = value.second;
        } else {
            std::get<BsonArray>(*this).push_back(value.second);
        }
    }

    std::pair<std::string, Bson> back()
    {
        if (std::holds_alternative<BsonDocument>(*this)) {
            auto pair = *std::get<BsonDocument>(*this).back();

            return pair;
        } else {
            return {"", std::get<BsonArray>(*this).back()};
        }
    }

    void pop_back()
    {
        if (std::holds_alternative<BsonDocument>(*this)) {
            std::get<BsonDocument>(*this).pop_back();
        } else {
            std::get<BsonArray>(*this).pop_back();
        }
    }
private:
//    std::variant<BsonDocument, BsonArray> m_container;
};

struct bson_key_t {
    typedef enum {
        none, string, index
    } bson_key_type;

    union {
        char* string_val;
        std::size_t index_val;
    };

    bson_key_type type = none;

    bson_key_t() {
    }

    bson_key_t(const std::string& v) {
        string_val = (char*)malloc(sizeof(char*) * v.size());
        v.copy(string_val, v.size());
        type = string;
    }

    bson_key_t(size_t v) {
        index_val = v;
        type = index;
    }

    bool operator==(const bson_key_t& key)
    {
        if (type == key.type) {
            switch (type) {
                case none:
                    return true;
                case string:
                    return string_val == key.string_val;
                case index:
                    return index_val = key.index_val;
            }
        }

        return false;
    }

    friend std::ostream& operator<<(std::ostream& out, const bson_key_t& m);
};


inline std::ostream& operator<<(std::ostream& out, const bson_key_t& m)
{
    switch (m.type) {
        case bson_key_t::none:
            out << "(none)";
            break;
        case bson_key_t::string:
            out << m.string_val;
            break;
        case bson_key_t::index:
            out << m.index_val;
            break;
    }
    return out;
}


enum class BsonType {
    Double              = 1,
    String              = 2,
    Object              = 3,
    Array               = 4,
    BinaryData          = 5,
    ObjectId            = 7,
    Boolean             = 8,
    Date                = 9,
    Null                = 10,
    RegularExpression   = 11,
    JavaScript          = 13,
    JavaScriptWithScope = 15,
    Int32               = 16,
    Timestamp           = 17,
    Int64               = 18,
    Decimal128          = 19,
    MinKey              = -1,
    MaxKey              = 127
};

inline std::ostream& operator<<(std::ostream& out, const BsonArray& m)
{

}

static void to_json(const Bson& bson, std::stringstream& ss)
{
//    switch (bson)
//    {
//        case BsonType::Int32:
//            ss << "{" << "\"$numberInt\"" << ":" << '"' << bson.get_int32() << '"' << "}";
//            break;
//        case realm::BsonType::Double:
//            ss << "{" << "\"$numberDouble\"" << ":" << '"' << bson.get_double() << '"' << "}";
//            break;
//        case realm::BsonType::Int64:
//            ss << "{" << "\"$numberLong\"" << ":" << '"' << bson.get_int64() << '"' << "}";
//            break;
//        case realm::BsonType::Decimal128:
//            ss << "{" << "\"$numberDecimal\"" << ":" << '"' << bson.get_decimal() << '"' << "}";
//            break;
//        case realm::BsonType::ObjectId:
//            ss << "{" << "\"$oid\"" << ":" << '"' << bson.get_object_id() << '"' << "}";
//            break;
//        case realm::BsonType::Object:
//            ss << "{";
//            for (auto const& pair : bson.get_document())
//            {
//                ss << '"' << pair.first << "\":";
//                to_json(pair.second, ss);
//                ss << ",";
//            }
//            ss.seekp(-1, std::ios_base::end);
//            ss << "}";
//            break;
//        case realm::BsonType::Array:
//            ss << "[";
//            for (auto const& b : bson.get_array())
//            {
//                to_json(b, ss);
//                ss << ",";
//            }
//            ss.seekp(-1, std::ios_base::end);
//            ss << "]";
//            break;
//        case realm::BsonType::BinaryData:
//
//            break;
//        case realm::BsonType::Date:
//            break;
//        case realm::BsonType::Null:
//            ss << "null";
//            break;
//        case realm::BsonType::RegularExpression:
//
//            break;
//        case realm::BsonType::JavaScript:
//
//            break;
//        case realm::BsonType::JavaScriptWithScope:
//
//            break;
//        case realm::BsonType::Timestamp:
//            ss << "{\"$timestamp\":{\"t\":" << bson.get_timestamp().get_seconds() << ",\"i\":" << 1 << "}}";
//            break;
//        case realm::BsonType::MinKey:
//            ss << "{\"$minKey\": 1}";
//            break;
//        case realm::BsonType::MaxKey:
//            ss << "{\"$maxKey\": 1}";
//            break;
//        case realm::BsonType::String:
//            ss << '"' << bson.get_string() << '"';
//            break;
//        case realm::BsonType::Boolean:
//            ss << bson.get_bool();
//            break;
//    }
}

inline std::string to_json(const Bson& bson)
{
    std::stringstream ss;
    bson::to_json(bson, ss);
    return ss.str();
}
} // namespace bson
} // namespace realm


            
#endif // REALM_BSON_HPP
