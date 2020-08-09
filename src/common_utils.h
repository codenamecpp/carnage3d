#pragma once

// small c++ std templates library extensions

template <typename Array> struct ArrayType;
template <typename TElement, int NumElements> 
struct ArrayType<TElement[NumElements]>
{
    enum { Countof = NumElements };
};
template <typename TElement, int NumElements>
constexpr int CountOf(const TElement(&)[NumElements])
{
    return NumElements;
}
template <typename TElement>
inline void SafeDelete(TElement*& elementPointer)
{
    if (elementPointer)
    {
        delete elementPointer;
        elementPointer = nullptr;
    }
}
template<typename TElement>
inline void SafeDeleteArray(TElement*& elementPointer)
{
    if (elementPointer)
    {
        delete [] elementPointer;
        elementPointer = nullptr;
    }
}

namespace cxx
{
    // stl containers helpers

    template<typename TContainer, typename TElement>
    inline void erase_elements(TContainer& container, const TElement& element)
    {
        auto remove_iterator = std::remove(container.begin(), container.end(), element);
        if (remove_iterator != container.end())
        {
            container.erase(remove_iterator, container.end());
        }
    }

    template<typename TContainer>
    inline void erase_elements(TContainer& container, const TContainer& erase_container)
    {
        for (const auto& currElement: erase_container)
        {
            erase_elements(container, currElement);
        }
    }

    template<typename TContainer, typename TFuncPred>
    inline void erase_elements_if(TContainer& container, TFuncPred funcPred)
    {
        auto remove_iterator = std::remove_if(container.begin(), container.end(), funcPred);
        if (remove_iterator != container.end())
        {
            container.erase(remove_iterator, container.end());
        }
    }

    template<typename TContainer, typename TElement>
    inline bool contains(TContainer& container, const TElement& element)
    {
        auto find_iterator = std::find(container.begin(), container.end(), element);
        if (find_iterator == container.end())
            return false;

        return true;
    }

    template<typename TContainer, typename TFuncPred>
    inline bool contains_if(TContainer& container, TFuncPred funcPred)
    {
        auto find_iterator = std::find_if(container.begin(), container.end(), funcPred);
        if (find_iterator == container.end())
            return false;

        return true;
    }

    template<typename TContainer, typename TElement>
    inline void push_back_if_unique(TContainer& container, const TElement& element)
    {
        if (contains(container, element))
            return;

        container.push_back(element);
    }

    template<typename TContainer>
    inline bool collections_equals(const TContainer& lhs, const TContainer& rhs)
    {
        if (lhs.size() != rhs.size())
            return false;

        for (const auto& currElement: lhs)
        {
            if (!contains(rhs, currElement))
                return false;
        }
        return true;
    }

} // namespace cxx