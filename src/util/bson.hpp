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

namespace realm {

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

class Bson;

template <typename ValueType>
class BsonContainer : public std::vector<ValueType> {
public:
    friend class ExtendedJsonParser;
};

class BsonArray : public BsonContainer<Bson> {
};

class Document : public std::map<std::string, Bson> {
public:
    static Document parse(const std::string& json);
    inline std::string to_json() const;


    friend class ExtendedJsonParser;
};

inline std::ostream& operator<<(std::ostream& out, const BsonArray& m)
{

}

class Bson {
public:
    Bson() noexcept
        : m_type(BsonType::Null)
    {
    }

    Bson(util::None) noexcept
        : Bson()
    {
    }

    Bson(int32_t i) noexcept;
    Bson(int64_t) noexcept;
    Bson(bool) noexcept;
    Bson(float) noexcept;
    Bson(double) noexcept;
    Bson(StringData) noexcept;
    Bson(BinaryData) noexcept;
    Bson(Timestamp) noexcept;
    Bson(Decimal128);
    Bson(ObjectId) noexcept;
    Bson(Document*) noexcept;
    Bson(BsonArray*) noexcept;

    // These are shortcuts for Bson(StringData(c_str)), and are
    // needed to avoid unwanted implicit conversion of char* to bool.
    Bson(char* c_str) noexcept
        : Bson(StringData(c_str))
    {
    }
    Bson(const char* c_str) noexcept
        : Bson(StringData(c_str))
    {
    }
    Bson(const std::string& s) noexcept
        : Bson(StringData(s))
    {
    }
    
    BsonType get_type() const noexcept
    {
        return BsonType(m_type);
    }

    template <class T>
    T get() const noexcept;

    // These functions are kept to be backwards compatible
    int32_t get_int32() const;
    int64_t get_int64() const;
    bool get_bool() const;
    float get_float() const;
    double get_double() const;
    StringData get_string() const;
    BinaryData get_binary() const;
    Timestamp get_timestamp() const;
    Decimal128 get_decimal() const;
    ObjectId get_object_id() const;
    Document& get_document() const;
    BsonArray& get_array() const;

    bool is_null() const;
    int compare(const Bson& b) const;
    bool operator==(const Bson& other) const
    {
        return compare(other) == 0;
    }
    bool operator!=(const Bson& other) const
    {
        return compare(other) != 0;
    }

private:
    friend std::ostream& operator<<(std::ostream& out, const Bson& m);

