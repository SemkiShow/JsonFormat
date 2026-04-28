// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: MIT
/**
 * @file
 * @brief A json library with individual json object formatting support
 */

#pragma once

#include <cstddef>
#include <filesystem>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
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
    template <typename T>
    using is_json_type = std::disjunction<std::is_same<T, std::nullptr_t>, std::is_same<T, bool>,
                                          std::is_same<T, int>, std::is_same<T, double>,
                                          std::is_same<T, std::string>, std::is_same<T, array_t>,
                                          std::is_same<T, object_t>>;

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
        if (!IsBool()) throw std::runtime_error("JsonValue is not a bool" + ToString());
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
        throw std::runtime_error("JsonValue is not an int" + ToString());
    }

    double GetDouble() const
    {
        if (IsInt()) return GetInt();
#ifndef JSON_FORMAT_STRICT_ERRORS
        if (IsNull()) return 0;
#endif
        if (!IsDouble()) throw std::runtime_error("JsonValue is not a double" + ToString());
        return std::get<double>(value_);
    }

    std::string GetString() const
    {
#ifndef JSON_FORMAT_STRICT_ERRORS
        if (IsNull()) return "";
#endif
        if (!IsString()) throw std::runtime_error("JsonValue is not a string" + ToString());
        return std::get<std::string>(value_);
    }

    array_t& GetArray()
    {
#ifndef JSON_FORMAT_STRICT_ERRORS
        if (IsNull()) value_ = array_t{};
#endif
        if (!IsArray()) throw std::runtime_error("JsonValue is not an array" + ToString());
        return std::get<array_t>(value_);
    }

    object_t& GetObject()
    {
#ifndef JSON_FORMAT_STRICT_ERRORS
        if (IsNull()) value_ = object_t{};
#endif
        if (!IsObject()) throw std::runtime_error("JsonValue is not an object" + ToString());
        return std::get<object_t>(value_);
    }

    const array_t& GetArray() const
    {
        if (!IsArray()) throw std::runtime_error("JsonValue is not an array" + ToString());
        return std::get<array_t>(value_);
    }

    const object_t& GetObject() const
    {
        if (!IsObject()) throw std::runtime_error("JsonValue is not an object" + ToString());
        return std::get<object_t>(value_);
    }

    // Access
    void push_back(const Json& element) // NOLINT (readability-identifier-naming)
    {
#ifndef JSON_FORMAT_STRICT_ERRORS
        if (IsNull()) value_ = array_t{};
#endif
        if (!IsArray()) throw std::runtime_error("Cannot push_back to non-array Json" + ToString());
        GetArray().push_back(element);
    }

    void emplace_back(const Json& element) // NOLINT (readability-identifier-naming)
    {
#ifndef JSON_FORMAT_STRICT_ERRORS
        if (IsNull()) value_ = array_t{};
#endif
        if (!IsArray())
            throw std::runtime_error("Cannot emplace_back to non-array Json" + ToString());
        GetArray().emplace_back(element);
    }

    Json& back() // NOLINT (readability-identifier-naming)/
    {
#ifndef JSON_FORMAT_STRICT_ERRORS
        if (IsNull()) value_ = array_t{};
#endif
        if (!IsArray()) throw std::runtime_error("Cannot push_back to non-array Json" + ToString());
        return GetArray().back();
    }

    size_t size() const // NOLINT (readability-identifier-naming)
    {
        if (IsString()) return GetString().size();
        if (IsArray()) return GetArray().size();
        if (IsObject()) return GetObject().size();
        throw std::runtime_error("Json type doesn't have size(): " + ToString());
    }

    bool empty() // NOLINT (readability-identifier-naming)
    {
        if (!IsArray())
            throw std::runtime_error("Cannot call empty on a non-array Json: " + ToString());
        return GetArray().empty();
    }

    Json& operator[](const std::string& key)
    {
        if (!IsObject()) value_ = object_t{};
        return std::get<object_t>(value_)[key];
    }

    Json& operator[](const char* key)
    {
        if (!IsObject()) value_ = object_t{};
        return std::get<object_t>(value_)[key];
    }

    Json& operator[](int idx)
    {
        if (!IsArray()) value_ = array_t{};
        return std::get<array_t>(value_)[idx];
    }

    Json& operator[](size_t index)
    {
        if (!IsArray()) value_ = array_t{};
        auto& arr = std::get<array_t>(value_);
        if (index >= arr.size()) throw std::out_of_range("Index out of range");
        return arr[index];
    }

    const Json& operator[](size_t index) const
    {
        if (!IsArray()) throw std::runtime_error("Not an array" + ToString());
        const auto& arr = std::get<array_t>(value_);
        if (index >= arr.size()) throw std::out_of_range("Index out of range");
        return arr[index];
    }

    template <typename T, typename = std::enable_if_t<is_json_type<std::decay_t<T>>::value>>
    Json& operator=(T&& v)
    {
        value_ = std::forward<T>(v);
        return *this;
    }

    template <typename T, typename = std::enable_if_t<is_json_type<T>::value>>
    operator T() const
    {
        return std::get<T>(value_);
    }

    template <typename T, typename = std::enable_if_t<is_json_type<T>::value>>
    bool operator==(const T& other) const
    {
        if (auto* val = std::get_if<T>(&value_))
        {
            return *val == other;
        }
        return false;
    }

    bool operator==(const char* other) const
    {
        if (const std::string* val = std::get_if<std::string>(&value_))
        {
            return *val == other;
        }
        return false;
    }

    template <typename T>
    bool operator!=(const T& other) const
    {
        return !(*this == other);
    }

    bool operator!=(const char* other) const { return !(*this == other); }

    void Save(const std::filesystem::path& path) const;
    [[nodiscard]] static Json Load(const std::filesystem::path& path);
    [[nodiscard]] static Json Parse(std::string_view json);
    std::string ToString(size_t level = 0) const;

  private:
    void ToString(std::string& buf, size_t level = 0) const;
};

inline std::ostream& operator<<(std::ostream& os, const Json& json)
{
    os << json.ToString();
    return os;
}
