#include "stdafx.h"
#include "strings.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

namespace cxx
{

int f_snprintf(char* buffer_string, int buffer_size, const char* format_string, ...)
{
    va_list vaList {};
    va_start(vaList, format_string);
    int num_formatted = stbsp_vsnprintf(buffer_string, buffer_size, format_string, vaList);
    va_end(vaList);
    return num_formatted;
}

int f_vsnprintf(char* buffer_string, int buffer_size, const char* format_string, va_list args_list)
{
    int num_formatted = stbsp_vsnprintf(buffer_string, buffer_size, format_string, args_list);
    return num_formatted;
}

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

static const char* eat_quoted_string(const char* cursor, string_buffer& outputString)
{
    cursor = eat_whitespaces(cursor);

    outputString.clear();
    if (cursor == nullptr || *cursor == 0)
        return false;

    if (*cursor != '\"')
        return cursor;

    ++cursor;
    while (*cursor)
    {
        if (*cursor == '\"')
            break;

        outputString.append_char(*cursor);
        ++cursor;
    }

    if (*cursor == '\"')
    {
        ++cursor;
        return cursor;
    }

    return nullptr;
}

static const char* eat_argument(const char* cursor, string_buffer& outputString)
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

        outputString.append_char(*cursor);
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