    BsonType m_type;
    union {
        int32_t int32_val;
        int64_t int64_val;
        bool bool_val;
        double double_val;
        StringData string_val;
        BinaryData binary_val;
        Timestamp date_val;
        ObjectId id_val;
        Decimal128 decimal_val;
        Document* document_val;
        BsonArray* array_val;
    };
};

// MARK: - BSON Implementation

template <>
inline BsonArray& Bson::get<BsonArray&>() const noexcept
{
    REALM_ASSERT(m_type == BsonType::Array);
    return *array_val;
}

inline BsonArray& Bson::get_array() const
{
    return get<BsonArray&>();
}

template <>
inline int32_t Bson::get<int32_t>() const noexcept
{
    REALM_ASSERT(get_type() == BsonType::Int32);
    return int32_val;
}

inline int32_t Bson::get_int32() const
{
    return get<int32_t>();
}

template <>
inline int64_t Bson::get<int64_t>() const noexcept
{
    REALM_ASSERT(get_type() == BsonType::Int64);
    return int64_val;
}

inline int64_t Bson::get_int64() const
{
    return get<int64_t>();
}

template <>
inline bool Bson::get<bool>() const noexcept
{
    REALM_ASSERT(get_type() == BsonType::Boolean);
    return bool_val;
}

inline bool Bson::get_bool() const
{
    return get<bool>();
}

inline float Bson::get_float() const
{
    return get<float>();
}

template <>
inline double Bson::get<double>() const noexcept
{
    REALM_ASSERT(get_type() == BsonType::Double);
    return double_val;
}

inline double Bson::get_double() const
{
    return get<double>();
}

template <>
inline StringData Bson::get<StringData>() const noexcept
{
    REALM_ASSERT(get_type() == BsonType::String);
    return string_val;
}

inline StringData Bson::get_string() const
{
    return get<StringData>();
}

template <>
inline BinaryData Bson::get<BinaryData>() const noexcept
{
    REALM_ASSERT(get_type() == BsonType::BinaryData);
    return binary_val;
}

inline BinaryData Bson::get_binary() const
{
    return get<BinaryData>();
}

template <>
inline Timestamp Bson::get<Timestamp>() const noexcept
{
    REALM_ASSERT(get_type() == BsonType::Timestamp);
    return date_val;
}

inline Timestamp Bson::get_timestamp() const
{
    return get<Timestamp>();
}

template <>
inline Decimal128 Bson::get<Decimal128>() const noexcept
{
    REALM_ASSERT(get_type() == BsonType::Decimal128);
    return decimal_val;
}

template <>
inline ObjectId Bson::get<ObjectId>() const noexcept
{
    REALM_ASSERT(get_type() == BsonType::ObjectId);
    return id_val;
}

template <>
inline Document& Bson::get<Document&>() const noexcept
{
    REALM_ASSERT(m_type == BsonType::Object);
    return *document_val;
}

inline Document& Bson::get_document() const
{
    return get<Document&>();
}

inline Decimal128 Bson::get_decimal() const
{
    return get<Decimal128>();
}

inline ObjectId Bson::get_object_id() const
{
    return get<ObjectId>();
}

inline bool Bson::is_null() const
{
    return (m_type == BsonType::Null);
}

inline std::ostream& operator<<(std::ostream& out, const Bson& m)
{
    out << "Bson(";
    if (m.is_null()) {
        out << "null";
    }
    else {
        switch (m.m_type) {
            case BsonType::Int32:
                out << m.int32_val;
                break;
            case BsonType::Int64:
                out << m.int64_val;
                break;
            case BsonType::Boolean:
                out << (m.bool_val ? "true" : "false");
                break;
            case BsonType::Double:
                out << m.double_val;
                break;
            case BsonType::String:
                out << m.string_val;
                break;
            case BsonType::BinaryData:
                out << m.binary_val;
                break;
            case BsonType::Timestamp:
                out << m.date_val;
                break;
            case BsonType::Decimal128:
                out << m.decimal_val;
                break;
            case BsonType::ObjectId: {
                out << m.get<ObjectId>();
                break;
            }
            case BsonType::Object: {
//                for (const auto& b : m.get_document()) {
//                    out<<b.first<<':'<<b.second;
//                }
                break;
            }
            case BsonType::Array: {
                out << m.get_array();
                break;
//                for (const auto& b : m.get_array()) {
//                    out << b;
//                }
//                break;
            }

            default:
                REALM_ASSERT(false);
        }
    }
    out << ")";
    return out;
}
//    namespace _impl {
//    inline int compare_string(StringData a, StringData b)
//    {
//        if (a == b)
//            return 0;
//        return utf8_compare(a, b) ? -1 : 1;
//    }
//
//    template <int>
//    struct IntTypeForSize;
//    template <>
//    struct IntTypeForSize<1> {
//        using type = uint8_t;
//    };
//    template <>
//    struct IntTypeForSize<2> {
//        using type = uint16_t;
//    };
//    template <>
//    struct IntTypeForSize<4> {
//        using type = uint32_t;
//    };
//    template <>
//    struct IntTypeForSize<8> {
//        using type = uint64_t;
//    };
//
//    template <typename Float>
//    inline int compare_float(Float a_raw, Float b_raw)
//    {
//        bool a_nan = std::isnan(a_raw);
//        bool b_nan = std::isnan(b_raw);
//        if (!a_nan && !b_nan) {
//            // Just compare as IEEE floats
//            return a_raw == b_raw ? 0 : a_raw < b_raw ? -1 : 1;
//        }
//        if (a_nan && b_nan) {
//            // Compare the nan values as unsigned
//            using IntType = typename _impl::IntTypeForSize<sizeof(Float)>::type;
//            IntType a = 0, b = 0;
//            memcpy(&a, &a_raw, sizeof(Float));
//            memcpy(&b, &b_raw, sizeof(Float));
//            return a == b ? 0 : a < b ? -1 : 1;
//        }
//        // One is nan, the other is not
//        // nans are treated as being less than all non-nan values
//        return a_nan ? -1 : 1;
//    }
//    } // namespace _impl

