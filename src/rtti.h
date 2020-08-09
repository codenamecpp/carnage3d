#pragma once

#include "noncopyable.h"

namespace cxx
{
    // custom runtime type information support

    // type info holder

    class rtti_type : public noncopyable
    {
    public:
        // @param typeString: Type string identifier
        // @param parentType: optional base rtti
        rtti_type(const std::string& typeString, const rtti_type* parentType)

            : mParentType(parentType)
            , mTypeString(typeString)
        {
        }
    public:
        const std::string mTypeString;
        const rtti_type* mParentType;
    };

    template<typename TClass>
    inline const rtti_type* get_rtti_type()
    {
        return TClass::class_rtti_info::get_type();
    }

    // lightweight dynamic_cast alternative

    template<typename TDerivedClass, typename TBaseClass>
    inline TDerivedClass* rtti_cast(TBaseClass* baseClassPointer)
    {
        static_assert(std::is_base_of<TBaseClass, TDerivedClass>::value ||
            std::is_same<TBaseClass, TDerivedClass>::value, "Invalid type cast");

        if (baseClassPointer == nullptr)
            return nullptr;

        if (std::is_same<TBaseClass, TDerivedClass>::value)
        {
            debug_assert(dynamic_cast<TDerivedClass*>(baseClassPointer));
            return static_cast<TDerivedClass*>(baseClassPointer);
        }

        const rtti_type* actual_rtti_type = baseClassPointer->get_rtti_type();
        const rtti_type* target_rtti_type = get_rtti_type<TDerivedClass>();

        debug_assert(actual_rtti_type);
        debug_assert(target_rtti_type);

        for (; actual_rtti_type; actual_rtti_type = actual_rtti_type->mParentType)
        {
            if (actual_rtti_type == target_rtti_type)
            {
                debug_assert(dynamic_cast<TDerivedClass*>(baseClassPointer));
                return static_cast<TDerivedClass*>(baseClassPointer);
            }
        }
        return nullptr;
    }

} // namespace cxx

// put this macro into base class to enable custom rtti support

#define decl_rtti_base(class_name) \
\
public: \
    class class_rtti_info \
    { \
    public: \
        static const cxx::rtti_type* get_type() \
        { \
            static const cxx::rtti_type mRtti ( #class_name, nullptr ); \
            return &mRtti; \
        } \
        static const cxx::rtti_type* get_parent_type() \
        { \
            return nullptr; \
        } \
    }; \
    \
public:\
    virtual const cxx::rtti_type* get_rtti_type() const { return class_rtti_info::get_type(); }

// put this macro into derived class declaration

#define decl_rtti(class_name, base_class) \
\
public: \
    class class_rtti_info \
    { \
    public: \
        static const cxx::rtti_type* get_type() \
        { \
            static const cxx::rtti_type mRtti ( #class_name, base_class::class_rtti_info::get_type() ); \
            return &mRtti; \
        } \
        static const cxx::rtti_type* get_parent_type() \
        { \
            return get_type()->mParentType; \
        } \
    }; \
    \
public:\
    const cxx::rtti_type* get_rtti_type() const override { return class_rtti_info::get_type(); }