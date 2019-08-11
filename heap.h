#pragma once

namespace cxx
{
    // helper class fot heap sorting
    template<typename container, typename compare_type = std::less<element_type>>
    class heap_sorter
    {
    public:
        using element_type = typename container::value_type;
        static void push_element(container& containerref, const element_type& elementref)
        {
            containerref.emplace_back(elementref);
            std::push_heap(containerref.begin(), containerref.end(), get_cmp());
        }
        static void pop_element(container& containerref)
        {
            std::pop_heap(containerref.begin(), containerref.end(), get_cmp());
            containerref.pop_back();
        }
        static void fixup(container& containerref)
        {
            std::make_heap(containerref.begin(), containerref.end(), get_cmp());
        }
    private:
        static compare_type& get_cmp()
        {
            static compare_type static_cmp;
            return static_cmp;
        }
    };

    template<typename ContainerT, typename ElementT>
    inline void heap_push(ContainerT& theContainer, const ElementT& theElement)
    {
        heap_sorter::push_element(theContainer, theElement);
    }

    template<typename ContainerT>
    inline void heap_pop(ContainerT& theContainer)
    {
        heap_sorter::pop_element(theContainer);
    }

    template<typename ContainerT>
    inline void heap_fixup(ContainerT& theContainer)
    {
        heap_sorter::fixup(theContainer);
    }

} // namespace cxx