    inline int Bson::compare(const Bson& b) const
    {
        // Comparing types first makes it possible to make a sort of a list of Mixed
        // This will also handle the case where null values are considered lower than all other values
        if (m_type > b.m_type)
            return 1;
        else if (m_type < b.m_type)
            return -1;

        // Now we are sure the two types are the same
        if (is_null()) {
            // Both are null
            return 0;
        }

        switch (get_type()) {
            case BsonType::Int32:
                if (get<int32_t>() > b.get<int32_t>())
                    return 1;
                else if (get<int32_t>() < b.get<int32_t>())
                    return -1;
                break;
            case BsonType::Int64:
                if (get<int64_t>() > b.get<int64_t>())
                    return 1;
                else if (get<int64_t>() < b.get<int64_t>())
                    return -1;
                break;
            case BsonType::String:
                return _impl::compare_string(get<StringData>(), b.get<StringData>());
                break;
            case BsonType::Double:
                return _impl::compare_float(get<double>(), b.get<double>());
            case BsonType::Boolean:
                if (get<bool>() > b.get<bool>())
                    return 1;
                else if (get<bool>() < b.get<bool>())
                    return -1;
                break;
            case BsonType::Timestamp:
                if (get<Timestamp>() > b.get<Timestamp>())
                    return 1;
                else if (get<Timestamp>() < b.get<Timestamp>())
                    return -1;
                break;
            case BsonType::ObjectId: {
                auto l = get<ObjectId>();
                auto r = b.get<ObjectId>();
                if (l > r)
                    return 1;
                else if (l < r)
                    return -1;
                break;
            }
            case BsonType::Decimal128: {
                auto l = get<Decimal128>();
                auto r = b.get<Decimal128>();
                if (l > r)
                    return 1;
                else if (l < r)
                    return -1;
                break;
            }
            default:
                REALM_ASSERT_RELEASE(false && "Compare not supported for this column type");
                break;
        }

        return 0;
    }





inline Bson::Bson(int32_t v) noexcept
{
    m_type = BsonType::Int32;
    int32_val = v;
}

inline Bson::Bson(int64_t v) noexcept
{
    m_type = BsonType::Int64;
    int64_val = v;
}

inline Bson::Bson(bool v) noexcept
{
    m_type = BsonType::Boolean;
    bool_val = v;
}

inline Bson::Bson(double v) noexcept
{
    m_type = BsonType::Double;
    double_val = v;
}

inline Bson::Bson(StringData v) noexcept
{
    if (!v.is_null()) {
        m_type = BsonType::String;
        string_val = v;
    }
    else {
        m_type = BsonType::Null;
    }
}

inline Bson::Bson(BinaryData v) noexcept
{
    if (!v.is_null()) {
        m_type = BsonType::BinaryData;
        binary_val = v;
    }
    else {
        m_type = BsonType::Null;
    }
}

inline Bson::Bson(Timestamp v) noexcept
{
    if (!v.is_null()) {
        m_type = BsonType::Timestamp;
        date_val = v;
    }
    else {
        m_type = BsonType::Null;
    }
}

inline Bson::Bson(Decimal128 v)
{
    if (!v.is_null()) {
        m_type = BsonType::Decimal128;
        decimal_val = v;
    }
    else {
        m_type = BsonType::Null;
    }
}

inline Bson::Bson(ObjectId v) noexcept
{
    m_type = BsonType::ObjectId;
    id_val = v;
}

inline Bson::Bson(Document* v) noexcept
{
    m_type = BsonType::Object;
    document_val = v;
}

//inline Bson::Bson(Document& v) noexcept
//{
//    m_type = BsonType::Object;
//    document_val = v;
//}

inline Bson::Bson(BsonArray* v) noexcept
{
    m_type = BsonType::Array;
    array_val = v;
}

static void to_json(const Bson& bson, std::stringstream& ss)
{
    switch (bson.get_type())
    {
        case BsonType::Int32:
            ss << "{" << "\"$numberInt\"" << ":" << '"' << bson.get_int32() << '"' << "}";
            break;
        case realm::BsonType::Double:
            ss << "{" << "\"$numberDouble\"" << ":" << '"' << bson.get_double() << '"' << "}";
            break;
        case realm::BsonType::Int64:
            ss << "{" << "\"$numberLong\"" << ":" << '"' << bson.get_int64() << '"' << "}";
            break;
        case realm::BsonType::Decimal128:
            ss << "{" << "\"$numberDecimal\"" << ":" << '"' << bson.get_decimal() << '"' << "}";
            break;
        case realm::BsonType::ObjectId:
            ss << "{" << "\"$oid\"" << ":" << '"' << bson.get_object_id() << '"' << "}";
            break;
        case realm::BsonType::Object:
            ss << "{";
            for (auto const& pair : bson.get_document())
            {
                ss << '"' << pair.first << "\":";
                to_json(pair.second, ss);
                ss << ",";
            }
            ss.seekp(-1, std::ios_base::end);
            ss << "}";
            break;
        case realm::BsonType::Array:
            ss << "[";
            for (auto const& b : bson.get_array())
            {
                to_json(b, ss);
                ss << ",";
            }
            ss.seekp(-1, std::ios_base::end);
            ss << "]";
            break;
        case realm::BsonType::BinaryData:

            break;
        case realm::BsonType::Date:
            break;
        case realm::BsonType::Null:
            ss << "null";
            break;
        case realm::BsonType::RegularExpression:

            break;
        case realm::BsonType::JavaScript:

            break;
        case realm::BsonType::JavaScriptWithScope:

            break;
        case realm::BsonType::Timestamp:
            ss << "{\"$timestamp\":{\"t\":" << bson.get_timestamp().get_seconds() << ",\"i\":" << 1 << "}}";
            break;
        case realm::BsonType::MinKey:
            ss << "{\"$minKey\": 1}";
            break;
        case realm::BsonType::MaxKey:
            ss << "{\"$maxKey\": 1}";
            break;
        case realm::BsonType::String:
            ss << '"' << bson.get_string() << '"';
            break;
        case realm::BsonType::Boolean:
            ss << bson.get_bool();
            break;
    }
}

inline std::string Document::to_json() const
{
    std::stringstream ss;
    realm::to_json(Bson((Document*)this), ss);
    return ss.str();
}
} // namespace realm


            
#endif // REALM_BSON_HPP
