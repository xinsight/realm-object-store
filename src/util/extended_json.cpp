#include "extended_json.hpp"
#include <realm/binary_data.hpp>
#include <realm/decimal128.hpp>
#include <realm/object_id.hpp>
#include <realm/util/to_string.hpp>
#include <realm/util/any.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <ctype.h>
#include <limits>
#include <time.h>
#include <map>
#include <typeindex>
#include "bson_container.hpp"
//
//using namespace nlohmann;

typedef enum {
    bson_StartDocument,
    bson_EndDocument,
    bson_StartArray,
    bson_EndArray,
    bson_NumberInt,
    bson_NumberLong,
    bson_NumberDouble,
    bson_NumberDecimal,
    bson_Binary,
    bson_Date,
    bson_Timestamp,
    bson_ObjectId,
    bson_Key
} instruction_type_t;

Document Document::parse(const std::string& json) {
    auto out = new Document();
    ExtendedJsonParser p = ExtendedJsonParser(out);
    nlohmann::json::sax_parse(json, &p);
    return *out;
}

ExtendedJsonParser::ExtendedJsonParser(Document* d, const bool allow_exceptions_) :
head(d),
allow_exceptions(allow_exceptions_) {
//    objects.push(d);
}

///*!
//@brief a null value was read
//@return whether parsing should proceed
//*/
//bool ExtendedJsonParser::null() {
////    m_instructions.top().;
//    return true;
//}
//
///*!
//@brief a boolean value was read
//@param[in] val  boolean value
//@return whether parsing should proceed
//*/
//bool ExtendedJsonParser::boolean(bool val) {
//    m_instructions.top().value = val;
//    return true;
//}
//
///*!
//@brief an integer number was read
//@param[in] val  integer value
//@return whether parsing should proceed
//*/
//bool ExtendedJsonParser::number_integer(number_integer_t val) {
//    switch (m_type) {
//        case bson_Timestamp:
//        if (val != 1)
////            head->head->prev->prev->value.operator[](head->head->prev->prev->key) = Timestamp(val, 0);
//        break;
//    }
//    return true;
//}
//
///*!
//@brief an unsigned integer number was read
//@param[in] val  unsigned integer value
//@return whether parsing should proceed
//*/
//bool ExtendedJsonParser::number_unsigned(number_unsigned_t val) {
//    switch (m_type) {
//        case bson_Timestamp:
//        if (val != 1)
////            head->head->prev->prev->value.operator[](head->head->prev->prev->key) = Timestamp(val, 0);
//        break;
//    }
//    return true;
//}
//
///*!
//@brief an floating-point number was read
//@param[in] val  floating-point value
//@param[in] s    raw token value
//@return whether parsing should proceed
//*/
//bool ExtendedJsonParser::number_float(number_float_t, const string_t&) {
//    return true;
//}
//
///*!
//@brief a string was read
//@param[in] val  string value
//@return whether parsing should proceed
//@note It is safe to move the passed string.
//*/
//bool ExtendedJsonParser::string(string_t& val) {
//
////    std::cout<<"string: "<<val<<" for key:"<<head->head->key<<std::endl;
//    auto instruction = m_instructions.top();
//    switch (instruction.type) {
//        case bson_NumberInt:
//            m_instructions.top().value = atoi(val.data());
//            break;
//        case bson_NumberLong:
//            m_instructions.top().value = (int64_t)atol(val.data());
//            break;
//        case bson_NumberDouble:
//            m_instructions.top().value = std::stod(val);
//            break;
//        case bson_NumberDecimal:
//            m_instructions.top().value = Decimal128(val);
//            break;
//        case bson_ObjectId:
//            m_instructions.top().value = ObjectId(val.data());
//            break;
//        case bson_Date:
//            m_instructions.top().value = Timestamp(atol(val.data()), 0);
//            break;
//        default: {
//            char* buffer = (char*)malloc(sizeof(char*) * val.size());
//            strcpy(buffer, val.data());
//            m_instructions.top().value = buffer;
//            break;
//        }
//    }
//    return true;
//}
//
//static constexpr bool str_equal(char const * a, char const * b) {
//    return *a == *b && (*a == '\0' || str_equal(a + 1, b + 1));
//}
//
//static constexpr int bson_type_for_key(const char * val) {
//    if (str_equal(val, "$numberInt"))
//        return bson_NumberInt;
//    if (str_equal(val, "$numberLong"))
//        return bson_NumberLong;
//    if (str_equal(val, "$numberDouble"))
//        return bson_NumberDouble;
//    else if (str_equal(val, "$numberDecimal"))
//        return bson_NumberDecimal;
//    if (str_equal(val, "$timestamp"))
//        return bson_Timestamp;
//    if (str_equal(val, "$date"))
//        return bson_Date;
//    else if (str_equal(val, "$oid"))
//        return bson_ObjectId;
//    else return bson_Key;
//}
//
///*!
//@brief an object key was read
//@param[in] val  object key
//@return whether parsing should proceed
//@note It is safe to move the passed string.
//*/
//bool ExtendedJsonParser::key(string_t& val) {
//    std::cout<<"key:"<<val<<std::endl;
//    if (m_type == bson_Date || m_type == bson_Timestamp)
//        return true;
//
//    // check if this is a plain key, or xjson key
//    auto instruction_type = bson_type_for_key(val.data());
//    if (instruction_type == bson_Key) {
//        m_instructions.top().key = val;
//    } else {
////        delete m_marks.top();
//    }
//
//    // if it's a plain key, we need to mark it
//    // else, we intentionally ignore it
//    return true;
//}
//
//
///*!
//@brief the beginning of an object was read
//@param[in] elements  number of object elements or -1 if unknown
//@return whether parsing should proceed
//@note binary formats may report the number of elements
//*/
//bool ExtendedJsonParser::start_object(std::size_t) {
//    std::cout<<"start object"<<std::endl;
//    // If m_type is any of the nested types,
//    // do not switch the type
//    if (m_type != bson_Date
//        && m_type != bson_Timestamp
//        && m_type != bson_Binary)
//        m_type = bson_StartDocument;
//
//    if (!m_marks.empty())
//        head = new Document();
//    m_marks.push(head);
//    m_instructions.push(Instruction{bson_StartDocument});
//    return true;
//}
//
///*!
//@brief the end of an object was read
//@return whether parsing should proceed
//*/
//bool ExtendedJsonParser::end_object() {
//    if (m_marks.empty()) {
//        return true;
//    }
//
//    m_marks.pop();
//    head = m_marks.top();
//
//    while (!m_instructions.empty()) {
//        auto top = m_instructions.top();
//        head->operator[](top.key.string_val) = top.value;
//        m_instructions.pop();
//    }
//    return true;
//};
//
///*!
//@brief the beginning of an array was read
//@param[in] elements  number of array elements or -1 if unknown
//@return whether parsing should proceed
//@note binary formats may report the number of elements
//*/
//bool ExtendedJsonParser::start_array(std::size_t) {
//    std::cout<<"start array"<<std::endl;
//    m_type = bson_StartArray;
//    auto array = new BsonArray();
////    head->head->value = Bson(array);
////    head = (Document*)array;
////    head->head->key = 0;
//    return true;
//};
//
///*!
//@brief the end of an array was read
//@return whether parsing should proceed
//*/
//bool ExtendedJsonParser::end_array() {
//    return true;
//};
//
///*!
//@brief a parse error occurred
//@param[in] position    the position in the input where the error occurs
//@param[in] last_token  the last read token
//@param[in] ex          an exception object describing the error
//@return whether parsing should proceed (must return false)
//*/
//bool ExtendedJsonParser::parse_error(std::size_t position,
//                                     const std::string& last_token,
//                                     const nlohmann::detail::exception& ex) {
//    std::cout<<"!! error at !!"<<position<<last_token<<ex.what()<<std::endl;
//    return false;
//};

