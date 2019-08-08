#pragma once

#include <type_traits>

namespace cxx
{
    // implements objects pool

    namespace details
    {
        // node contains object data along with additional info
        template<typename TPoolElement>
        class object_pool_node
        {
        private:
            using pool_node_t = object_pool_node<TPoolElement>;
            using data_storage_t = std::aligned_storage<sizeof(TPoolElement), alignof(TPoolElement)>;
            // raw data bytes
            using raw_data_t = typename data_storage_t::type;
            raw_data_t mData;

        public:
            // initialize element
            template<typename ... TArgs>
            inline TPoolElement* construct(TArgs&& ... args)
            {
                TPoolElement* element = reinterpret_cast<TPoolElement*>(&mData);
                // placement new
                new (element) TPoolElement(std::forward<TArgs>(args)...);
                return element;
            }
            // deinitialze element
            inline void destruct()
            {
                TPoolElement* element = reinterpret_cast<TPoolElement*>(&mData);
                element->~TPoolElement();
            }
        public:
            // chain pointers, both null if node is in use
            pool_node_t* mNextFreeNode;
            pool_node_t* mPrevFreeNode;
        };

        // chunk contains fixed number of nodes
        template<typename TPoolElement, int BlockSize>
        class object_pool_chunk
        {
            using pool_node_t = object_pool_node<TPoolElement>;
            using pool_chunk_t = object_pool_chunk<TPoolElement, BlockSize>;

        public:
            object_pool_chunk()
                : mFreeNodesHead()
                , mNextChunk()
            {
                // init free nodes chain
                for (int inode = 0; inode < BlockSize; ++inode)
                {
                    mNodes[inode].mNextFreeNode = (inode < BlockSize - 1) ? &mNodes[inode + 1] : nullptr;
                    mNodes[inode].mPrevFreeNode = (inode > 0) ? &mNodes[inode - 1] : nullptr;
                }
                mFreeNodesHead = mNodes;
            }
            ~object_pool_chunk()
            {
#ifdef _DEBUG
                int numFreeNodes = 0;
                for (pool_node_t* currentNode = mFreeNodesHead; currentNode; 
                    currentNode = currentNode->mNextFreeNode)
                {
                    ++numFreeNodes;
                }
                debug_assert(numFreeNodes == BlockSize);
#endif
                mFreeNodesHead = nullptr;
                if (mNextChunk)
                {
                    delete mNextChunk;
                }
            }
            // request free element from pool chunk
            template<typename ... TArgs>
            inline TPoolElement* allocate_object(TArgs&& ... args)
            {
                if (mFreeNodesHead == nullptr)
                {
                    // allocate new chunk
                    if (mNextChunk == nullptr)
                    {
                        mNextChunk = new pool_chunk_t();
                    }

                    TPoolElement* element = mNextChunk->allocate_object(std::forward<TArgs>(args)...);
                    return element;
                }

                pool_node_t* node = mFreeNodesHead;
                pop_from_free_nodes_list(node);

                // initialize object
                return node->construct(std::forward<TArgs>(args)...);
            }
            // return used element to pool chunk
            inline void deallocate_object(TPoolElement* element)
            {
                debug_assert(element);
                pool_node_t* node = reinterpret_cast<pool_node_t*>(element);
                if (node >= mNodes && node < mNodes + BlockSize)
                {
                    bool isUsedNode = is_used_node(node);

                    debug_assert(isUsedNode);
                    if (isUsedNode) // valid node
                    {
                        node->destruct();
                        put_to_free_nodes_list(node);
                    }
                    return;
                }
        
                if (mNextChunk)
                {
                    mNextChunk->deallocate_object(element);
                    return;
                }
                // invalid node
                debug_assert(false);
            }
        private:
            // remove node from free list
            inline void pop_from_free_nodes_list(pool_node_t* node)
            {
                debug_assert(is_free_node(node));

                if (node->mNextFreeNode)
                    node->mNextFreeNode->mPrevFreeNode = node->mPrevFreeNode;

                if (node->mPrevFreeNode)
                    node->mPrevFreeNode->mNextFreeNode = node->mNextFreeNode;

                if (node == mFreeNodesHead)
                    mFreeNodesHead = node->mNextFreeNode;

                node->mNextFreeNode = nullptr;
                node->mPrevFreeNode = nullptr;
            }
            // add node to free list
            inline void put_to_free_nodes_list(pool_node_t* node)
            {
                debug_assert(is_used_node(node));

                node->mNextFreeNode = mFreeNodesHead;
                if (mFreeNodesHead)
                {
                    mFreeNodesHead->mPrevFreeNode = node;
                }
                mFreeNodesHead = node;
            }
            // test whether node is free
            inline bool is_free_node(pool_node_t* node)
            {
                return node->mNextFreeNode || node->mPrevFreeNode || node == mFreeNodesHead;
            }
            // test whether node is used
            inline bool is_used_node(pool_node_t* node)
            {
                return !is_free_node(node);
            }
        private:
            pool_chunk_t* mNextChunk;
            pool_node_t* mFreeNodesHead;
            pool_node_t mNodes[BlockSize];
        };

    } // namespace details

    // template objects pool class
    template<typename TPoolElement, int BlockSize = 1024>
    class object_pool
    {
        using pool_chunk_t = details::object_pool_chunk<TPoolElement, BlockSize>;

    public:
        object_pool() = default;
        ~object_pool()
        {
            cleanup();
        }
        // request new object from pool
        template<typename ... TArgs>
        inline TPoolElement* create(TArgs&& ... args)
        {
            if (mFirstChunk == nullptr)
            {
                mFirstChunk = new pool_chunk_t;
            }

            TPoolElement* poolElement = mFirstChunk->allocate_object(std::forward<TArgs>(args)...);
            return poolElement;
        }
        // return object to pool
        inline void destroy(TPoolElement* element)
        {
            debug_assert(mFirstChunk);
            if (mFirstChunk)
            {
                mFirstChunk->deallocate_object(element);
            }
        }
        // frees allocated memory but does not destruct objects inside pool - user must do it manually
        inline void cleanup()
        {
            if (mFirstChunk)
            {
                delete mFirstChunk;
                mFirstChunk = nullptr;
            }
        }
    private:
        pool_chunk_t* mFirstChunk = nullptr;
    };

} // namespace cxx