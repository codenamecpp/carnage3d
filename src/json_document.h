#pragma once

// internals
struct cJSON;

namespace cxx
{
    // element of configuration document
    class json_document_node
    {
        friend class json_document;

    public:
        json_document_node();
        json_document_node(const json_document_node& other);
        json_document_node& operator = (const json_document_node& other);
        ~json_document_node();

        // get next / previous sibling element
        json_document_node next_sibling() const;
        json_document_node prev_sibling() const;

        // get first child element of object or array element
        json_document_node first_child() const;

        // add child object node
        // @param parent: Parent node, must be non null, must be object type or array type
        // @param nodeName: New node name
        // @returns null if node with same name already exists
        json_document_node create_object_node(const std::string& nodeName);

        // add child string node
        // @param parent: Parent node, must be non null, must be object type or array type
        // @param nodeName: New node name
        // @param value: Initial value
        // @returns null if node with same name already exists
        json_document_node create_string_node(const std::string& nodeName, const std::string& value);

        // add child boolean node
        // @param parent: Parent node, must be non null, must be object type or array type
        // @param nodeName: New node name
        // @param value: Initial value
        // @returns null if node with same name already exists
        json_document_node create_boolean_node(const std::string& nodeName, bool value);

        // add child numeric node
        // @param parent: Parent node, must be non null, must be object type or array type
        // @param nodeName: New node name
        // @param value: Initial value
        // @returns null if node with same name already exists
        json_document_node create_numeric_node(const std::string& nodeName, int value);

        // add child numeric node
        // @param parent: Parent node, must be non null, must be object type or array type
        // @param nodeName: New node name
        // @param value: Initial value
        // @returns null if node with same name already exists
        json_document_node create_numeric_node(const std::string& nodeName, float value);

        // add child array node
        // @param parent: Parent node, must be non null, must be object type or array type
        // @param nodeName: New node name
        // @returns null if node with same name already exists
        json_document_node create_array_node(const std::string& nodeName);

        // get child node element by name
        // @param name: Node name
        json_document_node operator [] (const std::string& name) const;
        json_document_node operator [] (const char* name) const;

        // get array element by index if array
        // get child element by index if object
        // @param elementIndex: Array element index
        json_document_node operator [] (int elementIndex) const;

        // get number of elements in array
        // get number of child elements if object
        int get_elements_count() const;

        // test whether child node element is exists
        // @param name: Name
        bool is_child_exists(const std::string& name) const;

        // get name of element
        std::string get_element_name() const;

        // set node null
        void set_null();

        // operators
        inline bool operator == (const json_document_node& rhs) const { return mJsonElement && rhs.mJsonElement == mJsonElement; }
        inline bool operator != (const json_document_node& rhs) const { return !mJsonElement || rhs.mJsonElement != mJsonElement; }
        inline operator bool () const { return mJsonElement != nullptr; }
        
    protected:
        json_document_node(cJSON* jsonElementImplementation);

    protected:
        cJSON* mJsonElement = nullptr;
    };

    //////////////////////////////////////////////////////////////////////////

    // to set new node value just remove that node and create new of same type

    class json_node_boolean: public json_document_node
    {
    public:
        // cast generic node to boolean
        // @param genericNode: Generic json node
        json_node_boolean(const json_document_node& genericNode);
        json_node_boolean& operator = (const json_document_node& genericNode);

        // get/set boolean value
        bool get_value() const;
    
    private:
        void validate();
    };

    //////////////////////////////////////////////////////////////////////////

    class json_node_string: public json_document_node
    {
    public:
        // cast generic node to string
        // @param genericNode: Generic json node
        json_node_string(const json_document_node& genericNode);
        json_node_string& operator = (const json_node_string& genericNode);

        // get/set string value
        std::string get_value() const;

    private:
        void validate();
    };

    //////////////////////////////////////////////////////////////////////////

    class json_node_numeric: public json_document_node
    {
    public:
        // cast generic node to numeric
        // @param genericNode: Generic json node
        json_node_numeric(const json_document_node& genericNode);
        json_node_numeric& operator = (const json_node_numeric& genericNode);

        // get numeric value
        float get_value_float() const;
        int get_value_integer() const;

    private:
        void validate();
    };

