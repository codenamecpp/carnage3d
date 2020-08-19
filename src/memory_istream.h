#pragma once

#include <iosfwd>
#include <istream>

namespace cxx
{
    class memory_istream: public std::streambuf
    {
    public:
        memory_istream(char* memory_begin, char* memory_end) 
            : mBegin(memory_begin)
            , mEnd(memory_end)
        {
            this->setg(memory_begin, memory_begin, memory_end);
        }
    
    private:
        // override streambuf
        pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) override
        {
            switch (dir)
            {
                case std::ios_base::cur:
                        this->gbump(static_cast<int>(off));
                    break;
                case std::ios_base::end:
                        this->setg(mBegin, mEnd + off, mEnd);
                    break;
                case std::ios_base::beg:
                        this->setg(mBegin, mBegin + off, mEnd);
                    break;
            }

            return this->gptr() - this->eback();
        }
 
        pos_type seekpos(std::streampos pos, std::ios_base::openmode mode) override
        {
            return this->seekoff(pos - pos_type(off_type(0)), std::ios_base::beg, mode);
        }

    private:
        char* mBegin; 
        char* mEnd;
    };

    // stream helpers
    template<typename TElement>
    inline bool read_elements(std::istream& instream, TElement* elements, int elements_count)
    {
        if (!instream.read((char*) elements, elements_count * sizeof(TElement)))
            return false;

        return true;
    }

    template<typename TElement>
    inline int read_some_elements(std::istream& instream, TElement* elements, int elements_count)
    {
        int numElements = 0;
        while (instream.good())
        {
            if (numElements == elements_count)
                break;

            if (!instream.read((char*)elements, sizeof(TElement)))
                break;

            ++elements;
            ++numElements;
        }
        
        return numElements;
    }

    inline unsigned char read_int8(std::istream& instream)
    {
        unsigned char resultData = 0;
        bool isSuccess = read_elements(instream, &resultData, 1);
        debug_assert(isSuccess);
        return resultData;
    }

    inline unsigned short read_int16(std::istream& instream)
    {
        unsigned short resultData = 0;
        bool isSuccess = read_elements(instream, &resultData, 1);
        debug_assert(isSuccess);
        return resultData;
    }

    inline unsigned int read_int32(std::istream& instream)
    {
        unsigned int resultData = 0;
        bool isSuccess = read_elements(instream, &resultData, 1);
        debug_assert(isSuccess);
        return resultData;
    }

    inline bool read_cstring(std::istream& instream, char* output_buffer, int output_buffer_length)
    {
        if (!instream.getline(output_buffer, output_buffer_length, 0))
            return false;

        return true;
    }

} // namespace cxx