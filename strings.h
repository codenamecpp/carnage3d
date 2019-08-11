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

            const int iresult = ::_stricmp(astring.c_str(), bstring.c_str());
            return iresult == 0;
        }
    };

    struct icase_less
    {
        inline bool operator () (const std::string& astring, const std::string& bstring) const 
        {
            const int iresult = ::_stricmp(astring.c_str(), bstring.c_str());
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

    // fast print formatted string into destination buffer
    // @param buffer_string: Destination buffer
    // @param buffer_size: Destination buffer size
    // @param format_string: String format
    int f_snprintf(char* buffer_string, int buffer_size, const char* format_string, ...);
    int f_vsnprintf(char* buffer_string, int buffer_size, const char* format_string, va_list args_list);

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
        inline void clear()
        {
            debug_assert(mBufferPtr);
            if (mBufferCapacity > 0)
            {
                mBufferEnd = mBufferPtr;
                *mBufferPtr = 0;
            }
        }
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
        inline void append_char(char c)
        {
            int currLength = get_length();
            if (currLength < mBufferCapacity)
            {
                mBufferEnd[0] = c;
                mBufferEnd[1] = 0;
                ++mBufferEnd;
            }
        }
        // discard last character in buffer
        inline void pop_back()
        {
            if (mBufferPtr == mBufferEnd)
            {
                debug_assert(false);
                return;
            }
            --mBufferEnd;
            mBufferEnd[0] = 0;
        }
        // append string to buffer, will discard all data that does not fit in buffer
        inline void append_string(const char* sourceString)
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
        // set string buffer content
        // @param sourceString: Source string
        // @param length: Source string length, optional
        inline void set_content(const char* sourceString, int length = 0)
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
        //print formatted
        // @param szFormatString: Format string
        inline int printf(const char* szFormatString, ...)
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
        // test whether content is empty
        inline bool empty() const { return mBufferPtr == mBufferEnd; }

    protected:
        string_buffer(char* buffer_begin, char* buffer_end, int buffer_capacity)
            : mBufferPtr(buffer_begin)
            , mBufferEnd(buffer_end)
            , mBufferCapacity(buffer_capacity)
        {
            debug_assert(mBufferPtr);
            clear();
        }
    private:
        char* mBufferPtr;
        char* mBufferEnd;
        int mBufferCapacity;
    };

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

    // get length of common prefix in two strings
    int count_common_prefix_length(const char* string_a, const char* string_b);

} // namespace cxx