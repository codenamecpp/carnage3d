#include "stdafx.h"
#include "PixelsArray.h"
#include "MemoryManager.h"

//////////////////////////////////////////////////////////////////////////

#define STBI_ASSERT(s) debug_assert(s)
#define STBI_NO_JPEG
#define STBI_NO_PSD
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM

static cxx::memory_allocator* gPixelsArrayAllocator = nullptr;

inline void* stbi_malloc_proxy(size_t dataLength)
{
    debug_assert(gPixelsArrayAllocator);
    return gPixelsArrayAllocator->allocate(dataLength);
}

inline void* stbi_mrealloc_proxy(void* dataPointer, size_t dataLength)
{
    debug_assert(gPixelsArrayAllocator);
    return gPixelsArrayAllocator->reallocate(dataPointer, dataLength);
}

inline void stbi_mfree_proxy(void *dataPointer)
{
    debug_assert(gPixelsArrayAllocator);
    gPixelsArrayAllocator->deallocate(dataPointer);
}

#define STBI_MALLOC(s) stbi_malloc_proxy(s)
#define STBI_REALLOC(p, s) stbi_mrealloc_proxy(p, s)
#define STBI_FREE(p) stbi_mfree_proxy(p)

#define STB_IMAGE_IMPLEMENTATION
#include "std_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

struct PixelsAllocatorScope
{
public:
    PixelsAllocatorScope(cxx::memory_allocator* memAllocator)
    {
        debug_assert(gPixelsArrayAllocator == nullptr);
        debug_assert(memAllocator);
        gPixelsArrayAllocator = memAllocator;
    }
    ~PixelsAllocatorScope()
    {
        gPixelsArrayAllocator = nullptr;
    }
};

//////////////////////////////////////////////////////////////////////////

PixelsArray::~PixelsArray()
{
    Cleanup();
}

bool PixelsArray::Create(eTextureFormat format, int sizex, int sizey, cxx::memory_allocator* allocator)
{
    if (format == eTextureFormat_Null || sizex < 1 || sizey < 1)
    {
        debug_assert(false);
        return false;
    }

    // try to reuse allocated memory
    if (mFormat == format && mSizex == sizex && mSizey == sizey)
        return true;

    int bytesPerPixel = NumBytesPerPixel(format);
    if (bytesPerPixel == 0)
    {
        assert(bytesPerPixel > 0);
        return false;
    }

    Cleanup();
    SetPixelsAllocator(allocator);

    PixelsAllocatorScope setupAllocator {mPixelsAllocator};

    mData = static_cast<unsigned char*>(stbi__malloc(sizex * sizey * bytesPerPixel));
    debug_assert(mData);

    if (mData == nullptr)
        return false;

    mSizex = sizex;
    mSizey = sizey;
    mFormat = format;

    return true;
}

bool PixelsArray::LoadFromFile(const std::string& fileName, eTextureFormat forceFormat, cxx::memory_allocator* allocator)
{
    Cleanup();

    int imagecomponents;
    int forcecomponents = 0;
    if (forceFormat != eTextureFormat_Null)
    {
        switch (forceFormat)
        {
            case eTextureFormat_RGBA8: forcecomponents = 4; break;
            case eTextureFormat_RGB8: forcecomponents = 3; break;
            case eTextureFormat_R8_G8: forcecomponents = 2; break;
            case eTextureFormat_R8: forcecomponents = 1; break;
            default:
            {
                debug_assert(forceFormat == eTextureFormat_Null);
                return false;
            }
        }
    }

    std::ifstream fileStream;
    if (!gFiles.OpenBinaryFile(fileName, fileStream))
        return false;

    stbi_io_callbacks stbi_cb =
    {       
        // read, fill 'data' with 'size' bytes.  return number of bytes actually read
        [](void *user, char *data, int size) -> int
        {
            std::ifstream& fs = *static_cast<std::ifstream*>(user);
            fs.read(data, size);
            std::streamsize bytes = fs.gcount();
            return static_cast<int>(bytes);
        }
        ,
        // skip, skip the next 'n' bytes, or 'unget' the last -n bytes if negative
        [](void *user, int n) -> void
        {
            std::ifstream& fs = *static_cast<std::ifstream*>(user);
            fs.seekg(n, std::ios::cur);
        }
        ,
        // eof, returns nonzero if we are at end of file/data
        [](void *user) -> int
        {
            std::ifstream& fs = *static_cast<std::ifstream*>(user);
            return fs.eof() ? 1 : 0;
        }
    };

    SetPixelsAllocator(allocator);

    PixelsAllocatorScope setupAllocator {mPixelsAllocator};
    stbi_uc* pImageContent = stbi_load_from_callbacks(&stbi_cb, &fileStream, &mSizex, &mSizey, &imagecomponents, forcecomponents);
    if (pImageContent == nullptr)
        return false;

    if (forcecomponents)
    {
        imagecomponents = forcecomponents;
    }

    debug_assert(imagecomponents >= 1 && imagecomponents <= 4); // not sure if it could happen
    if (imagecomponents < 1 || imagecomponents > 4)
    {
        stbi_image_free(pImageContent);
        return false;
    }

    eTextureFormat formats[] =
    {
        eTextureFormat_Null, eTextureFormat_R8, eTextureFormat_R8_G8, eTextureFormat_RGB8, eTextureFormat_RGBA8
    };
    mFormat = formats[imagecomponents];
    mData = pImageContent;
    return true;
}

