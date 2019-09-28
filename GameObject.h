#pragma once

// defines base class of game objects
class GameObject: public cxx::noncopyable
{
public:
    virtual ~GameObject()
    {
    }
};