#ifndef MAYBE_H
#define MAYBE_H

#include <cassert>
#include <string>
#include "optional.h"

/*
This is the "Either String" monad, as a way to handle errors.
*/

//from http://zenol.fr/site/2013/08/27/an-alternative-error-handling-strategy-for-cpp/

template
<typename T>
class Maybe
{
private:
    //The value stored
    optional<T> m_value;
    //The error message stored
    std::string m_error;
    Maybe(void*, std::string error) : m_value(nil), m_error(error) {}
    Maybe(T value) : m_value(value), m_error("") {}
public:
    //Encapsulate the value
    static Maybe ret(T v)
    {
        return Maybe(v);
    }
    //Encapsulate an error
    static Maybe fail(std::string str)
    {
        return Maybe(nullptr, str);
    }
    //True if it's a valid value
    operator bool() const
    {
        return m_value.available();
    }
    //Deconstruct an Maybe to a value
    T to_value() const
    {
        assert(m_value.available());
        return *m_value;
    }
    //Deconstruct an Maybe to an error
    std::string to_error() const
    {
        assert(!m_value.available());
        return m_error;
    }

    std::string to_safe_error() const
    {
        if(m_value.available()) {
            return std::string("no error");
        }
        else {
            return m_error;
        }
    }

    template<typename F>
    inline
    auto bind(F f) -> decltype(f(m_value))
    {
        using type = decltype(f(m_value));
        if (*this)
            return f(m_value);
        else
            return type::fail(m_error);
    }
};

//special case for T=void
/*
    Special instance for void
*/

template<>
class Maybe<void>
{
private:
    std::string m_error;
    bool m_valid;

    Maybe()
    {}

public:
    //Encapsulate the value
    static Maybe ret()
    {
        Maybe box;
        box.m_valid = true;
        return box;
    }

    //Encapsulate an error
    static Maybe fail(std::string str)
    {
        Maybe box;
        box.m_error = str;
        box.m_valid = false;
        return box;
    }

    //True if it's a valid value
    operator bool() const
    {
        return m_valid;
    }

    //Déconstruct an Maybe to a value
    void to_value() const
    {
        assert(m_valid);
    }

    //Deconstruct an Maybe to an error
    std::string to_error() const
    {
        assert(!m_valid);
        return m_error;
    }

    std::string to_safe_error() const
    {
        if(m_valid) {
            return std::string("no error");
        }
        else {
            return m_error;
        }
    }

    template<typename F>
    inline
    auto bind(F f) -> decltype(f())
    {
        using type = decltype(f());
        if (*this)
            return f();
        else
        return type::fail(m_error);
    }
};

#endif // MAYBE_H
