// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: MIT

#define JSON_FORMAT_STRICT_ERRORS
#include <JsonFormat.hpp>
#include <gtest/gtest.h>

const std::string G_FILE = "file.json";

class Saving : public testing::Test
{
  protected:
    void SetUp() override
    {
        if (std::filesystem::exists(G_FILE)) std::filesystem::remove(G_FILE);

        json_["null"] = nullptr;
        json_["bool"] = true;
        json_["int"] = 5;       // NOLINT (readability-magic-numbers)
        json_["double"] = 1.23; // NOLINT (readability-magic-numbers)
        json_["string"] = "str";
        json_["array"] = Json::array_t();
        json_["array"].format = JsonFormat::Inline;
        json_["array"].push_back(1);
        json_["array"].push_back("2");
        json_["array"].push_back(true);
        json_["object"]["nested"] = true;
        json_["object"]["nestedValue"] = "abc";
        json_.Save(G_FILE);
    }
    Json json_;
};

TEST_F(Saving, Formatting)
{
    EXPECT_EQ(json_.ToString(),
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
    void SetUp() override { json_ = Json::Load(G_FILE); }
    Json json_;
};

TEST_F(Loading, TypeQueries)
{
    EXPECT_TRUE(json_["null"].IsNull());
    EXPECT_TRUE(json_["bool"].IsBool());
    EXPECT_TRUE(json_["int"].IsInt());
    EXPECT_TRUE(json_["double"].IsDouble());
    EXPECT_TRUE(json_["string"].IsString());
    EXPECT_TRUE(json_["array"].IsArray());
    EXPECT_TRUE(json_["object"].IsObject());
    EXPECT_FALSE(json_["object"].IsNull());
}

TEST_F(Loading, Queries)
{
    EXPECT_EQ(json_["bool"], true);
    EXPECT_EQ(json_["int"], 5);
    EXPECT_EQ(json_["double"], 1.23);
    EXPECT_EQ(json_["string"], "str");
    EXPECT_EQ(json_["array"][0], 1);
    EXPECT_EQ(json_["array"][1], "2");
    EXPECT_EQ(json_["array"][2], true);
    EXPECT_EQ(json_["object"]["nested"], true);
    EXPECT_EQ(json_["object"]["nestedValue"], "abc");
}

TEST_F(Loading, StrictErrors)
{
    EXPECT_THROW(json_["bool"].GetString(), std::runtime_error);
    EXPECT_THROW(json_["null"].GetArray(), std::runtime_error);
    EXPECT_NO_THROW(json_["int"].GetInt());
}
