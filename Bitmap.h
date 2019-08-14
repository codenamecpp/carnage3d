#pragma once

// defines simple pixels array in system memory
class Bitmap final: public cxx::noncopyable
{
public:
    eTextureFormat mFormat = eTextureFormat_Null;
    int mSizex = 0;
    int mSizey = 0;
    unsigned char* mData = nullptr;

public:
    Bitmap() = default;
    ~Bitmap();

    // Allocates array of pixels of specified format and dimensions
    // @param format: Format
    // @param sizex, sizey: Dimensions
    bool Create(eTextureFormat format, int sizex, int sizey);

    // Load bitmap from external file
    // Note that filesystem must be initialized otherwise error occurs
    // @param fileName: File name
    // @param forceFormat: Explicit format conversion, optional
    bool LoadFromFile(const char* fileName, eTextureFormat forceFormat = eTextureFormat_Null);

    // Save bitmap content to external file
    // Note that filesystem must be initialized otherwise error occurs
    // @param fileName: File name
    bool SaveToFile(const char* fileName);

    // Fill bitmap with checkerboard pattern, does not allocate memory
    // @returns false if bitmap null
    bool FillWithCheckerBoard();

    // Fill bitmap with solid color, does not allocate memory
    // @returns false if bitmap null
    bool FillWithColor(Color32 color);

    // Free allocated memory
    void Cleanup();

    // Test whether bitmap has pixels
    bool HasContent() const;
};