    //////////////////////////////////////////////////////////////////////////

    class json_node_object: public json_document_node
    {
    public:
        // cast generic node to object
        // @param genericNode: Generic json node
        json_node_object(const json_document_node& genericNode);
        json_node_object& operator = (const json_node_object& genericNode);

    private:
        void validate();
    };

    //////////////////////////////////////////////////////////////////////////

    class json_node_array: public json_document_node
    {
    public:
        // cast generic node to array
        // @param genericNode: Generic json node
        json_node_array(const json_document_node& genericNode);
        json_node_array& operator = (const json_node_array& genericNode);

    private:
        void validate();
    };

    //////////////////////////////////////////////////////////////////////////

    template<typename TEnumClass>
    class json_node_enum: public json_document_node
    {
    public:
        // cast generic node to enum
        // @param genericNode: Generic json node
        json_node_enum(const json_document_node& genericNode): json_document_node(genericNode)
        {
            validate();
        }

        json_node_enum& operator = (const json_node_array& genericNode)
        {
            json_document_node::operator = (genericNode);
            validate();
            return *this;
        }

        TEnumClass get_value() const
        {
            debug_assert(this->operator bool());
            return mEnumValue;
        }

    private:
        void validate()
        {
            if (json_node_string stringNode = *this)
            {
                std::string enumValueString = stringNode.get_value();
                if (parse_enum(enumValueString.c_str(), mEnumValue))
                {
                    return; // success
                }
                // fail
                debug_assert(false);
            }
            set_null();            
        }

    private:
        TEnumClass mEnumValue = TEnumClass();
    };

    //////////////////////////////////////////////////////////////////////////

    // helpers

    // get attribute by name

    bool json_get_attribute(json_node_object json_node, const std::string& attribute_name, bool& output);
    bool json_get_attribute(json_node_object json_node, const std::string& attribute_name, std::string& output);
    bool json_get_attribute(json_node_object json_node, const std::string& attribute_name, int& output);
    bool json_get_attribute(json_node_object json_node, const std::string& attribute_name, float& output);
        
    template<typename TEnumClass>
    inline bool json_get_attribute(json_node_object json_node, const std::string& attribute_name, TEnumClass& output)
    {
        static_assert(std::is_enum<TEnumClass>::value, "Enum expected");
        if (json_node_enum<TEnumClass> enumNode = json_node[attribute_name])
        {
            output = enumNode.get_value();
            return true;
        }
        return false;
    }

    // get attribute by index

    bool json_get_attribute(json_node_array json_node, int item_index, bool& output);
    bool json_get_attribute(json_node_array json_node, int item_index, std::string& output);
    bool json_get_attribute(json_node_array json_node, int item_index, int& output);
    bool json_get_attribute(json_node_array json_node, int item_index, unsigned char& output);
    bool json_get_attribute(json_node_array json_node, int item_index, char& output);
    bool json_get_attribute(json_node_array json_node, int item_index, float& output);

    template<typename TEnumClass>
    inline bool json_get_attribute(json_node_array json_node, int item_index, TEnumClass& output)
    {
        static_assert(std::is_enum<TEnumClass>::value, "Enum expected");
        if (json_node_enum<TEnumClass> enumNode = json_node[item_index])
        {
            output = enumNode.get_value();
            return true;
        }
        return false;
    }

    //////////////////////////////////////////////////////////////////////////

    // json document
    class json_document: public noncopyable
    {
    public:
        // @param content: Content string
        json_document(const std::string& content);
        json_document();
        ~json_document();

        // parse json document from string
        // @param content: Content string
        bool parse_document(const std::string& content);
        void close_document();

        // create empty document with initial root node
        void create_document();

        // save document content to string
        // @param outputContent: Document data
        void dump_document(std::string& outputContent) const;

        // get root json object of document
        json_document_node get_root_node() const;

        // operators
        inline bool operator == (const json_document& rhs) const { return mJsonElement && rhs.mJsonElement == mJsonElement; }
        inline bool operator != (const json_document& rhs) const { return !mJsonElement || rhs.mJsonElement != mJsonElement; }
        inline operator bool () const { return mJsonElement != nullptr; }

    private:
        cJSON* mJsonElement;
    };

} // namespace cxx