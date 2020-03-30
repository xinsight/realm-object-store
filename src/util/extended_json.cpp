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
namespace realm {
namespace bson {
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
    bson_BinaryBase64,
    bson_BinarySubType,
    bson_Date,
    bson_Timestamp,
    bson_TimestampT,
    bson_TimestampI,
    bson_ObjectId,
    bson_String,
    bson_MaxKey,
    bson_MinKey,
    bson_RegularExpression,
    bson_RegularExpressionPattern,
    bson_RegularExpressionOptions,
    bson_Key,
    bson_XKey,
    bson_Skip
} instruction_type_t;

BsonDocument parse(const std::string& json) {
    auto out = new BsonDocument();
    ExtendedJsonParser p = ExtendedJsonParser(out);
    nlohmann::json::sax_parse(json, &p);
    return std::get<BsonDocument>(p.m_marks.top());
}

ExtendedJsonParser::ExtendedJsonParser(BsonDocument* d, const bool allow_exceptions_) :
head(d),
allow_exceptions(allow_exceptions_) {
//    objects.push(d);
}

/*!
@brief a null value was read
@return whether parsing should proceed
*/
bool ExtendedJsonParser::null() {
    auto instruction = m_instructions.top();
    m_instructions.pop();
    m_marks.top().push_back({instruction.key, Bson()});
    return true;
}

/*!
@brief a boolean value was read
@param[in] val  boolean value
@return whether parsing should proceed
*/
bool ExtendedJsonParser::boolean(bool val) {
    auto instruction = m_instructions.top();
    m_instructions.pop();
    m_marks.top().push_back({instruction.key, val});
    return true;
}

/*!
@brief an integer number was read
@param[in] val  integer value
@return whether parsing should proceed
*/
bool ExtendedJsonParser::number_integer(number_integer_t val) {
    switch (m_instructions.top().type) {
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
    auto instruction = m_instructions.top();
    m_instructions.pop();
    switch (instruction.type) {
        case bson_MaxKey:
            m_marks.top().push_back({instruction.key, max_key});
            break;
        case bson_MinKey:
            m_marks.top().push_back({instruction.key, min_key});
            break;
        case bson_TimestampI:
            if (m_marks.top().back().first == instruction.key) {
                auto ts = std::get<Timestamp>(m_marks.top().back().second);
                m_marks.top().pop_back();
                m_marks.top().push_back({instruction.key, Timestamp(ts.get_seconds(), val)});

                // pop vestigal timestamp instruction
                m_instructions.pop();
                m_instructions.push({bson_Skip});
                m_instructions.push({bson_Skip});
            } else {
//                m_marks.pop();
                m_marks.top().push_back({instruction.key, Timestamp(0, val)});
                instruction.type = bson_Timestamp;
            }
            break;
        case bson_TimestampT:
            if (m_marks.top().back().first == instruction.key) {
                auto ts = std::get<Timestamp>(m_marks.top().back().second);
                m_marks.top().pop_back();
                m_marks.top().push_back({instruction.key, Timestamp(val, ts.get_nanoseconds())});

                // pop vestigal teimstamp instruction
                m_instructions.pop();
                m_instructions.push({bson_Skip});
                m_instructions.push({bson_Skip});
            } else {
//                m_marks.pop();
                m_marks.top().push_back({instruction.key, Timestamp(val, 0)});
                instruction.type = bson_Timestamp;
            }
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
    // pop last instruction
    auto instruction = m_instructions.top();
    m_instructions.pop();

    switch (instruction.type) {
        case bson_NumberInt:
            m_marks.top().push_back({instruction.key, atoi(val.data())});
            m_instructions.push({bson_Skip});
            break;
        case bson_NumberLong:
            m_marks.top().push_back({instruction.key, (int64_t)atol(val.data())});
            m_instructions.push({bson_Skip});
            break;
        case bson_NumberDouble:
            m_marks.top().push_back({instruction.key, std::stod(val.data())});
            m_instructions.push({bson_Skip});
            break;
        case bson_NumberDecimal:
            m_marks.top().push_back({instruction.key, Decimal128(val)});
            m_instructions.push({bson_Skip});
            break;
        case bson_ObjectId:
            m_marks.top().push_back({instruction.key, ObjectId(val.data())});
            m_instructions.push({bson_Skip});
            break;
        case bson_Date:
            m_marks.top().push_back({instruction.key, Timestamp(atol(val.data()), 0)});
            // skip twice because this is a number long
            m_instructions.push({bson_Skip});
            m_instructions.push({bson_Skip});
            break;
        case bson_RegularExpressionPattern:
            // if we have already pushed a regex type
            if (m_marks.top().back().first == instruction.key) {
                auto regex = std::get<RegularExpression>(m_marks.top().back().second);
                m_marks.top().pop_back();
                m_marks.top().push_back({instruction.key, RegularExpression(val, regex.options())});

                // pop vestigal regex instruction
                m_instructions.pop();
                m_instructions.push({bson_Skip});
                m_instructions.push({bson_Skip});
            } else {
                m_marks.top().push_back({instruction.key, RegularExpression(val, "")});
            }

            break;
        case bson_RegularExpressionOptions:
            // if we have already pushed a regex type
            if (m_marks.top().back().first == instruction.key) {
                auto regex = std::get<RegularExpression>(m_marks.top().back().second);
                m_marks.top().pop_back();
                m_marks.top().push_back({instruction.key, RegularExpression(regex.pattern(), val)});
                // pop vestigal regex instruction
                m_instructions.pop();
                m_instructions.push({bson_Skip});
                m_instructions.push({bson_Skip});
            } else {
                m_marks.top().push_back({instruction.key, RegularExpression("", val)});
            }

            break;
        case bson_BinarySubType:
            // if we have already pushed a binary type
            if (m_marks.top().back().first == instruction.key) {
                // we will ignore the subtype for now
                // pop vestigal binary instruction
                m_instructions.pop();
                m_instructions.push({bson_Skip});
                m_instructions.push({bson_Skip});
            } else {
                // we will ignore the subtype for now
                m_marks.top().push_back({instruction.key, BinaryData()});
            }

            break;
        case bson_BinaryBase64:
            // if we have already pushed a binary type
            if (m_marks.top().back().first == instruction.key) {
                m_marks.top().pop_back();
                m_marks.top().push_back({instruction.key, BinaryData(val)});

                // pop vestigal binary instruction
                m_instructions.pop();
                m_instructions.push({bson_Skip});
                m_instructions.push({bson_Skip});
            } else {
                // we will ignore the subtype for now
                m_marks.top().push_back({instruction.key, BinaryData(val)});
            }
            
            break;
        default: {
            char* v = (char *)malloc(sizeof(char) * val.size());
            m_marks.top().push_back({instruction.key, StringData(strcpy(v, val.data()))});
            break;
        }
    }
    return true;
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
    else if (str_equal(val, "$timestamp"))
        return bson_Timestamp;
    else if (str_equal(val, "$date"))
        return bson_Date;
    else if (str_equal(val, "$oid"))
        return bson_ObjectId;
    else if (str_equal(val, "$maxKey"))
        return bson_MaxKey;
    else if (str_equal(val, "$minKey"))
        return bson_MinKey;
    else if (str_equal(val, "$regularExpression"))
        return bson_RegularExpression;
    else if (str_equal(val, "$binary"))
        return bson_Binary;
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
    
    if (!m_instructions.empty()) {
        auto top = m_instructions.top();

        if (top.type == bson_RegularExpression) {
            if (val == "pattern") {
                m_instructions.push({bson_RegularExpressionPattern, top.key});
            } else if (val == "options") {
                m_instructions.push({bson_RegularExpressionOptions, top.key});
            }
            return true;
        } else if (top.type == bson_Date) {
            return true;
        } else if (top.type == bson_Binary) {
            if (val == "base64") {
                m_instructions.push({bson_BinaryBase64, top.key});
            } else if (val == "subType") {
                m_instructions.push({bson_BinarySubType, top.key});
            }
            return true;
        } else if (top.type == bson_Timestamp) {
            if (val == "t") {
                m_instructions.push({bson_TimestampT, top.key});
            } else if (val == "i") {
                m_instructions.push({bson_TimestampI, top.key});
            }
            return true;
        }
    }

    auto type = bson_type_for_key(val.data());
    // if we have a legitimate bson type,
    if (type != bson_Key) {
        m_marks.pop();

        // if the previous is a key, we don't want it
        if (m_instructions.top().type == bson_Key)
            m_instructions.pop();

        m_instructions.top().type = type;
    } else {
        m_instructions.push({
            .key = std::move(val),
            .type = type
        });
    }
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

    if (!m_instructions.empty()) {
        auto top = m_instructions.top();

        switch (top.type) {
            case bson_NumberInt:
            case bson_NumberLong:
            case bson_NumberDouble:
            case bson_NumberDecimal:
            case bson_Binary:
            case bson_BinaryBase64:
            case bson_BinarySubType:
            case bson_Date:
            case bson_Timestamp:
            case bson_ObjectId:
            case bson_String:
            case bson_MaxKey:
            case bson_MinKey:
            case bson_RegularExpression:
            case bson_RegularExpressionPattern:
            case bson_RegularExpressionOptions:
                return true;
        }
    }

    if (!m_marks.empty()) {
        m_instructions.push(Instruction{bson_StartDocument, m_instructions.top().key});
    }
    m_marks.push(BsonContainer(BsonDocument()));

    return true;
}

/*!
@brief the end of an object was read
@return whether parsing should proceed
*/
bool ExtendedJsonParser::end_object() {
    std::cout<<"end object"<<std::endl;
    if (m_instructions.top().type == bson_Skip) {
        m_instructions.pop();
        return true;
    }

    if (m_marks.size() > 1) {
        auto document = m_marks.top();
        m_marks.pop();
        m_marks.top().push_back({m_instructions.top().key, std::get<BsonDocument>(document)});
        // pop key and document instructions
        m_instructions.pop();
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

    m_instructions.push(Instruction{bson_StartArray, m_instructions.top().key});
    m_marks.push(BsonArray());

    return true;
};

/*!
@brief the end of an array was read
@return whether parsing should proceed
*/
bool ExtendedJsonParser::end_array() {
    std::cout<<"end array"<<std::endl;

    if (m_marks.size() > 1) {
        auto container = m_marks.top();
        m_marks.pop();
        m_marks.top().push_back({m_instructions.top().key, std::get<BsonArray>(container)});
        // pop key and document instructions
        m_instructions.pop();
        m_instructions.pop();
    }
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

} // namespace bson
}   // namespace realm