bool PixelsArray::SaveToFile(const std::string& fileName)
{
    return SaveToFile(fileName, mFormat, mSizex, mSizey, mData);
}

bool PixelsArray::SaveToFile(const std::string& fileName, eTextureFormat format, int sizex, int sizey, unsigned char* pixels)
{
    if (format == eTextureFormat_Null)
        return false;

    int comp = NumBytesPerPixel(format);
    return stbi_write_png(fileName.c_str(), sizex, sizey, comp, pixels, sizex * comp) > 0;
}

bool PixelsArray::FillWithCheckerBoard()
{
    if (mFormat == eTextureFormat_Null)
        return false;

    if (mFormat == eTextureFormat_RGB8)
    {
        for (int iy = 0; iy < mSizey; ++iy)
        for (int ix = 0; ix < mSizex; ++ix)
        {
            unsigned char color = 0xFF;
            if ((iy / 8) % 2 == (ix / 8) % 2)
            {
                color = 0x00;
            }
            int offset = (iy * mSizex * 3) + (ix * 3);
            mData[offset + 0] = color;
            mData[offset + 1] = 0x00;
            mData[offset + 2] = color;
        }
    }
    if (mFormat == eTextureFormat_RGBA8)
    {
        for (int iy = 0; iy < mSizey; ++iy)
        for (int ix = 0; ix < mSizex; ++ix)
        {
            unsigned char color = 0xFF;
            if ((iy / 8) % 2 == (ix / 8) % 2)
            {
                color = 0x00;
            }
            int offset = (iy * mSizex * 4) + (ix * 4);
            mData[offset + 0] = color;
            mData[offset + 1] = 0x00;
            mData[offset + 2] = color;
            mData[offset + 3] = 0xFF;
        }
    }
    return true;
}

bool PixelsArray::FillWithColor(Color32 color)
{
    if (mFormat == eTextureFormat_Null)
        return false;

    int bpp = NumBytesPerPixel(mFormat);
    debug_assert(bpp == 3 || bpp == 4 || bpp == 1);

    for (int iy = 0; iy < mSizey; ++iy)
    for (int ix = 0; ix < mSizex; ++ix)
    {
        int offset = (iy * mSizex * bpp) + (ix * bpp);
        if (bpp == 1)
        {
            mData[0] = color.mR;
        }
        else
        {
            mData[offset + 0] = color.mR;
            mData[offset + 1] = color.mG;
            mData[offset + 2] = color.mB;
            if (bpp == 4)
            {
                mData[offset + 3] = color.mA;
            }
        }
    }
    return true;
}

void PixelsArray::Cleanup()
{
    if (mFormat == eTextureFormat_Null)
        return;

    debug_assert(mData);
    if (mData)
    {
        PixelsAllocatorScope setupAllocator {mPixelsAllocator};

        stbi_image_free(mData);
        mData = nullptr;
    }

    mFormat = eTextureFormat_Null;
    mSizex = 0;
    mSizey = 0;
    mPixelsAllocator = nullptr;
}

bool PixelsArray::HasContent() const
{
    return mFormat != eTextureFormat_Null;
}

void PixelsArray::SetPixelsAllocator(cxx::memory_allocator* allocator)
{
    debug_assert(mPixelsAllocator == nullptr);
    if (allocator == nullptr)
    {
        allocator = gMemoryManager.mHeapAllocator;
    }
    mPixelsAllocator = allocator;
}
