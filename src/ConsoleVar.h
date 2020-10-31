#pragma once

enum CvarFlags: unsigned long
{
    CvarFlags_None               = 0,
    CvarFlags_Archive            = (1 << 1), // will save and load from json
    CvarFlags_Readonly           = (1 << 2), // cannot change either via console nor command line
    CvarFlags_Init               = (1 << 3), // don't allow change from console at all, but can be set from the command line
    CvarFlags_Modified           = (1 << 4), // was modified
    CvarFlags_Cheat              = (1 << 5), // treat as cheat cvar
    CvarFlags_RequiresMapRestart = (1 << 6), // requires current level restart to apply
    CvarFlags_RequiresAppRestart = (1 << 7), // requires application restart to apply
    CvarFlags_Hidden             = (1 << 8), // invisible to console, but can be set from the command line
    // cvar value type flags
    CvarFlags_CvarString         = (1 << 9),
    CvarFlags_CvarInt            = (1 << 10),
    CvarFlags_CvarFloat          = (1 << 11),
    CvarFlags_CvarBool           = (1 << 12), 
    CvarFlags_CvarColor          = (1 << 13), // rgba, 4 unsigned bytes
    CvarFlags_CvarPoint          = (1 << 14), // 2 ints
    CvarFlags_CvarVec3           = (1 << 15), // 3 floats
    CvarFlags_CVarEnum           = (1 << 16), // int
};
decl_enum_as_flags(CvarFlags)

enum eCvarSetMethod
{
    eCvarSetMethod_Config,
    eCvarSetMethod_CommandLine,
    eCvarSetMethod_Console,
};

// Base console variable class
class Cvar: public cxx::noncopyable
{
    friend class Console;

public:
    // readonly
    std::string mName;
    std::string mDescription;
    CvarFlags mCvarFlags = CvarFlags_None;

public:
    inline void SetModified()
    {
        mCvarFlags = (mCvarFlags | CvarFlags_Modified);
    }
    inline void ClearModified()
    {
        mCvarFlags = (mCvarFlags & ~CvarFlags_Modified);
    }

    // Try set new value from string
    bool SetFromString(const std::string& input, eCvarSetMethod setMethod);

    // Save or restore cvar from jscon document node
    void SaveCvar(cxx::json_document_node rootNode) const;
    bool LoadCvar(cxx::json_document_node rootNode);

    // Get current value string representation
    virtual void GetPrintableValue(std::string& output) const = 0;
    virtual void GetPrintableDefaultValue(std::string& output) const = 0;

    // cvar flags shortcuts
    bool IsArchive()    const { return (mCvarFlags & CvarFlags_Archive)  > 0; }
    bool IsReadonly()   const { return (mCvarFlags & CvarFlags_Readonly) > 0; }
    bool IsModified()   const { return (mCvarFlags & CvarFlags_Modified) > 0; }
    bool IsCheat()      const { return (mCvarFlags & CvarFlags_Cheat)    > 0; }
    bool IsInit()       const { return (mCvarFlags & CvarFlags_Init)     > 0; }
    bool IsHidden()     const { return (mCvarFlags & CvarFlags_Hidden)   > 0; }
    bool IsRequiresMapRestart() const { return (mCvarFlags & CvarFlags_RequiresMapRestart) > 0; }
    bool IsRequiresAppRestart() const { return (mCvarFlags & CvarFlags_RequiresAppRestart) > 0; }
    bool IsString()     const { return (mCvarFlags & CvarFlags_CvarString) > 0; }
    bool IsBool()       const { return (mCvarFlags & CvarFlags_CvarBool)   > 0; }
    bool IsEnum()       const { return (mCvarFlags & CvarFlags_CVarEnum)   > 0; }
    bool IsInt()        const { return (mCvarFlags & CvarFlags_CvarInt)    > 0; }
    bool IsFloat()      const { return (mCvarFlags & CvarFlags_CvarFloat)  > 0; }

protected:
    Cvar(const std::string& cvarName, const std::string& description, CvarFlags cvarFlags);
    virtual ~Cvar();

    // Load new value from input string
    virtual bool DeserializeValue(const std::string& input, bool& valueChanged) = 0;
};

//////////////////////////////////////////////////////////////////////////

class CvarBoolean: public Cvar
{
public:
    bool mValue = false;
    bool mDefaultValue = false;

public:
    CvarBoolean(const std::string& cvarName, bool cvarValue, const std::string& description, CvarFlags cvarFlags);

protected:
    // Get current value string representation
    void GetPrintableValue(std::string& output) const override;
    void GetPrintableDefaultValue(std::string& output) const override;

    // Load new value from input string
    bool DeserializeValue(const std::string& input, bool& valueChanged) override;
};

//////////////////////////////////////////////////////////////////////////

