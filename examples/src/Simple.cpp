// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: MIT

#include <JsonFormat.hpp>
#include <iostream>

void Load()
{
    // Load a json file
    Json json = Json::Load("resources/in.json");

    // Print the loaded json
    std::cout << "Loaded json:\n";
    std::cout << json.ToString() << '\n';

    // Print an individual entry of the loaded json
    std::cout << "loadedJson[\"string\"] = " << json["string"].GetString() << '\n';
}

void Save()
{
    // Save data to a json file
    Json json;
    json["number"] = 3;
    json["none"] = nullptr;

    // Create an inline array
    json["array"] = Json::array_t();
    json["array"].format = JsonFormat::Inline;
    json["array"].push_back(1);
    json["array"].push_back("2");
    json["array"].push_back(true);

    // Save the created json
    json.Save("out.json");

    // Print the saved json
    std::cout << "Saved json:\n";
    std::cout << json.ToString() << '\n';
}

int main()
{
#ifdef JSON_FORMAT_STRICT_ERRORS
    std::cout << "Strict errors are enabled!\n";
#endif

    Load();
    Save();

    return 0;
}
