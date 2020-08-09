#pragma once

namespace cxx
{
    // ignore case string comparators
    struct icase_eq
    {
        inline bool operator () (const std::string& astring, const std::string& bstring) const 
        {
            if (astring.length() != bstring.length())
                return false;

            const int iresult = cxx_stricmp(astring.c_str(), bstring.c_str());
            return iresult == 0;
        }
    };

    struct icase_less
    {
        inline bool operator () (const std::string& astring, const std::string& bstring) const 
        {
            const int iresult = cxx_stricmp(astring.c_str(), bstring.c_str());
            return iresult < 0;
        }
    };

    struct icase_hashfunc
    {
        enum 
        { 
            FNV_OFFSET_BASIS = 2166136261U,
            FNV_PRIME = 16777619U
        };

        inline unsigned int operator() (const std::string& theString) const
        {
	        unsigned int result_value = FNV_OFFSET_BASIS;
            for (char c: theString) 
            {
		        result_value ^= tolower(c);
		        result_value *= FNV_PRIME;
            }
	        return result_value;
        }
    };

    // does a varargs printf into a temp buffer, not thread safe
    const char* va(const char *format_string, ...);

    // simple tokenizer for strings
    struct string_tokenizer
    {
    public:
        string_tokenizer(const std::string& source_string)
            : mSourceStringStream(source_string)
        {
        }
        bool get_next(std::string& output_string, char delimiter = '/')
        {
            output_string.clear();
            if (std::getline(mSourceStringStream, output_string, delimiter))
                return true;

            return false;
        }
    private:
        std::stringstream mSourceStringStream;
    };

    //////////////////////////////////////////////////////////////////////////

    struct arguments_parser
    {
    public:
        arguments_parser(const char* args) : mArgsCursor(args)
        { 
        }
        // Parse next argument from input stream
        // @param output: Result
        bool parse_next(int& output);
        bool parse_next(float& output);
        bool parse_next(bool& output);
        bool parse_next_string();

    public:
        std::string mContent;
        const char* mArgsCursor;
    };

    //////////////////////////////////////////////////////////////////////////

    // get length of common prefix in two strings
    int count_common_prefix_length(const char* string_a, const char* string_b);
    
    // test whether string has specific suffix or prefix, case sensitive
    bool has_prefix(const char* string_a, const char* prefix_string);
    bool has_suffix(const char* string_a, const char* suffix_string);

    // test whether string has specific suffix or prefix, ignore case
    bool has_prefix_icase(const char* string_a, const char* prefix_string);
    bool has_suffix_icase(const char* string_a, const char* suffix_string);

    inline bool is_space(char inchar)
    {
        return inchar == ' ' || inchar == '\n' || inchar == '\t' || inchar == '\r';
    }

    // erase white spaces on left and right of the input string
    void trim_left(std::string& input_string);
    void trim_right(std::string& input_string);
    void trim(std::string& input_string);

    // convert target string to lower case
    // @param string: Target string
    inline std::string lower_string(std::string string)
    {
        ::std::transform(string.begin(), string.end(), string.begin(), ::tolower);
        return string;
    }

    // convert target string to upper case
    // @param string: Target string
    inline std::string upper_string(std::string string)
    {
        ::std::transform(string.begin(), string.end(), string.begin(), ::toupper);
        return string;
    }

} // namespace cxx