class CvarString: public Cvar
{
public:
    std::string mValue;
    std::string mDefaultValue;

public:
    CvarString(const std::string& cvarName, const std::string& cvarValue, const std::string& description, CvarFlags cvarFlags);

protected:
    // Get current value string representation
    void GetPrintableValue(std::string& output) const override;
    void GetPrintableDefaultValue(std::string& output) const override;

    // Load new value from input string
    bool DeserializeValue(const std::string& input, bool& valueChanged) override;
};

//////////////////////////////////////////////////////////////////////////

class CvarInt: public Cvar
{
public:
    int mValue = 0;
    int mDefaultValue = 0;
    int mMinValue = 0;
    int mMaxValue = 0;

public:
    CvarInt(const std::string& cvarName, int cvarValue, int cvarMin, int cvarMax, const std::string& description, CvarFlags cvarFlags);
    CvarInt(const std::string& cvarName, int cvarValue, const std::string& description, CvarFlags cvarFlags);

protected:
    // Get current value string representation
    void GetPrintableValue(std::string& output) const override;
    void GetPrintableDefaultValue(std::string& output) const override;

    // Load new value from input string
    bool DeserializeValue(const std::string& input, bool& valueChanged) override;
};

//////////////////////////////////////////////////////////////////////////

class CvarFloat: public Cvar
{
public:
    float mValue = 0.0f;
    float mDefaultValue = 0.0f;
    float mMinValue = 0.0f;
    float mMaxValue = 0.0f;

public:
    CvarFloat(const std::string& cvarName, float cvarValue, float cvarMin, float cvarMax, const std::string& description, CvarFlags cvarFlags);
    CvarFloat(const std::string& cvarName, float cvarValue, const std::string& description, CvarFlags cvarFlags);

protected:
    // Get current value string representation
    void GetPrintableValue(std::string& output) const override;
    void GetPrintableDefaultValue(std::string& output) const override;

    // Load new value from input string
    bool DeserializeValue(const std::string& input, bool& valueChanged) override;
};

//////////////////////////////////////////////////////////////////////////

class CvarColor: public Cvar
{
public:
    Color32 mValue;
    Color32 mDefaultValue;

public:
    CvarColor(const std::string& cvarName, Color32 cvarValue, const std::string& description, CvarFlags cvarFlags);

protected:
    // Get current value string representation
    void GetPrintableValue(std::string& output) const override;
    void GetPrintableDefaultValue(std::string& output) const override;

    // Load new value from input string
    bool DeserializeValue(const std::string& input, bool& valueChanged) override;
};

//////////////////////////////////////////////////////////////////////////

class CvarPoint: public Cvar
{
public:
    Point mValue;
    Point mDefaultValue;

public:
    CvarPoint(const std::string& cvarName, const Point& cvarValue, const std::string& description, CvarFlags cvarFlags);

protected:
    // Get current value string representation
    void GetPrintableValue(std::string& output) const override;
    void GetPrintableDefaultValue(std::string& output) const override;

    // Load new value from input string
    bool DeserializeValue(const std::string& input, bool& valueChanged) override;
};

//////////////////////////////////////////////////////////////////////////

class CvarVec3: public Cvar
{
public:
    glm::vec3 mValue;
    glm::vec3 mDefaultValue;

public:
    CvarVec3(const std::string& cvarName, const glm::vec3& cvarValue, const std::string& description, CvarFlags cvarFlags);

protected:
    // Get current value string representation
    void GetPrintableValue(std::string& output) const override;
    void GetPrintableDefaultValue(std::string& output) const override;

    // Load new value from input string
    bool DeserializeValue(const std::string& input, bool& valueChanged) override;
};

//////////////////////////////////////////////////////////////////////////

template<typename TEnum>
class CvarEnum: public Cvar
{
public:
    TEnum mValue;
    TEnum mDefaultValue;

public:
    CvarEnum(const std::string& cvarName, TEnum cvarValue, const std::string& description, CvarFlags cvarFlags)
        : Cvar(cvarName, description, cvarFlags | CvarFlags_CVarEnum)
        , mValue(cvarValue)
        , mDefaultValue(cvarValue)
    {
    }

protected:
    // Get current value string representation
    void GetPrintableValue(std::string& output) const override
    {
        output = cxx::enum_to_string(mValue);
    }
    void GetPrintableDefaultValue(std::string& output) const override
    {
        output = cxx::enum_to_string(mDefaultValue);
    }

    // Load new value from input string
    bool DeserializeValue(const std::string& input, bool& valueChanged) override
    {
        TEnum deserializeValue;
        if (!cxx::parse_enum(input.c_str(), deserializeValue))
        {
            gConsole.LogMessage(eLogMessage_Debug, "Cannot parse enum value '%s'", input.c_str());
            return false;
        }
        valueChanged = (deserializeValue != mValue);
        if (valueChanged)
        {
            mValue = deserializeValue;
        }
        return true;
    }
};