/*!
@brief a null value was read
@return whether parsing should proceed
*/
bool ExtendedJsonParser::null() {
//    m_instructions.top().;
    return true;
}

/*!
@brief a boolean value was read
@param[in] val  boolean value
@return whether parsing should proceed
*/
bool ExtendedJsonParser::boolean(bool val) {
    m_instructions.top().value = val;
    return true;
}

/*!
@brief an integer number was read
@param[in] val  integer value
@return whether parsing should proceed
*/
bool ExtendedJsonParser::number_integer(number_integer_t val) {
    switch (m_type) {
        case bson_Timestamp:
        if (val != 1)
//            head->head->prev->prev->value.operator[](head->head->prev->prev->key) = Timestamp(val, 0);
        break;
    }
    return true;
}

/*!
@brief an unsigned integer number was read
@param[in] val  unsigned integer value
@return whether parsing should proceed
*/
bool ExtendedJsonParser::number_unsigned(number_unsigned_t val) {
    switch (m_type) {
        case bson_Timestamp:
        if (val != 1)
//            head->head->prev->prev->value.operator[](head->head->prev->prev->key) = Timestamp(val, 0);
        break;
    }
    return true;
}

/*!
@brief an floating-point number was read
@param[in] val  floating-point value
@param[in] s    raw token value
@return whether parsing should proceed
*/
bool ExtendedJsonParser::number_float(number_float_t, const string_t&) {
    return true;
}

