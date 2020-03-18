#include "catch2/catch.hpp"
#include <stdio.h>
#include <json.hpp>
#include "util/test_utils.hpp"
#include "util/test_file.hpp"
#include "util/bson.hpp"

using namespace nlohmann;
using namespace realm;

TEST_CASE("extjson", "[util]") {
    SECTION("basics") {
//        "Array": [
//            {"$numberInt": "1"},
//            {"$numberInt": "2"},
//            {"$numberInt": "3"},
//            {"$numberInt": "4"},
//            {"$numberInt": "5"}
//        ],
        auto document = Document::parse(R"P(
          {
             "_id": {
                 "$oid": "57e193d7a9cc81b4027498b5"
             },
             "String": "string",
             "Int32": {
                 "$numberInt": "42"
             },
             "Int64": {
                 "$numberLong": "42"
             },
             "Double": {
                 "$numberDouble": "42.42"
             },
             "SpecialFloat": {
                 "$numberDouble": "NaN"
             },
             "Decimal": {
                 "$numberDecimal": "1234"
             },
             "Binary": {
                 "$binary": {
                     "base64": "o0w498Or7cijeBSpkquNtg==",
                     "subType": "03"
                 }
             },
             "BinaryUserDefined": {
                 "$binary": {
                     "base64": "AQIDBAU=",
                     "subType": "80"
                 }
             },
             "Code": {
                 "$code": "function() {}"
             },
             "CodeWithScope": {
                 "$code": "function() {}",
                 "$scope": {}
             },
             "Subdocument": {
                 "foo": "bar"
             },

             "Timestamp": {
                 "$timestamp": { "t": 42, "i": 1 }
             },
             "RegularExpression": {
                 "$regularExpression": {
                     "pattern": "foo*",
                     "options": "ix"
                 }
             },
             "DatetimeEpoch": {
                 "$date": {
                     "$numberLong": "0"
                 }
             },
             "DatetimePositive": {
                 "$date": {
                     "$numberLong": "9223372036854775807"
                 }
             },
             "DatetimeNegative": {
                 "$date": {
                     "$numberLong": "-9223372036854775808"
                 }
             },
             "True": true,
             "False": false,
             "DBPointer": {
                 "$dbPointer": {
                     "$ref": "db.collection",
                     "$id": {
                         "$oid": "57e193d7a9cc81b4027498b1"
                     }
                 }
             },
             "Minkey": {
                 "$minKey": 1
             },
             "Maxkey": {
                 "$maxKey": 1
             },
             "Null": null
        })P");

        CHECK(document["_id"].get<realm::ObjectId>() == realm::ObjectId("57e193d7a9cc81b4027498b5"));
        CHECK(document["String"] == "string");
        CHECK(document["Int32"] == 42);
        CHECK(document["Int64"] == (int64_t)42);
        CHECK(document["Double"] == 42.42);
        CHECK(document["SpecialFloat"] == nan("NaN"));
        CHECK(document["Decimal"] == Decimal128(1234));
        CHECK(document["Timestamp"] == Timestamp(42, 0));
//        CHECK(document["Array"].get_array()[0] == 1);
//        CHECK(document["Array"].get_array()[1] == 2);
//        CHECK(document["Array"].get_array()[2] == 3);
//        CHECK(document["Array"].get_array()[3] == 4);
//        CHECK(document["Array"].get_array()[4] == 5);
//        CHECK(document["Array"].get_array()[0] == 1);
//        CHECK(document["Array"].get_array()[1] == 2);
//        CHECK(document["Array"].get_array()[2] == 3);
//        CHECK(document["Array"].get_array()[3] == 4);
//        CHECK(document["Array"].get_array()[4] == 5);
        CHECK(document["Subdocument"].get_document()["foo"] == "bar");

        std::cout<<document.to_json()<<std::endl;
//        auto reader = JsonReader("{ \"hi\": { \"$numberInt\": \"42\" } }");
//
//        reader.read_start_document();
//        auto type = reader.read_bson_type();
//        std::cout << bson_type_to_string(type) << std::endl;
//        auto name = reader.read_name();
//        std::cout << name << std::endl;
//        std::cout << realm::util::any_cast<int>(reader.current_value()) << std::endl;
    }
};

