#include "stdafx.h"
#include "json_document.h"
#include "cJSON.h"

namespace cxx
{

json_document_node::json_document_node(cJSON* jsonElementImplementation)
    : mJsonElement(jsonElementImplementation)
{
}

json_document_node::json_document_node()
{
}

json_document_node::~json_document_node()
{
}

json_document_node::json_document_node(const json_document_node& other)
    : mJsonElement(other.mJsonElement)
{
}

json_document_node& json_document_node::operator = (const json_document_node& other)
{
    mJsonElement = other.mJsonElement;
    return *this;
}

json_document_node json_document_node::operator[](const std::string& name) const
{
    if (mJsonElement == nullptr)
        return json_document_node { nullptr };

    cJSON* element = cJSON_GetObjectItem(mJsonElement, name.c_str());
    return json_document_node { element };
}

json_document_node json_document_node::operator [] (const char* name) const
{
    if (mJsonElement == nullptr)
        return json_document_node { nullptr };

    cJSON* element = cJSON_GetObjectItem(mJsonElement, name);
    return json_document_node { element };
}

json_document_node json_document_node::next_sibling() const
{
    if (mJsonElement == nullptr)
        return json_document_node { nullptr };

    cJSON* element = mJsonElement->next;
    return json_document_node { element };
}

json_document_node json_document_node::prev_sibling() const
{
    if (mJsonElement == nullptr)
        return json_document_node { nullptr };

    cJSON* element = mJsonElement->prev;
    return json_document_node { element };
}

json_document_node json_document_node::first_child() const
{
    if (mJsonElement == nullptr)
        return json_document_node { nullptr };

    cJSON* element = mJsonElement->child;
    return json_document_node { element };
}

json_document_node json_document_node::operator[](int elementIndex) const
{
    if (mJsonElement == nullptr)
        return json_document_node { nullptr };

    if (mJsonElement->type == cJSON_Object)
    {
        cJSON* currentChild = mJsonElement->child;
        while (currentChild && elementIndex > 0) 
        {
            --elementIndex; 
            currentChild = currentChild->next; 
        }
        return json_document_node {currentChild};
    }

    if (mJsonElement->type == cJSON_Array)
    {
        cJSON* element = cJSON_GetArrayItem(mJsonElement, elementIndex);
        return json_document_node { element };
    }
    debug_assert(false);
    return json_document_node {};
}

int json_document_node::get_elements_count() const
{
    if (mJsonElement == nullptr)
        return 0;

    if (mJsonElement->type == cJSON_Object)
    {
        cJSON* currentChild = mJsonElement->child;
        int counter = 0;
        while(currentChild)
        {
            ++counter;
            currentChild = currentChild->next; 
        }
        return counter;
    }

    if (mJsonElement->type == cJSON_Array)
    {
        return cJSON_GetArraySize(mJsonElement);
    }
    return 0;
}

bool json_document_node::is_child_exists(const std::string& name) const
{
    if (mJsonElement == nullptr)
        return false;

    return cJSON_HasObjectItem(mJsonElement, name.c_str()) > 0;
}

std::string json_document_node::get_element_name() const
{
    if (mJsonElement == nullptr || mJsonElement->string == nullptr)
        return std::string();

    return mJsonElement->string;
}

void json_document_node::set_null()
{
    mJsonElement = nullptr;
}

//////////////////////////////////////////////////////////////////////////

json_document::json_document(const std::string& content)
    : mJsonElement()
{
    parse_document(content);
}

json_document::json_document()
    : mJsonElement()
{
}

json_document::~json_document()
{
    close_document();
}

bool json_document::parse_document(const std::string& content)
{
    close_document();

    mJsonElement = cJSON_Parse(content.c_str());
    return mJsonElement != nullptr;
}

void json_document::close_document()
{
    if (mJsonElement)
    {
        cJSON_Delete(mJsonElement);
        mJsonElement = nullptr;
    }
}

void json_document::create_document()
{
    close_document();

    const char* null_document_data = "{" "}";

    mJsonElement = cJSON_Parse(null_document_data);
    debug_assert(mJsonElement);
}

void json_document::dump_document(std::string& outputContent) const
{
    outputContent.clear();

    if (mJsonElement)
    {
        char* jsonData = cJSON_Print(mJsonElement);
        debug_assert(jsonData);
        outputContent.assign(jsonData);
        free(jsonData);
    }
}

json_document_node json_document::get_root_node() const
{
    return json_document_node { mJsonElement };
}

json_node_object json_document::create_object_node(const json_document_node& parent, const std::string& nodeName)
{
    if (!parent || parent.is_child_exists(nodeName))
    {
        debug_assert(false);
        return json_document_node { nullptr };
    }

    bool validType = (parent.mJsonElement->type == cJSON_Object) || (parent.mJsonElement->type == cJSON_Array);
    if (!validType)
    {
        debug_assert(false);
        return json_document_node { nullptr };
    }

    cJSON* jsonObject = cJSON_CreateObject();

    cJSON_AddItemToObject(parent.mJsonElement, nodeName.c_str(), jsonObject);
    return json_document_node { jsonObject };
}

json_node_string json_document::create_string_node(const json_document_node& parent, const std::string& nodeName, const std::string& value)
{
    if (!parent || parent.is_child_exists(nodeName))
    {
        debug_assert(false);
        return json_document_node { nullptr };
    }

    bool validType = (parent.mJsonElement->type == cJSON_Object) || (parent.mJsonElement->type == cJSON_Array);
    if (!validType)
    {
        debug_assert(false);
        return json_document_node { nullptr };
    }

    cJSON* jsonObject = cJSON_CreateString(value.c_str());

    cJSON_AddItemToObject(parent.mJsonElement, nodeName.c_str(), jsonObject);
    return json_document_node { jsonObject };
}

json_node_boolean json_document::create_boolean_node(const json_document_node& parent, const std::string& nodeName, bool value)
{
    if (!parent || parent.is_child_exists(nodeName))
    {
        debug_assert(false);
        return json_document_node { nullptr };
    }

    bool validType = (parent.mJsonElement->type == cJSON_Object) || (parent.mJsonElement->type == cJSON_Array);
    if (!validType)
    {
        debug_assert(false);
        return json_document_node { nullptr };
    }

    cJSON* jsonObject = cJSON_CreateBool(value);

    cJSON_AddItemToObject(parent.mJsonElement, nodeName.c_str(), jsonObject);
    return json_document_node { jsonObject };
}

json_node_numeric json_document::create_numeric_node(const json_document_node& parent, const std::string& nodeName, int value)
{
    if (!parent || parent.is_child_exists(nodeName))
    {
        debug_assert(false);
        return json_document_node { nullptr };
    }

    bool validType = (parent.mJsonElement->type == cJSON_Object) || (parent.mJsonElement->type == cJSON_Array);
    if (!validType)
    {
        debug_assert(false);
        return json_document_node { nullptr };
    }

    cJSON* jsonObject = cJSON_CreateNumber(value * 1.0);

    cJSON_AddItemToObject(parent.mJsonElement, nodeName.c_str(), jsonObject);
    return json_document_node { jsonObject };
}

cxx::json_node_numeric json_document::create_numeric_node(const json_document_node& parent, const std::string& nodeName, float value)
{
    if (!parent || parent.is_child_exists(nodeName))
    {
        debug_assert(false);
        return json_document_node { nullptr };
    }

    bool validType = (parent.mJsonElement->type == cJSON_Object) || (parent.mJsonElement->type == cJSON_Array);
    if (!validType)
    {
        debug_assert(false);
        return json_document_node { nullptr };
    }

    cJSON* jsonObject = cJSON_CreateNumber(value);

    cJSON_AddItemToObject(parent.mJsonElement, nodeName.c_str(), jsonObject);
    return json_document_node { jsonObject };
}

json_node_array json_document::create_array_node(const json_document_node& parent, const std::string& nodeName)
{
    if (!parent || parent.is_child_exists(nodeName))
    {
        debug_assert(false);
        return json_document_node { nullptr };
    }

    bool validType = (parent.mJsonElement->type == cJSON_Object) || (parent.mJsonElement->type == cJSON_Array);
    if (!validType)
    {
        debug_assert(false);
        return json_document_node { nullptr };
    }

    cJSON* jsonObject = cJSON_CreateArray();

    cJSON_AddItemToObject(parent.mJsonElement, nodeName.c_str(), jsonObject);
    return json_document_node { jsonObject };
}

//////////////////////////////////////////////////////////////////////////

json_node_boolean::json_node_boolean(const json_document_node& genericNode)
    : json_document_node(genericNode)
{
    validate();
}

json_node_boolean& json_node_boolean::operator = (const json_document_node& genericNode)
{
    json_document_node::operator = (genericNode);
    validate();
    return *this;
}

void json_node_boolean::validate()
{
    if (mJsonElement)
    {
        bool validType = mJsonElement->type == cJSON_True || mJsonElement->type == cJSON_False;
        if (!validType)
        {
            mJsonElement = nullptr; // invalid type
        }
    }
}

bool json_node_boolean::get_value() const
{
    debug_assert(mJsonElement);
    return mJsonElement && mJsonElement->valueint > 0;
}

//////////////////////////////////////////////////////////////////////////

json_node_string::json_node_string(const json_document_node& genericNode)
    : json_document_node(genericNode)
{
    validate();
}

json_node_string& json_node_string::operator = (const json_node_string& genericNode)
{
    json_document_node::operator = (genericNode);
    validate();
    return *this;
}

std::string json_node_string::get_value() const
{
    debug_assert(mJsonElement);

    if (mJsonElement && mJsonElement->valuestring)
        return mJsonElement->valuestring;

    return std::string();
}

void json_node_string::validate()
{
    if (mJsonElement)
    {
        bool validType = mJsonElement->type == cJSON_String;
        if (!validType)
        {
            mJsonElement = nullptr; // invalid type
        }
    }
}

//////////////////////////////////////////////////////////////////////////

json_node_numeric::json_node_numeric(const json_document_node& genericNode)
    : json_document_node(genericNode)
{
    validate();
}

json_node_numeric& json_node_numeric::operator = (const json_node_numeric& genericNode)
{
    json_document_node::operator = (genericNode);
    validate();
    return *this;
}

float json_node_numeric::get_value_float() const
{
    debug_assert(mJsonElement);
    return mJsonElement ? (float) mJsonElement->valuedouble : 0.0f;
}

int json_node_numeric::get_value_integer() const
{
    debug_assert(mJsonElement);
    return mJsonElement ? mJsonElement->valueint : 0;
}

void json_node_numeric::validate()
{
    if (mJsonElement)
    {
        bool validType = mJsonElement->type == cJSON_Number;
        if (!validType)
        {
            mJsonElement = nullptr; // invalid type
        }
    }
}

//////////////////////////////////////////////////////////////////////////

json_node_object::json_node_object(const json_document_node& genericNode)
    : json_document_node(genericNode)
{
    validate();
}

json_node_object& json_node_object::operator = (const json_node_object& genericNode)
{
    json_document_node::operator = (genericNode);
    validate();
    return *this;
}

void json_node_object::validate()
{
    if (mJsonElement)
    {
        bool validType = mJsonElement->type == cJSON_Object;
        if (!validType)
        {
            mJsonElement = nullptr; // invalid type
        }
    }
}

//////////////////////////////////////////////////////////////////////////

json_node_array::json_node_array(const json_document_node& genericNode)
    : json_document_node(genericNode)
{
    validate();
}

json_node_array& json_node_array::operator = (const json_node_array& genericNode)
{
    json_document_node::operator = (genericNode);
    validate();
    return *this;
}

void json_node_array::validate()
{
    if (mJsonElement)
    {
        bool validType = mJsonElement->type == cJSON_Array;
        if (!validType)
        {
            mJsonElement = nullptr; // invalid type
        }
    }
}

//////////////////////////////////////////////////////////////////////////

bool json_get_attribute(json_node_object json_node, const std::string& attribute_name, bool& output)
{
    if (json_node_boolean booleanNode = json_node[attribute_name])
    {
        output = booleanNode.get_value();
        return true;
    }
    return false;
}

bool json_get_attribute(json_node_object json_node, const std::string& attribute_name, std::string& output)
{
    if (json_node_string stringNode = json_node[attribute_name])
    {
        output = stringNode.get_value();
        return true;
    }
    return false;
}

bool json_get_attribute(json_node_object json_node, const std::string& attribute_name, int& output)
{
    if (json_node_numeric numericNode = json_node[attribute_name])
    {
        output = numericNode.get_value_integer();
        return true;
    }
    return false;
}

bool json_get_attribute(json_node_object json_node, const std::string& attribute_name, float& output)
{
    if (json_node_numeric numericNode = json_node[attribute_name])
    {
        output = numericNode.get_value_float();
        return true;
    }
    return false;
}

bool json_get_attribute(json_node_array json_node, int item_index, bool& output)
{
    if (json_node_boolean booleanNode = json_node[item_index])
    {
        output = booleanNode.get_value();
        return true;
    }
    return false;
}

bool json_get_attribute(json_node_array json_node, int item_index, std::string& output)
{
    if (json_node_string stringNode = json_node[item_index])
    {
        output = stringNode.get_value();
        return true;
    }
    return false;
}

bool json_get_attribute(json_node_array json_node, int item_index, int& output)
{
    if (json_node_numeric numericNode = json_node[item_index])
    {
        output = numericNode.get_value_integer();
        return true;
    }
    return false;
}

bool json_get_attribute(json_node_array json_node, int item_index, unsigned char& output)
{
    if (json_node_numeric numericNode = json_node[item_index])
    {
        int source_value_int = numericNode.get_value_integer();
        output = (unsigned char) source_value_int;
        debug_assert(output == source_value_int);
        return true;
    }
    return false;
}

bool json_get_attribute(json_node_array json_node, int item_index, char& output)
{
    if (json_node_numeric numericNode = json_node[item_index])
    {
        int source_value_int = numericNode.get_value_integer();
        output = (char) source_value_int;
        debug_assert(output == source_value_int);
        return true;
    }
    return false;
}

bool json_get_attribute(json_node_array json_node, int item_index, float& output)
{
    if (json_node_numeric numericNode = json_node[item_index])
    {
        output = numericNode.get_value_float();
        return true;
    }
    return false;
}

} // namespace cxx