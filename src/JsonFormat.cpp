// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: MIT

#include "JsonFormat.hpp"
#include <cctype>
#include <charconv>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

static void Indentation(std::string& buf, size_t level) { buf.append(4 * level, ' '); }

static void StdToString(std::string& buf, int val)
{
    constexpr int TMP_SIZE = 64;
    char tmp[TMP_SIZE];
    auto [ptr, ec] = std::to_chars(tmp, tmp + TMP_SIZE, val);
    buf.append(tmp, ptr - tmp);
}

static void StdToString(std::string& buf, double val)
{
    constexpr int TMP_SIZE = 64;
    char tmp[TMP_SIZE];
    auto [ptr, ec] = std::to_chars(tmp, tmp + TMP_SIZE, val);
    buf.append(tmp, ptr - tmp);
}

static int Stoi(std::string_view s)
{
    int val;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), val);
    if (ec == std::errc()) return val;
    throw std::runtime_error("Failed to convert " + std::string(s));
}

static double Stod(std::string_view s)
{
    double val;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), val);
    if (ec == std::errc()) return val;
    throw std::runtime_error("Failed to convert " + std::string(s));
}

static void SkipWhitespace(std::string_view s, size_t& idx)
{
    while (idx < s.size() && (isspace(s[idx]) != 0)) ++idx;
}

static Json ParseObject(std::string_view s, size_t& idx);
static Json ParseArray(std::string_view s, size_t& idx);
static Json ParseString(std::string_view s, size_t& idx);
static Json ParseNumber(std::string_view s, size_t& idx);

static Json ParseValue(std::string_view s, size_t& idx)
{
    SkipWhitespace(s, idx);
    if (idx >= s.size()) throw std::runtime_error("Unexpected end of input");

    if (s[idx] == '{') return ParseObject(s, idx);
    if (s[idx] == '[') return ParseArray(s, idx);
    if (s[idx] == '"') return ParseString(s, idx);
    if ((isdigit(s[idx]) != 0) || s[idx] == '-' || s[idx] == '+') return ParseNumber(s, idx);

    auto checkForToken = [&s, &idx](std::string_view token) -> bool
    {
        if (s.compare(idx, token.size(), token) == 0)
        {
            idx += token.size();
            return true;
        }
        return false;
    };

    if (checkForToken("true")) return {true};
    if (checkForToken("false")) return {false};
    if (checkForToken("null")) return {nullptr};

    throw std::runtime_error(std::string("Unexpected token: ") + s[idx]);
}

static Json ParseObject(std::string_view s, size_t& idx)
{
    ++idx; // skip '{'
    Json::object_t obj;
    SkipWhitespace(s, idx);
    if (idx < s.size() && s[idx] == '}')
    {
        ++idx;
        return obj;
    }

    while (true)
    {
        SkipWhitespace(s, idx);
        if (s[idx] != '"') throw std::runtime_error("Expected string key");
        std::string key = ParseString(s, idx).GetString();
        SkipWhitespace(s, idx);
        if (s[idx] != ':') throw std::runtime_error("Expected ':' after key");
        ++idx;
        obj[std::move(key)] = ParseValue(s, idx);
        SkipWhitespace(s, idx);
        if (s[idx] == '}')
        {
            ++idx;
            break;
        }
        if (s[idx] != ',') throw std::runtime_error("Expected ',' or '}'");
        ++idx;
    }
    return {obj};
}

static Json ParseArray(std::string_view s, size_t& idx)
{
    ++idx; // skip '['
    Json::array_t arr;
    SkipWhitespace(s, idx);
    if (idx < s.size() && s[idx] == ']')
    {
        ++idx;
        return arr;
    }

    while (true)
    {
        arr.push_back(ParseValue(s, idx));
        SkipWhitespace(s, idx);
        if (s[idx] == ']')
        {
            ++idx;
            break;
        }
        if (s[idx] != ',') throw std::runtime_error("Expected ',' or ']'");
        ++idx;
    }
    return {arr};
}

static Json ParseString(std::string_view s, size_t& idx)
{
    ++idx; // skip '"'
    std::string str;
    while (idx < s.size())
    {
        if (s[idx] == '"')
        {
            ++idx;
            break;
        }
        if (s[idx] == '\\')
        {
            ++idx;
            if (idx >= s.size()) throw std::runtime_error("Invalid escape sequence");
            switch (s[idx])
            {
            case '"':
                str.push_back('"');
                break;
            case '\\':
                str.push_back('\\');
                break;
            case '/':
                str.push_back('/');
                break;
            case 'b':
                str.push_back('\b');
                break;
            case 'f':
                str.push_back('\f');
                break;
            case 'n':
                str.push_back('\n');
                break;
            case 'r':
                str.push_back('\r');
                break;
            case 't':
                str.push_back('\t');
                break;
            default:
                throw std::runtime_error("Unknown escape character");
            }
        }
        else
            str.push_back(s[idx]);
        ++idx;
    }
    return {str};
}

