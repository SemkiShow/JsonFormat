// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: MIT
/**
 * @file
 * @brief A json library with individual json object formatting support
 */

#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

/**
 * @brief Formatting options for a json object
 */
enum class JsonFormat
{
    Newline,
    Inline
};

/**
 * @brief The json object class
 *
 */
class Json
{
  public:
    using array_t = std::vector<Json>;
    using object_t = std::unordered_map<std::string, Json>;
    using value_t = std::variant<std::nullptr_t, bool, int, double, std::string, array_t, object_t>;

  private:
    value_t value_;

  public:
    Json() : value_(nullptr) {}
    Json(std::nullptr_t) : value_(nullptr) {}
    Json(bool b) : value_(b) {}
    Json(int n) : value_(n) {}
    Json(double n) : value_(n) {}
    Json(const char* s) : value_(std::string(s)) {}
    Json(const std::string& s) : value_(s) {}
    Json(const array_t& a) : value_(a) {}
    Json(const object_t& o) : value_(o) {}
    Json(JsonFormat format) : value_(nullptr), format(format) {}

    JsonFormat format = JsonFormat::Newline;

    // Type queries
    bool IsNull() const { return std::holds_alternative<std::nullptr_t>(value_); }
    bool IsBool() const { return std::holds_alternative<bool>(value_); }
    bool IsInt() const { return std::holds_alternative<int>(value_); }
    bool IsDouble() const { return std::holds_alternative<double>(value_); }
    bool IsString() const { return std::holds_alternative<std::string>(value_); }
    bool IsArray() const { return std::holds_alternative<array_t>(value_); }
    bool IsObject() const { return std::holds_alternative<object_t>(value_); }

    // Queries
    bool GetBool() const
    {
#ifndef JSON_FORMAT_STRICT_ERRORS
        if (IsNull()) return false;
#endif
        if (!IsBool()) throw std::runtime_error("JsonValue is not a bool");
        return std::get<bool>(value_);
    }

    int GetInt() const
    {
        if (IsInt()) return std::get<int>(value_);
        if (IsDouble())
        {
            double d = std::get<double>(value_);
            if (d == static_cast<int>(d)) return static_cast<int>(d);
        }
#ifndef JSON_FORMAT_STRICT_ERRORS
        if (IsNull()) return 0;
#endif
        throw std::runtime_error("JsonValue is not an int");
    }

    double GetDouble() const
    {
        if (IsInt()) return GetInt();
#ifndef JSON_FORMAT_STRICT_ERRORS
        if (IsNull()) return 0;
#endif
        if (!IsDouble()) throw std::runtime_error("JsonValue is not a double");
        return std::get<double>(value_);
    }

    std::string GetString() const
    {
#ifndef JSON_FORMAT_STRICT_ERRORS
        if (IsNull()) return "";
#endif
        if (!IsString()) throw std::runtime_error("JsonValue is not a string");
        return std::get<std::string>(value_);
    }

    array_t& GetArray()
    {
#ifndef JSON_FORMAT_STRICT_ERRORS
        if (IsNull()) value_ = array_t{};
#endif
        if (!IsArray()) throw std::runtime_error("JsonValue is not an array");
        return std::get<array_t>(value_);
    }

    object_t& GetObject()
    {
#ifndef JSON_FORMAT_STRICT_ERRORS
        if (IsNull()) value_ = object_t{};
#endif
        if (!IsObject()) throw std::runtime_error("JsonValue is not an object");
        return std::get<object_t>(value_);
    }

    const array_t& GetArray() const
    {
        if (!IsArray()) throw std::runtime_error("JsonValue is not an array");
        return std::get<array_t>(value_);
    }

    const object_t& GetObject() const
    {
        if (!IsObject()) throw std::runtime_error("JsonValue is not an object");
        return std::get<object_t>(value_);
    }

    // Access
    void push_back(const Json& element) // NOLINT (readability-identifier-naming)
    {
#ifndef JSON_FORMAT_STRICT_ERRORS
        if (IsNull()) value_ = array_t{};
#endif
        if (!IsArray()) throw std::runtime_error("Cannot push_back to non-array Json");
        GetArray().push_back(element);
    }

    void emplace_back(const Json& element) // NOLINT (readability-identifier-naming)
    {
#ifndef JSON_FORMAT_STRICT_ERRORS
        if (IsNull()) value_ = array_t{};
#endif
        if (!IsArray()) throw std::runtime_error("Cannot emplace_back to non-array Json");
        GetArray().emplace_back(element);
    }

    Json& back() // NOLINT (readability-identifier-naming)/
    {
#ifndef JSON_FORMAT_STRICT_ERRORS
        if (IsNull()) value_ = array_t{};
#endif
        if (!IsArray()) throw std::runtime_error("Cannot push_back to non-array Json");
        return GetArray().back();
    }

    size_t size() // NOLINT (readability-identifier-naming)
    {
        if (!IsArray() && !IsObject()) return 0;
        return IsArray() ? GetArray().size() : GetObject().size();
    }

    Json& operator[](const std::string& key)
    {
        if (!IsObject()) value_ = object_t{};
        return std::get<object_t>(value_)[key];
    }

    const Json& operator[](const std::string& key) const
    {
        if (!IsObject()) throw std::runtime_error("Not an object");
        const auto& obj = std::get<object_t>(value_);
        auto it = obj.find(key);
        if (it == obj.end()) throw std::out_of_range("Key not found");
        return it->second;
    }

    Json& operator[](size_t index)
    {
        if (!IsArray()) value_ = array_t{};
        auto& arr = std::get<array_t>(value_);
        if (index >= arr.size()) arr.resize(index + 1);
        return arr[index];
    }

    const Json& operator[](size_t index) const
    {
        if (!IsArray()) throw std::runtime_error("Not an array");
        const auto& arr = std::get<array_t>(value_);
        if (index >= arr.size()) throw std::out_of_range("Index out of range");
        return arr[index];
    }

    // Assignment
    Json& operator=(std::nullptr_t)
    {
        value_ = nullptr;
        return *this;
    }

    Json& operator=(bool b)
    {
        value_ = b;
        return *this;
    }

    Json& operator=(int n)
    {
        value_ = n;
        return *this;
    }

    Json& operator=(double n)
    {
        value_ = n;
        return *this;
    }

    Json& operator=(const char* s)
    {
        value_ = std::string(s);
        return *this;
    }

    Json& operator=(const std::string& s)
    {
        value_ = s;
        return *this;
    }

    Json& operator=(const array_t& a)
    {
        value_ = a;
        return *this;
    }

    Json& operator=(const object_t& o)
    {
        value_ = o;
        return *this;
    }

    void Save(const std::filesystem::path& path) const;
    [[nodiscard]] static Json Load(const std::filesystem::path& path);
    [[nodiscard]] static Json Parse(const std::string& json);
    std::string ToString(size_t level = 0) const;

  private:
    static void SkipWhitespace(const std::string& s, size_t& idx);
    static Json ParseValue(const std::string& s, size_t& idx);
    static Json ParseObject(const std::string& s, size_t& idx);
    static Json ParseArray(const std::string& s, size_t& idx);
    static Json ParseString(const std::string& s, size_t& idx);
    static Json ParseNumber(std::string_view s, size_t& idx);
    static void EscapeString(std::string& out, const std::string& s);
    void ToString(std::string& buf, size_t level = 0) const;
};
