#pragma once

enum CvarFlags: unsigned long
{
    CvarFlags_None               = 0,
    CvarFlags_Archive            = BIT(0), // will save and load from json
    CvarFlags_Readonly           = BIT(1), // cannot change either via console nor command line
    CvarFlags_Init               = BIT(2), // don't allow change from console at all, but can be set from the command line
    CvarFlags_Modified           = BIT(3), // was modified
    CvarFlags_Cheat              = BIT(4), // treat as cheat cvar
    CvarFlags_RequiresMapRestart = BIT(5), // requires current level restart to apply
    CvarFlags_RequiresAppRestart = BIT(6), // requires application restart to apply
    CvarFlags_Hidden             = BIT(7), // invisible to console, but can be set from the command line
    // cvar value type flags
    CvarFlags_CvarString         = BIT(8),
    CvarFlags_CvarInt            = BIT(9),
    CvarFlags_CvarFloat          = BIT(10),
    CvarFlags_CvarBool           = BIT(11), 
    CvarFlags_CvarColor          = BIT(12), // rgba, 4 unsigned bytes
    CvarFlags_CvarPoint          = BIT(13), // 2 ints
    CvarFlags_CvarVec3           = BIT(14), // 3 floats
    CvarFlags_CvarEnum           = BIT(15), // int
    CvarFlags_CvarVoid           = BIT(16), // cvar contains to data and acts like command
};
decl_enum_as_flags(CvarFlags);

enum eCvarSetMethod
{
    eCvarSetMethod_Config,
    eCvarSetMethod_CommandLine,
    eCvarSetMethod_Console,
};

class CvarCommand;
using CvarCommandProc = std::function<void(const char*)>;

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

    // Print brief info about variable to console
    void PrintInfo();

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
    bool IsEnum()       const { return (mCvarFlags & CvarFlags_CvarEnum)   > 0; }
    bool IsInt()        const { return (mCvarFlags & CvarFlags_CvarInt)    > 0; }
    bool IsFloat()      const { return (mCvarFlags & CvarFlags_CvarFloat)  > 0; }
    bool IsColor()      const { return (mCvarFlags & CvarFlags_CvarColor)  > 0; }
    bool IsPoint()      const { return (mCvarFlags & CvarFlags_CvarPoint)  > 0; }
    bool IsVec3()       const { return (mCvarFlags & CvarFlags_CvarVec3)   > 0; }
    bool IsVoid()       const { return (mCvarFlags & CvarFlags_CvarVoid)   > 0; }

protected:
    Cvar(const std::string& cvarName, const std::string& description, CvarFlags cvarFlags);
    virtual ~Cvar();

    // Load new value from input string
    virtual bool DeserializeValue(const std::string& input, bool& valueChanged) = 0;

    virtual void CallWithParams(const std::string& params);
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

class CvarVoid: public Cvar
{
public:
    std::string mCallingArgs;

public:
    CvarVoid(const std::string& cvarName, const std::string& description, CvarFlags cvarFlags);

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
        : Cvar(cvarName, description, cvarFlags | CvarFlags_CvarEnum)
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
            
            std::vector<const char*> enumStrings;
            cxx::get_enum_strings<TEnum>(enumStrings);

            if (!enumStrings.empty())
            {
                gConsole.LogMessage(eLogMessage_Debug, "Possible values:");
                for (const char* currString: enumStrings)
                {
                    gConsole.LogMessage(eLogMessage_Debug, " - %s", currString);
                }
            }
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