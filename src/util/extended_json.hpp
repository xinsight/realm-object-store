//
//  extended_json.hpp
//  realm-object-store
//
//  Created by Jason Flax on 29/02/2020.
//

#ifndef extended_json_hpp
#define extended_json_hpp

#include <stdio.h>
#include <realm/binary_data.hpp>
#include <realm/decimal128.hpp>
#include <realm/object_id.hpp>
#include <realm/util/to_string.hpp>
#include <realm/util/any.hpp>
#include <json.hpp>
#include <realm/util/any.hpp>
#include <realm/mixed.hpp>
#include <stack>
#include "bson.hpp"

using namespace nlohmann;
using namespace realm;

class ExtendedJsonParser : public nlohmann::json_sax<json> {
public:
    using number_integer_t = typename json::number_integer_t;
    using number_unsigned_t = typename json::number_unsigned_t;
    using number_float_t = typename json::number_float_t;
    using string_t = typename json::string_t;

    ExtendedJsonParser(Document* d, const bool allow_exceptions_ = true);

    /*!
    @brief a null value was read
    @return whether parsing should proceed
    */
    bool null() override;

    /*!
    @brief a boolean value was read
    @param[in] val  boolean value
    @return whether parsing should proceed
    */
    bool boolean(bool val) override;

    /*!
    @brief an integer number was read
    @param[in] val  integer value
    @return whether parsing should proceed
    */
    bool number_integer(number_integer_t val) override;

    /*!
    @brief an unsigned integer number was read
    @param[in] val  unsigned integer value
    @return whether parsing should proceed
    */
    bool number_unsigned(number_unsigned_t val) override;

    /*!
    @brief an floating-point number was read
    @param[in] val  floating-point value
    @param[in] s    raw token value
    @return whether parsing should proceed
    */
    bool number_float(number_float_t val, const string_t& s) override;

    /*!
    @brief a string was read
    @param[in] val  string value
    @return whether parsing should proceed
    @note It is safe to move the passed string.
    */
    bool string(string_t& val) override;

    /*!
    @brief the beginning of an object was read
    @param[in] elements  number of object elements or -1 if unknown
    @return whether parsing should proceed
    @note binary formats may report the number of elements
    */
    inline bool start_object(std::size_t) override;

    /*!
    @brief an object key was read
    @param[in] val  object key
    @return whether parsing should proceed
    @note It is safe to move the passed string.
    */
    bool key(string_t& val) override;

    /*!
    @brief the end of an object was read
    @return whether parsing should proceed
    */
    bool end_object() override;

    /*!
    @brief the beginning of an array was read
    @param[in] elements  number of array elements or -1 if unknown
    @return whether parsing should proceed
    @note binary formats may report the number of elements
    */
    bool start_array(std::size_t elements) override;

    /*!
    @brief the end of an array was read
    @return whether parsing should proceed
    */
    bool end_array() override;

    /*!
    @brief a parse error occurred
    @param[in] position    the position in the input where the error occurs
    @param[in] last_token  the last read token
    @param[in] ex          an exception object describing the error
    @return whether parsing should proceed (must return false)
     */
    bool parse_error(std::size_t position,
                     const std::string& last_token,
                     const nlohmann::detail::exception& ex)  override;
private:


    struct Mark {
        Bson container;
    };

    struct Instruction {
        int type;
        Bson value;
        bson_key_t key;
    };

    /// the parsed JSON value
    Document* head;
    
    std::stack<Document*> m_marks;
    std::stack<Instruction> m_instructions;
    bool allow_exceptions;
    int32_t m_type = -1;
};

#endif /* extended_json_hpp */
