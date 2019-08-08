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
            : mHeadReferencesList() // do not share handles to this on copy
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
            while (mHeadReferencesList)
            {
                mHeadReferencesList->_detach_reference();
            }
        }
    private:
        handle<TSelfClass>* mHeadReferencesList = nullptr;
    };

    // handle class
    template<typename TClass>
    class handle final
    {
        friend class handled_object<TClass>;

        // in order to get things work, TClass should be derived from CHandledObject<>
        static_assert(std::is_base_of<handled_object<TClass>, TClass>::value, "Expect TClass is derived from handled_object<>");

    public: 
        // ctor
        // @param targetObject: Pointer to target object
        handle() = default;
        handle(TClass* targetObject): mPointer(), mNext(), mPrev()
        {
            _attach_reference(targetObject);
        }
        // @param targetObject: Handle
        handle(const handle<TClass>& targetObject): mPointer(), mNext(), mPrev()
        {
            _attach_reference(targetObject.mPointer);
        }
        // dtor
        ~handle()
        {
            _detach_reference();
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
            _detach_reference();
        }

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

            _detach_reference();
            if (!targetObject)
                return;

            mPointer = targetObject;

            handled_object<TClass>* basePointer = targetObject;
            mNext = basePointer->mHeadReferencesList;
            if (basePointer->mHeadReferencesList)
            {
                basePointer->mHeadReferencesList->mPrev = this;
            }
            basePointer->mHeadReferencesList = this;
        }

        // detach reference from target
        inline void _detach_reference()
        {
            if (!mPointer)
                return;

            handled_object<TClass>* basePointer = mPointer;
            if (mPrev) mPrev->mNext = mNext;
            if (mNext) mNext->mPrev = mPrev;
            if (basePointer->mHeadReferencesList == this)
            {
                basePointer->mHeadReferencesList = mNext;
            }
            _set_null();
        }

        // clean internal pointers
        inline void _set_null()
        {
            mNext = nullptr;
            mPrev = nullptr;
            mPointer = nullptr; 
        }

    private:
        TClass* mPointer = nullptr;
        handle* mNext = nullptr;
        handle* mPrev = nullptr;
    };

} // namespace cxx