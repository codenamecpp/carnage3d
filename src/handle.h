#pragma once

namespace cxx
{
    // implements weak-pointer pattern via intrusive linked list

    template<typename TClass>
    class handle;
    class handle_base;

    // base class of all handled object
    class handled_object
    {
        friend class handle_base;

    public:
        handled_object() = default;
        // @param rside
        handled_object(const handled_object& rhs) 
            : mHandlesListHead() // do not share handles to this on copy
        {
        }
        ~handled_object()
        {
            reset_handle_references();
        }
        // do not share handles to this on copy
        inline void operator = (const handled_object& rhs)
        {
            // do not share handles to this on copy
        }
    protected:
        void reset_handle_references();

    private:
        handle_base* mHandlesListHead = nullptr;
    };

    //////////////////////////////////////////////////////////////////////////

    // base handle class
    class handle_base
    {
        friend class handled_object;
    public:
        // reset target
        inline void reset()
        {
            _detach_reference(false);
        }

        // whether handle is not pointing at object
        inline bool is_null() const { return mPointer == nullptr; }

        // check if pointing object was destroyed

        // determine whether handle was pointing at some object
        inline bool is_expired() const { return mHandleExpired; }

    protected: 
        // @param targetObject: Pointer to target object
        handle_base() = default;
        handle_base(const handle_base& otherHandle) = delete;
        ~handle_base()
        {
            _detach_reference(false);
        }

        // assign target
        handle_base& operator = (const handle_base& otherHandle) = delete;

    protected:
        // attach reference to target
        // @param targetObject: Pointer to object
        inline void _attach_reference(handled_object* targetObject)
        {
            if (targetObject == mPointer)
                return;

            _detach_reference(false);
            if (targetObject == nullptr)
                return;

            mPointer = targetObject;

            handled_object* basePointer = targetObject;
            mNext = basePointer->mHandlesListHead;
            if (basePointer->mHandlesListHead)
            {
                basePointer->mHandlesListHead->mPrev = this;
            }
            basePointer->mHandlesListHead = this;
        }

        // detach reference from target
        inline void _detach_reference(bool isExpired)
        {
            if (mPointer == nullptr)
                return;

            handled_object* basePointer = mPointer;
            if (mPrev) mPrev->mNext = mNext;
            if (mNext) mNext->mPrev = mPrev;
            if (basePointer->mHandlesListHead == this)
            {
                basePointer->mHandlesListHead = mNext;
            }
            _set_null();

            if (isExpired)
            {
                mHandleExpired = isExpired;
            }
        }

        // clean internal pointers
        inline void _set_null()
        {
            mNext = nullptr;
            mPrev = nullptr;
            mPointer = nullptr;
            mHandleExpired = false;
        }

    protected:
        handled_object* mPointer = nullptr;
        handle_base* mNext = nullptr;
        handle_base* mPrev = nullptr;

        bool mHandleExpired = false;
    };

    //////////////////////////////////////////////////////////////////////////

    template<typename TObject>
    class handle final: public handle_base
    {
    public:
        // ctor
        // @param targetObject: Pointer to target object
        handle() = default;
        handle(TObject* targetObject)
        {
            _attach_reference(targetObject);
        }
        // @param targetObject: Handle
        handle(const handle<TObject>& targetObject)
        {
            _attach_reference(targetObject.mPointer);
        }
        // assign target
        inline handle& operator = (TObject* targetObject)
        {
            _attach_reference(targetObject);
            return *this;
        }
        // assign target
        inline handle& operator = (const handle<TObject>& targetObject)
        {
            _attach_reference(targetObject.mPointer);
            return *this;
        }
        // access to target instance
        // assume that pointer class is compatible
        inline TObject* operator -> () const 
        {
            debug_assert(mPointer);
            return static_cast<TObject*>(mPointer);
        }
        // implicit convert to class pointer
        // assume that pointer class is compatible
        inline operator TObject* () const 
        { 
            return static_cast<TObject*>(mPointer);
        }
    };

    //////////////////////////////////////////////////////////////////////////
    // impl
    //////////////////////////////////////////////////////////////////////////

    inline void handled_object::reset_handle_references()
    {
        while (mHandlesListHead)
        {
            mHandlesListHead->_detach_reference(true);
        }
    }

} // namespace cxx