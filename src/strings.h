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

    //////////////////////////////////////////////////////////////////////////

    // fast print formatted string into destination buffer
    // @param buffer_string: Destination buffer
    // @param buffer_size: Destination buffer size
    // @param format_string: String format
    int f_snprintf(char* buffer_string, int buffer_size, const char* format_string, ...);
    int f_vsnprintf(char* buffer_string, int buffer_size, const char* format_string, va_list args_list);

    //////////////////////////////////////////////////////////////////////////

    // defines string buffer of fixed length
    class string_buffer
    {
    public:
        // get string buffer capacity and content length
        inline int get_capacity() const { return mBufferCapacity; }
        inline int get_length() const 
        { 
            return mBufferEnd - mBufferPtr; 
        }
        // clear string buffer content
        void clear();

        // get c-string pointer for read only access
        inline const char* c_str() const 
        { 
            debug_assert(mBufferPtr);
            return mBufferPtr; 
        }
        // get c-string pointer for both reading and write data
        inline char* c_str() 
        { 
            debug_assert(mBufferPtr);
            return mBufferPtr; 
        }
        // push back character, will be discarded if buffer is full
        void append_char(char c);

        // discard last character in buffer
        void pop_back();

        // append string to buffer, will discard all data that does not fit in buffer
        void append_string(const char* sourceString);

        // set string buffer content
        // @param sourceString: Source string
        // @param length: Source string length, optional
        void set_content(const char* sourceString, int length = 0);

        //print formatted
        // @param szFormatString: Format string
        int printf(const char* szFormatString, ...);

        // remove whitespace characters on front and back of the string buffer
        void trim_left();
        void trim_right();
        void trim();

        // get string buffer character by index
        inline char& operator [] (int ichar) 
        { 
            debug_assert(ichar > -1 && ichar < get_length());
            return mBufferPtr[ichar]; 
        }
        // get string buffer character by index
        inline char operator [] (int ichar) const
        {
            debug_assert(ichar > -1 && ichar < get_length());
            return mBufferPtr[ichar];
        }

        // test whether content is empty
        inline bool empty() const { return mBufferPtr == mBufferEnd; }

    protected:
        // base string buffer could be instantiated only by derived class
        string_buffer(char* buffer_begin, char* buffer_end, int buffer_capacity);

    private:
        char* mBufferPtr;
        char* mBufferEnd;
        int mBufferCapacity;
    };

    //////////////////////////////////////////////////////////////////////////

    // template string buffer with capacity specialization
    template<unsigned int BufferCapacity>
    class string_buffer_t: public string_buffer
    {
        template<unsigned int N> friend class string_buffer_t;

    public:
        string_buffer_t()
            : string_buffer(mStringBuffer, mStringBuffer, BufferCapacity)
        {}
        // @param szString: Initial content
        // @param stringLength: Initial content length, optional
        string_buffer_t(const char* szString, int stringLength = 0) 
            : string_buffer(mStringBuffer, mStringBuffer, BufferCapacity)
        {
            set_content(szString, stringLength);
        }
        // @param sourceBuffer: Source buffer with initial content
        template<unsigned int OtherStringBufferCapacity>
        string_buffer_t(const string_buffer_t<OtherStringBufferCapacity>& sourceBuffer)
            : string_buffer(mStringBuffer, mStringBuffer, BufferCapacity)
        {
            set_content(sourceBuffer.mBufferPtr, sourceBuffer.get_length());
        }
        // set content
        // @param sourceBuffer: Source buffer
        template<unsigned int OtherStringBufferCapacity>
        inline string_buffer_t& operator = (const string_buffer_t<OtherStringBufferCapacity>& sourceBuffer) 
        {
            set_content(sourceBuffer.mBufferPtr, sourceBuffer.get_length());
            return *this;
        }
        // set content
        // @param szString: Source string
        inline string_buffer_t& operator = (const char* szString) 
        {
            set_content(szString);
            return *this; 
        }
    private:
        char mStringBuffer[BufferCapacity + 1];
    };

    //////////////////////////////////////////////////////////////////////////

    // Predefined string buffer typedefs
    using string_buffer_16 = string_buffer_t<16>;
    using string_buffer_32 = string_buffer_t<32>;
    using string_buffer_64 = string_buffer_t<64>;
    using string_buffer_90 = string_buffer_t<90>;
    using string_buffer_128 = string_buffer_t<128>;
    using string_buffer_256 = string_buffer_t<256>;
    using string_buffer_512 = string_buffer_t<512>;
    using string_buffer_1024 = string_buffer_t<1024>;
    using string_buffer_2048 = string_buffer_t<2048>;
    using string_buffer_4096 = string_buffer_t<4096>;

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
        cxx::string_buffer_64 mContent;
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

} // namespace cxx