static Json ParseNumber(std::string_view s, size_t& idx)
{
    size_t start = idx;
    if (s[idx] == '-' || s[idx] == '+') ++idx;

    bool isDouble = false;

    while (idx < s.size())
    {
        char c = s[idx];
        if (isdigit(c) != 0)
        {
            ++idx;
        }
        else if (c == '.')
        {
            if (isDouble) throw std::runtime_error("Invalid number: multiple decimals");
            isDouble = true;
            ++idx;
        }
        else if (c == 'e' || c == 'E')
        {
            isDouble = true;
            ++idx;
            if (idx < s.size() && (s[idx] == '+' || s[idx] == '-')) ++idx;
            if (idx >= s.size() || (isdigit(s[idx]) == 0))
                throw std::runtime_error("Invalid number: exponent missing digits");
            while (idx < s.size() && (isdigit(s[idx]) != 0)) ++idx;
        }
        else
        {
            break;
        }
    }

    std::string_view numStr = s.substr(start, idx - start);

    try
    {
        if (isDouble)
        {
            // If it has decimal point or exponent - store as double
            double d = Stod(numStr);
            return {d};
        }

        // Try parsing as int first
        int n = Stoi(numStr);
        if (n >= INT_MIN && n <= INT_MAX) return {n}; // store as int if fits
        return {static_cast<double>(n)};              // store as double if too big
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("Invalid number: ") + e.what());
    }
}

static void EscapeString(std::string& out, std::string_view s)
{
    for (char c: s)
    {
        switch (c)
        {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\b':
            out += "\\b";
            break;
        case '\f':
            out += "\\f";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            out += c;
            break;
        }
    }
}

void Json::ToString(std::string& buf, size_t level) const
{
    if (IsNull())
        buf += "null";
    else if (IsBool())
        buf += (std::get<bool>(value_) ? "true" : "false");
    else if (IsInt())
        StdToString(buf, std::get<int>(value_));
    else if (IsDouble())
        StdToString(buf, std::get<double>(value_));
    else if (IsString())
    {
        buf += "\"";
        EscapeString(buf, std::get<std::string>(value_));
        buf += "\"";
    }
    else if (IsArray())
    {
        const auto& arr = std::get<array_t>(value_);
        buf += "[";
        if (format == JsonFormat::Newline) buf += '\n';
        for (size_t i = 0; i < arr.size(); ++i)
        {
            if (format == JsonFormat::Newline) Indentation(buf, level + 1);
            arr[i].ToString(buf, level + 1);
            if (i < arr.size() - 1) buf += ", ";
            if (format == JsonFormat::Newline) buf += '\n';
        }
        if (format == JsonFormat::Newline) Indentation(buf, level);
        buf += "]";
    }
    else if (IsObject())
    {
        const auto& obj = std::get<object_t>(value_);
        buf += "{";
        if (format == JsonFormat::Newline) buf += '\n';
        size_t count = 0;
        for (const auto& [k, v]: obj)
        {
            if (format == JsonFormat::Newline) Indentation(buf, level + 1);
            buf += "\"";
            EscapeString(buf, k);
            buf += "\": ";
            v.ToString(buf, level + 1);
            if (count++ < obj.size() - 1) buf += ", ";
            if (format == JsonFormat::Newline) buf += '\n';
        }
        if (format == JsonFormat::Newline) Indentation(buf, level);
        buf += "}";
    }
}

std::string Json::ToString(size_t level) const
{
    constexpr int STR_START_SIZE = 4 * 1024 * 1024;
    std::string str;
    str.reserve(STR_START_SIZE);
    ToString(str, level);
    return str;
}

void Json::Save(const std::filesystem::path& path) const
{
    std::ofstream f(path);
    if (!f) throw std::runtime_error("Cannot open file: " + path.string());
    f << ToString();
    f.close();
}

Json Json::Load(const std::filesystem::path& path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Cannot open file: " + path.string());

    uintmax_t size = std::filesystem::file_size(path);
    if (size <= 0) return {};

    std::string s(size, '\0');
    if (f.read(s.data(), static_cast<std::streamsize>(size)))
    {
        return Parse(s);
    }

    throw std::runtime_error("Failed to read file: " + path.string());
}

Json Json::Parse(std::string_view json)
{
    size_t idx = 0;
    return ParseValue(json, idx);
}
