#include "stdafx.h"
#include "strings.h"

namespace cxx
{

int count_common_prefix_length(const char* string_a, const char* string_b)
{
    if (string_a == nullptr || string_b == nullptr)
        return 0;

    int current_length = 0;
    for (int icursor = 0;; ++icursor, ++current_length)
    {
        if (string_a[icursor] == 0 || string_b[icursor] == 0 ||
            string_a[icursor] != string_b[icursor])
        {
            break;
        }
    }
    return current_length;
}

bool has_prefix(const char* string_a, const char* prefix_string)
{
    if (string_a == nullptr || *string_a == 0 || 
        prefix_string == nullptr ||  *prefix_string == 0)
    {
        return false;
    }

    for (;; ++string_a, ++prefix_string)
    {
        char currc = *string_a;
        char currp = *prefix_string;

        if (currp != currc)
            return currp == 0;
    }
    return false;
}

bool has_suffix(const char* string_a, const char* suffix_string)
{
    if (string_a == nullptr || *string_a == 0 || 
        suffix_string == nullptr ||  *suffix_string == 0)
    {
        return false;
    }

    for (; *string_a; ++string_a)
    {
        if (strcmp(string_a, suffix_string) == 0)
            return true;
    }
    return false;
}

bool has_prefix_icase(const char* string_a, const char* prefix_string)
{
    if (string_a == nullptr || *string_a == 0 || 
        prefix_string == nullptr ||  *prefix_string == 0)
    {
        return false;
    }
    for (;; ++string_a, ++prefix_string)
    {
        char currc = ::toupper(*string_a);
        char currp = ::toupper(*prefix_string);

        if (currp != currc)
            return currp == 0;
    }
    return false;
}

bool has_suffix_icase(const char* string_a, const char* suffix_string)
{
    if (string_a == nullptr || suffix_string == nullptr)
        return false;

    debug_assert(!"not implemented");// todo
    return false;
}

const char* va(const char* format_string, ...)
{
    va_list argptr;

    static int scope_index = 0;
    static char string_buffers[4][16384]; // in case called by nested functions

    char *current_buffer = string_buffers[scope_index];
    scope_index = (scope_index + 1) & 3;

    va_start(argptr, format_string);
    vsprintf(current_buffer, format_string, argptr);
    va_end(argptr);

    return current_buffer;
}

void trim_left(std::string& input_string)
{
    input_string.erase(input_string.begin(), std::find_if(input_string.begin(), input_string.end(), 
        [](char ch) 
        {
            return !is_space(ch);
        }));
}

void trim_right(std::string& input_string)
{
    input_string.erase(std::find_if(input_string.rbegin(), input_string.rend(), 
        [](char ch) 
        {
            return !is_space(ch);
        }).base(), input_string.end());
}

void trim(std::string& input_string)
{
    trim_right(input_string);
    trim_left(input_string);
}

static const char* eat_whitespaces(const char* cursor)
{
    if (cursor == nullptr)
        return cursor;

    while (*cursor)
    {
        if (*cursor == ' ' || *cursor == '\t' || *cursor == '\r' || *cursor == '\n')
        {
            ++cursor;
            continue;
        }
        break;
    }
    return cursor;
}

static const char* eat_quoted_string(const char* cursor, std::string& outputString)
{
    cursor = eat_whitespaces(cursor);

    outputString.clear();
    if (cursor == nullptr || *cursor == 0)
        return cursor;

    if (*cursor != '\"')
        return cursor;

    ++cursor;
    while (*cursor)
    {
        if (*cursor == '\"')
            break;

        outputString.push_back(*cursor);
        ++cursor;
    }

    if (*cursor == '\"')
    {
        ++cursor;
        return cursor;
    }

    return nullptr;
}

static const char* eat_argument(const char* cursor, std::string& outputString)
{
    cursor = eat_whitespaces(cursor);

    outputString.clear();
    if (cursor == nullptr || *cursor == 0)
        return nullptr;

    // parse quoted string
    if (*cursor == '\"')
        return eat_quoted_string(cursor, outputString);

    while (*cursor)
    {
        if (*cursor == ' ' || *cursor == '\t' || *cursor == '\r' || *cursor == '\n' || *cursor == ',' ||
            *cursor == ';' || *cursor == ':' || *cursor == '=' || *cursor == '<' || *cursor == '>')
        {
            break;
        }

        outputString.push_back(*cursor);
        ++cursor;
    }
    return cursor;
}

bool arguments_parser::parse_next(int& output)
{
    mArgsCursor = eat_argument(mArgsCursor, mContent);
    if (mArgsCursor)
    {
        int numscan = std::sscanf(mContent.c_str(), "%d", &output);
        return numscan == 1;
    }
    return false;
}

bool arguments_parser::parse_next(float& output)
{
    mArgsCursor = eat_argument(mArgsCursor, mContent);
    if (mArgsCursor)
    {
        int numscan = std::sscanf(mContent.c_str(), "%f", &output);
        return numscan == 1;
    }
    return false;
}

bool arguments_parser::parse_next(bool& output)
{
    mArgsCursor = eat_argument(mArgsCursor, mContent);
    if (mArgsCursor)
    {
        int value = 0;
        int numscan = std::sscanf(mContent.c_str(), "%d", &value);
        if (numscan == 1)
        {
            output = value != 0;
        }
        return numscan == 1;
    }
    return false;
}

bool arguments_parser::parse_next_string()
{
    mArgsCursor = eat_argument(mArgsCursor, mContent);
    return mArgsCursor != nullptr;
}

} // namespace cxx