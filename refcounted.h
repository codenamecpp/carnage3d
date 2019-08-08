#pragma once

// boost lib
#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace cxx
{
    // defines strong pointer to reference counted object
    template<typename TRefCounted>
    using reference = boost::intrusive_ptr<TRefCounted>;

    template<typename TRefCounted>
    using refcounted_delete_proc = void (*)(TRefCounted* object);

    // defines base class for reference counted objects
    template<typename TDerived>
    struct refcounted : public boost::noncopyable
    {
    public:
        refcounted() = default;
        refcounted(refcounted_delete_proc<TDerived> deleteProc)
            : mDeleteProc(deleteProc)
        {
        }

        // increment references counter
        inline void inc_reference_count() { ++mRefCounter; }

        // decrement references counter, will automatically delete object when it don't referenced anymore
        inline void dec_reference_count()
        {
            debug_assert(mRefCounter > 0);
            if (--mRefCounter == 0)
            {
                delete_this();
            }
        }

        // get references counter
        inline int get_reference_count() const { return mRefCounter; }

        // boost::intrusive_ptr stuff
        friend void intrusive_ptr_add_ref(refcounted<TDerived>* object)
        {
            object->inc_reference_count();
        }

        friend void intrusive_ptr_release(refcounted<TDerived>* object)
        {
            object->dec_reference_count();
        }

    protected:
        ~refcounted()
        {
            debug_assert(mRefCounter == 0);
        }

        // destroy object instance
        inline void delete_this()
        {
            if (mDeleteProc)
            {
                TDerived* thisInstance = static_cast<TDerived*>(this);
                mDeleteProc(thisInstance);
            }
        }

        static void default_deleter(TDerived* object)
        {
            delete object;
        }

    private:
        // custom deleter, will be invoked on refcunted object when it don't referenced anymore
        refcounted_delete_proc<TDerived> mDeleteProc = default_deleter;
        int mRefCounter = 0;
    };

} // namespace cxx