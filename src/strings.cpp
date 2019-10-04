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
    if (string_a == nullptr || suffix_string == nullptr)
        return false;

    debug_assert(!"not implemented");// todo
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
    trim_left(input_string);
    trim_right(input_string);
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
        return cursor;

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

//////////////////////////////////////////////////////////////////////////

string_buffer::string_buffer(char* buffer_begin, char* buffer_end, int buffer_capacity)
    : mBufferPtr(buffer_begin)
    , mBufferEnd(buffer_end)
    , mBufferCapacity(buffer_capacity)
{
    debug_assert(mBufferPtr);
    clear();
}

void string_buffer::clear()
{
    debug_assert(mBufferPtr);
    if (mBufferCapacity > 0)
    {
        mBufferEnd = mBufferPtr;
        *mBufferPtr = 0;
    }
}

void string_buffer::append_char(char c)
{
    int currLength = get_length();
    if (currLength < mBufferCapacity)
    {
        mBufferEnd[0] = c;
        mBufferEnd[1] = 0;
        ++mBufferEnd;
    }
}

void string_buffer::pop_back()
{
    if (mBufferPtr == mBufferEnd)
    {
        debug_assert(false);
        return;
    }
    --mBufferEnd;
    mBufferEnd[0] = 0;
}

void string_buffer::append_string(const char* sourceString)
{
    if (mBufferPtr == sourceString)
        return;

    for (;sourceString && *sourceString; ++sourceString)
    {
        int currLength = get_length();
        if (currLength < mBufferCapacity)
        {
            mBufferEnd[0] = *sourceString;
            mBufferEnd[1] = 0;
            ++mBufferEnd;
            continue;
        }
        break;
    }
}

void string_buffer::set_content(const char* sourceString, int length)
{
    if (mBufferPtr == sourceString)
        return;

    clear();

    if (length == 0)
    {
        append_string(sourceString);
        return;
    }

    for (int i = 0; i < length; ++i)
    {
        int currLength = get_length();
        if (currLength < mBufferCapacity)
        {
            mBufferEnd[0] = sourceString[i];
            mBufferEnd[1] = 0;
            ++mBufferEnd;
            continue;
        }
        break;
    }        
}

int string_buffer::printf(const char* szFormatString, ...)
{
    debug_assert(mBufferPtr);

    clear();

    // formatted print routine
    va_list parguments;
    va_start(parguments, szFormatString);
    int icounter = f_vsnprintf(mBufferPtr, mBufferCapacity + 1, szFormatString, parguments);
    if (icounter > 0)
    {
        mBufferEnd = mBufferPtr + (icounter > mBufferCapacity ? mBufferCapacity : icounter);
    }
    va_end(parguments);
    return icounter;
}

void string_buffer::trim_left()
{
    // todo
}

void string_buffer::trim_right()
{
    // todo
}

void string_buffer::trim()
{
    trim_left();
    trim_right();
}

} // namespace cxx