// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: MIT

#define JSON_FORMAT_STRICT_ERRORS
#include <JsonFormat.hpp>
#include <gtest/gtest.h>

const std::string file = "file.json";

class Saving : public testing::Test
{
  protected:
    void SetUp() override
    {
        if (std::filesystem::exists(file)) std::filesystem::remove(file);

        json["null"] = nullptr;
        json["bool"] = true;
        json["int"] = 5;
        json["double"] = 1.23;
        json["string"] = "str";
        json["array"] = Json::array_t();
        json["array"].format = JsonFormat::Inline;
        json["array"].push_back(1);
        json["array"].push_back("2");
        json["array"].push_back(true);
        json["object"]["nested"] = true;
        json["object"]["nestedValue"] = "abc";
        json.Save(file);
    }
    Json json;
};

TEST_F(Saving, Formatting)
{
    EXPECT_EQ(json.ToString(),
              R"({
    "object": {
        "nestedValue": "abc", 
        "nested": true
    }, 
    "array": [1, "2", true], 
    "string": "str", 
    "double": 1.23, 
    "int": 5, 
    "bool": true, 
    "null": null
})");
}

class Loading : public testing::Test
{
  protected:
    void SetUp() override { json = Json::Load(file); }
    Json json;
};

TEST_F(Loading, TypeQueries)
{
    EXPECT_TRUE(json["null"].IsNull());
    EXPECT_TRUE(json["bool"].IsBool());
    EXPECT_TRUE(json["int"].IsInt());
    EXPECT_TRUE(json["double"].IsDouble());
    EXPECT_TRUE(json["string"].IsString());
    EXPECT_TRUE(json["array"].IsArray());
    EXPECT_TRUE(json["object"].IsObject());
    EXPECT_FALSE(json["object"].IsNull());
}

TEST_F(Loading, Queries)
{
    EXPECT_EQ(json["bool"].GetBool(), true);
    EXPECT_EQ(json["int"].GetInt(), 5);
    EXPECT_EQ(json["double"].GetDouble(), 1.23);
    EXPECT_EQ(json["string"].GetString(), "str");
    EXPECT_EQ(json["array"][0].GetInt(), 1);
    EXPECT_EQ(json["array"][1].GetString(), "2");
    EXPECT_EQ(json["array"][2].GetBool(), true);
    EXPECT_EQ(json["object"]["nested"].GetBool(), true);
    EXPECT_EQ(json["object"]["nestedValue"].GetString(), "abc");
}

TEST_F(Loading, StrictErrors)
{
    EXPECT_THROW(json["bool"].GetString(), std::runtime_error);
    EXPECT_THROW(json["null"].GetArray(), std::runtime_error);
    EXPECT_NO_THROW(json["int"].GetInt());
}
