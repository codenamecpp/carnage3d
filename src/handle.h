#pragma once

namespace cxx
{
    // implements weak-pointer pattern via intrusive linked list

    template<typename TClass>
    class handle;

    // base class of handled object
    template<typename TSelfClass>
    class handled_object
    {   
        friend class handle<TSelfClass>;

    public:
        handled_object() = default;
        // @param rside
        handled_object(const handled_object& rside) 
            : mHandlesListHead() // do not share handles to this on copy
        {
        }
        ~handled_object()
        {
            reset_handle_references();
        }
        // do not share handles to this on copy
        inline void operator = (const handled_object& rside)
        {
        }
    protected:
        void reset_handle_references()
        {
            while (mHandlesListHead)
            {
                mHandlesListHead->_detach_reference(true);
            }
        }
    private:
        handle<TSelfClass>* mHandlesListHead = nullptr;
    };

    // handle class
    template<typename TClass>
    class handle final
    {
        friend class handled_object<TClass>;

    public: 
        // ctor
        // @param targetObject: Pointer to target object
        handle() = default;
        handle(TClass* targetObject)
        {
            _attach_reference(targetObject);
        }
        // @param targetObject: Handle
        handle(const handle<TClass>& targetObject)
        {
            _attach_reference(targetObject.mPointer);
        }
        // dtor
        ~handle()
        {
            _detach_reference(false);
        }

        // assign target
        inline handle<TClass>& operator = (TClass* targetObject)
        {
            _attach_reference(targetObject);
            return *this;
        }

        // assign target
        inline handle<TClass>& operator = (const handle<TClass>& targetObject)
        {
            _attach_reference(targetObject.mPointer);
            return *this;
        }

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

        // access to target instance
        inline TClass* operator -> () const 
        {
            debug_assert(mPointer);
            return mPointer;
        }

        // implicit convert to class pointer
        inline operator TClass* () const { return mPointer; }

    private:
        // attach reference to target
        // @param targetObject: Pointer to object
        inline void _attach_reference(TClass* targetObject)
        {
            if (targetObject == mPointer)
                return;

            _detach_reference(false);
            if (!targetObject)
                return;

            mPointer = targetObject;

            handled_object<TClass>* basePointer = targetObject;
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
            if (!mPointer)
                return;

            handled_object<TClass>* basePointer = mPointer;
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

    private:
        TClass* mPointer = nullptr;
        handle* mNext = nullptr;
        handle* mPrev = nullptr;

        bool mHandleExpired = false;
    };

} // namespace cxx