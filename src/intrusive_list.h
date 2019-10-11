#pragma once

namespace cxx
{    
    // implements simple intrusive list, it works with no memory allocations
    // but note that it does not owns element nodes

    template<typename TClass> class intrusive_node;
    template<typename TClass> class intrusive_iterator;
    template<typename TClass>
    class intrusive_list final
    {
    public:
        // declare iterators
	    using iterator = intrusive_iterator<TClass>;

        intrusive_list()
            : mHead()
            , mTail()
            , mElementCount()
        {
        }
        // disable copy ctor
        intrusive_list(const intrusive_list&) = delete;
        ~intrusive_list()
        {
            clear();
        }

        // disable copy
        intrusive_list& operator = (const intrusive_list&) = delete;

        // insert element node to list
        // @param node: Element node
        inline void insert(intrusive_node<TClass>* node)
        {
            debug_assert(node && !node->mList);
            if (!node || node->mList)
                return;

            node->mNext = nullptr;
            node->mList = this;
            node->mPrev = mTail;
            if (mTail)
            {
                mTail->mNext = node;
            }
            mTail = node;
            if (!mHead)
            {
                mHead = mTail;
            }
            ++mElementCount;
        }

        // remove element node from list
        // @param node: Element node
        inline void remove(intrusive_node<TClass>* node)
        {
            debug_assert(node && node->mList == this);
            if (!node || node->mList != this)
                return;

            if (node == mHead) mHead = node->mNext;
            if (node == mTail) mTail = node->mPrev;
            if (node->mNext)
            {
                node->mNext->mPrev = node->mPrev;
            }
            if (node->mPrev)
            {
                node->mPrev->mNext = node->mNext;
            }
            --mElementCount;
            debug_assert(mElementCount > -1);

            node->set_unlinked();
        }

        // unlink all nodes from list
        inline void clear()
        {
            while (mHead)
            {
                remove(mHead);
            }
        }

        // test whether list has elements
        inline bool has_elements() const { return mHead != nullptr; }

        // test whether list has specified node
        // @param node: Element node
        inline bool contains(intrusive_node<TClass>* node) const
        {
            return node && node->mList == this;
        }

        // get head and tail nodes
        inline intrusive_node<TClass>* get_head_node() const { return mHead; }
        inline intrusive_node<TClass>* get_tail_node() const { return mTail; }

        // get iterator starting node
	    inline iterator begin() const { return iterator(mHead); }
	    inline iterator cbegin() const { return iterator(mHead); }

        // get iterator ending node
	    inline iterator end() const { return iterator(); }
	    inline iterator cend() const { return iterator(); }

        // get number of elements in list
        inline int size() const { return mElementCount; }

    private:
        intrusive_node<TClass>* mHead;
        intrusive_node<TClass>* mTail;
        int mElementCount;
    };

    // defines intrusive list node
    template<typename TClass>
    class intrusive_node final
    {
        friend class intrusive_list<TClass>;
        friend class intrusive_list<const TClass>;
	    friend class intrusive_iterator<TClass>;
	    friend class intrusive_iterator<const TClass>;

    public:
        intrusive_node(TClass* element): mElement(element), mNext(), mPrev(), mList()
        {
            debug_assert(element);
        }
        // disable copy ctor
        intrusive_node(const intrusive_node&) = delete;
        ~intrusive_node()
        {
            if (mList)
            {
                mList->remove(this);
            }
        }
        // disable copy
        intrusive_node& operator = (const intrusive_node&) = delete;

        // get sibling nodes
        inline intrusive_node* get_next_node() const { return mNext; }
        inline intrusive_node* get_prev_node() const { return mPrev; }

        // get element pointer
        inline TClass* get_element() const { return mElement; }

        // remove this node from list
        void unlink()
        {
            debug_assert(mList);
            if (mList)
            {
                mList->remove(this);
            }
        }

        // test whether this node is in list
        bool is_linked() const { return mList != nullptr; }

    private:
        void set_unlinked()
        {
            mNext = nullptr;
            mPrev = nullptr;
            mList = nullptr;
        }

    private:
        TClass* mElement;
        intrusive_node* mNext;
        intrusive_node* mPrev;
        intrusive_list<TClass>* mList;
    };

    // defines intrusive list bidirectional iterator
    template<typename TClass>
    class intrusive_iterator: 
        public std::iterator<std::bidirectional_iterator_tag, TClass*> 
    {
    public:
	    friend class intrusive_list<typename std::remove_const<TClass>::type>;

	    intrusive_iterator() = default;
	    explicit intrusive_iterator(intrusive_node<TClass>* node)
            : mNode(node)
        {
        }
        // move to next node in list, prefix semantics
	    inline intrusive_iterator& operator ++ ()
        {
            debug_assert(mNode);
            mNode = mNode->get_next_node();
            return *this;
        }

        // move to previous node in list, prefix semantics
	    inline intrusive_iterator& operator -- ()
        {
            debug_assert(mNode);
            mNode = mNode->get_prev_node();
            return *this;
        }

        // move to next node in list, postfix semantics
	    inline intrusive_iterator operator ++ (int)
        {
            debug_assert(mNode);

            intrusive_iterator it(mNode->get_next_node());
            return it;
        }

        // move to previous node in list, postfix semantics
	    inline intrusive_iterator operator -- (int)
        {
            debug_assert(mNode);

            intrusive_iterator it(mNode->get_prev_node());
            return it;
        }

        // test whether two iterators pointing same node or both is nullptr
	    bool operator == (const intrusive_iterator& other) const
        {
            return mNode == other.mNode;
        }

        // test whether two iterators pointing different nodes
	    bool operator != (const intrusive_iterator& other) const
        {
            return mNode != other.mNode;
        }

        // access to node element pointer
	    inline intrusive_node<TClass>* operator -> () const
        {
            debug_assert(mNode);
            return mNode;
        }

        // access to node element reference
	    inline TClass* operator * () const
        {
            debug_assert(mNode);
            return mNode->get_element();
        }

    private:
	    intrusive_node<TClass>* mNode = nullptr;
    };

} // namespace cxx