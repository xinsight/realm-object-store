#include "catch2/catch.hpp"
#include <stdio.h>
#include <json.hpp>
#include "util/test_utils.hpp"
#include "util/test_file.hpp"
#include "util/bson.hpp"
#include "util/extended_json.hpp"
#include <variant>

using namespace nlohmann;
using namespace realm;
using namespace bson;

TEST_CASE("extjson", "[util]") {
    SECTION("$numberInt") {
        
    }

    SECTION("basics") {
        auto document = bson::parse(R"P(
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
            "Array": [
                {"$numberInt": "1"},
                {"$numberInt": "2"},
                {"$numberInt": "3"},
                {"$numberInt": "4"},
                {"$numberInt": "5"}
            ],
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
             "Minkey": {
                 "$minKey": 1
             },
             "Maxkey": {
                 "$maxKey": 1
             },
             "Null": null
        })P");

//        auto get = [](const BsonDocument& document, const std::string& key) {
//            auto it = std::find_if(document.begin(),
//                                document.end(),
//                                [key](std::pair<std::string, bson::Bson> pair) {
//                return pair.first == key;
//            });
//            if (it != document.end()) {
//                return (*it).second;
//            }
//            return Bson();
//        };

        
//        std::find_if(document.begin(), document.end(), [](std::pair<std::string, bson::Bson> pair)
//                     { return pair.first == "_id"; });
        CHECK(std::get<realm::ObjectId>(document["_id"]) == realm::ObjectId("57e193d7a9cc81b4027498b5"));
        auto subdocument = std::get<BsonDocument>(document["Subdocument"]);
        std::cout<<std::get<StringData>(subdocument["foo"])<<std::endl;
        CHECK(std::get<StringData>(std::get<BsonDocument>(document["Subdocument"])["foo"]) == "bar");
//        CHECK(std::get<std::string>(document["String"]) == StringData("string"));
//        CHECK(std::get<int32_t>(document["Int32"]) == 42);
//        CHECK(std::get<int64_t>(document["Int64"]) == (int64_t)42);
//        CHECK(std::get<double>(document["Double"]) == 42.42);
//        CHECK(std::get<float>(document["SpecialFloat"]) == nan("NaN"));
//        CHECK(std::get<realm::Decimal128>(document["Decimal"]) == Decimal128(1234));
//        CHECK(std::get<realm::Timestamp>(document["Timestamp"]) == Timestamp(42, 0));
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
//        CHECK(std::get<std::vector<std::pair<std::string, Bson>>>(document["Subdocument"])["foo"] == "bar");

        std::cout<<bson::to_json(document)<<std::endl;
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

