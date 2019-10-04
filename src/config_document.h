#pragma once

// internals
struct cJSON;

namespace cxx
{
    // element of configuration document
    class config_node
    {
    public:
        config_node(cJSON* jsonElementImplementation);
        ~config_node();
        // find child node element by name
        // @param name: Name
        config_node get_child(const char* name) const;
        // get next / previous sibling element
        config_node next_sibling() const;
        config_node prev_sibling() const;
        // get first child element of object or array element
        config_node first_child() const;
        // get array element by index
        // @param elementIndex: Array element index
        config_node get_array_element(int elementIndex) const;
        // get number of elements in array
        int get_array_elements_count() const;
        // test whether child node element is exists
        // @param name: Name
        bool is_child_exists(const char* name) const;
        // determine type of element value
        bool is_string_element() const;
        bool is_number_element() const;
        bool is_boolean_element() const;
        bool is_array_element() const;
        bool is_object_element() const;
        // get element value
        const char* get_value_string() const;
        bool get_value_boolean() const;
        int get_value_integer() const;
        float get_value_float() const;
        // get name of element
        const char* get_element_name() const;
        // operators
        inline bool operator == (const config_node& rhs) const { return mJsonElement && rhs.mJsonElement == mJsonElement; }
        inline bool operator != (const config_node& rhs) const { return !mJsonElement || rhs.mJsonElement != mJsonElement; }
        inline operator bool () const { return mJsonElement != nullptr; }
    private:
        cJSON* mJsonElement;
    };

    // configuration document
    class config_document final: public noncopyable
    {
    public:
        // @param content: Content string
        config_document(const char* content);
        config_document();
        ~config_document();
        // Parse configuration document from string
        // @param content: Content string
        bool parse_document(const char* content);
        void close_document();
        // Get root json object of document
        config_node get_root_node() const;
        // test whether document is loaded
        bool is_loaded() const;
    private:
        cJSON* mJsonElement;
    };

} // namespace cxx