#pragma once

namespace cxx
{

    template<typename TValue>
    inline bool read_from_stream(std::istream& instream, TValue& outputValue)
    {
        if (!instream.read(reinterpret_cast<char*>(&outputValue), sizeof(outputValue)))
            return false;

        return true;
    }

} // namespace cxx

// helpers
#define READ_DATA(instream, destination, datatype) \
    { \
        datatype _$data; \
        if (!cxx::read_from_stream(instream, _$data)) \
            return false; \
        \
        destination = _$data; \
    }

#define READ_I8(instream, destination) READ_DATA(instream, destination, unsigned char)
#define READ_SI8(instream, destination) READ_DATA(instream, destination, char)
#define READ_I16(instream, destination) READ_DATA(instream, destination, unsigned short)
#define READ_SI16(instream, destination) READ_DATA(instream, destination, short)
#define READ_SI32(instream, destination) READ_DATA(instream, destination, int)
#define READ_BOOL(instream, destination) \
    { \
        unsigned char _$data; \
        if (!cxx::read_from_stream(instream, _$data)) \
            return false; \
        \
        destination = _$data > 0; \
    }

#define READ_FIXEDF32(instream, destination) \
    { \
        int _$data; \
        if (!cxx::read_from_stream(instream, _$data)) \
            return false; \
        \
        destination = _$data / 65536.0f; \
    }