/*!
@brief a string was read
@param[in] val  string value
@return whether parsing should proceed
@note It is safe to move the passed string.
*/
bool ExtendedJsonParser::string(string_t& val) {

//    std::cout<<"string: "<<val<<" for key:"<<head->head->key<<std::endl;
    auto instruction = m_instructions.top();
    switch (instruction.type) {
        case bson_NumberInt:
            m_instructions.top().value = atoi(val.data());
            break;
        case bson_NumberLong:
            m_instructions.top().value = (int64_t)atol(val.data());
            break;
        case bson_NumberDouble:
            m_instructions.top().value = std::stod(val);
            break;
        case bson_NumberDecimal:
            m_instructions.top().value = Decimal128(val);
            break;
        case bson_ObjectId:
            m_instructions.top().value = ObjectId(val.data());
            break;
        case bson_Date:
            m_instructions.top().value = Timestamp(atol(val.data()), 0);
            break;
        default: {
            char* buffer = (char*)malloc(sizeof(char*) * val.size());
            strcpy(buffer, val.data());
            m_instructions.top().value = buffer;
            break;
        }
    }
    return true;
}

static constexpr bool str_equal(char const * a, char const * b) {
    return *a == *b && (*a == '\0' || str_equal(a + 1, b + 1));
}

static constexpr int bson_type_for_key(const char * val) {
    if (str_equal(val, "$numberInt"))
        return bson_NumberInt;
    if (str_equal(val, "$numberLong"))
        return bson_NumberLong;
    if (str_equal(val, "$numberDouble"))
        return bson_NumberDouble;
    else if (str_equal(val, "$numberDecimal"))
        return bson_NumberDecimal;
    if (str_equal(val, "$timestamp"))
        return bson_Timestamp;
    if (str_equal(val, "$date"))
        return bson_Date;
    else if (str_equal(val, "$oid"))
        return bson_ObjectId;
    else return bson_Key;
}

/*!
@brief an object key was read
@param[in] val  object key
@return whether parsing should proceed
@note It is safe to move the passed string.
*/
bool ExtendedJsonParser::key(string_t& val) {
    std::cout<<"key:"<<val<<std::endl;
    if (m_type == bson_Date || m_type == bson_Timestamp)
        return true;

    // check if this is a plain key, or xjson key
    auto instruction_type = bson_type_for_key(val.data());
    if (instruction_type == bson_Key) {
        m_instructions.top().key = val;
    } else {
//        delete m_marks.top();
    }

    // if it's a plain key, we need to mark it
    // else, we intentionally ignore it
    return true;
}


/*!
@brief the beginning of an object was read
@param[in] elements  number of object elements or -1 if unknown
@return whether parsing should proceed
@note binary formats may report the number of elements
*/
bool ExtendedJsonParser::start_object(std::size_t) {
    std::cout<<"start object"<<std::endl;
    // If m_type is any of the nested types,
    // do not switch the type
    if (m_type != bson_Date
        && m_type != bson_Timestamp
        && m_type != bson_Binary)
        m_type = bson_StartDocument;

    if (!m_marks.empty())
        head = new Document();
    m_marks.push(head);
    m_instructions.push(Instruction{bson_StartDocument});
    return true;
}

/*!
@brief the end of an object was read
@return whether parsing should proceed
*/
bool ExtendedJsonParser::end_object() {
    if (m_marks.empty()) {
        return true;
    }

    m_marks.pop();
    head = m_marks.top();

    while (!m_instructions.empty()) {
        auto top = m_instructions.top();
        head->operator[](top.key.string_val) = top.value;
        m_instructions.pop();
    }
    return true;
};

/*!
@brief the beginning of an array was read
@param[in] elements  number of array elements or -1 if unknown
@return whether parsing should proceed
@note binary formats may report the number of elements
*/
bool ExtendedJsonParser::start_array(std::size_t) {
    std::cout<<"start array"<<std::endl;
    m_type = bson_StartArray;
    auto array = new BsonArray();
//    head->head->value = Bson(array);
//    head = (Document*)array;
//    head->head->key = 0;
    return true;
};

/*!
@brief the end of an array was read
@return whether parsing should proceed
*/
bool ExtendedJsonParser::end_array() {
    return true;
};

/*!
@brief a parse error occurred
@param[in] position    the position in the input where the error occurs
@param[in] last_token  the last read token
@param[in] ex          an exception object describing the error
@return whether parsing should proceed (must return false)
*/
bool ExtendedJsonParser::parse_error(std::size_t position,
                                     const std::string& last_token,
                                     const nlohmann::detail::exception& ex) {
    std::cout<<"!! error at !!"<<position<<last_token<<ex.what()<<std::endl;
    return false;
};
