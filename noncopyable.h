#pragma once

namespace cxx
{
    // noncopyable
    struct noncopyable
    {
    public:
        noncopyable() = default;
        ~noncopyable() = default;

        noncopyable(const noncopyable&) = delete;
        noncopyable& operator = (const noncopyable&) = delete;
    };

} // namespace cxx