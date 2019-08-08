#pragma once

// boost lib
#include <boost/noncopyable.hpp>

namespace cxx
{
    // noncopyable alias
    using noncopyable = boost::noncopyable;

    // convert target string to lower case
    // @param string: Target string
    inline std::string lower_string(std::string string)
    {
        ::std::transform(string.begin(), string.end(), string.begin(), ::tolower);
        return string;
    }

    // convert target string to upper case
    // @param string: Target string
    inline std::string upper_string(std::string string)
    {
        ::std::transform(string.begin(), string.end(), string.begin(), ::toupper);
        return string;
    }

} // namespace cxx