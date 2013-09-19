#ifndef OPTIONAL_H
#define OPTIONAL_H

#include <cassert>
#include <type_traits>
#include <memory>
#include <QString>

struct nil_t {
    nil_t() {}
};

static const nil_t nil;

template<typename T, bool Fundamental = std::is_fundamental<T>::value >
class optional {
};

template<typename T>
class optional <T,true> {
public:
    typedef optional<T, true> type;

    optional(T value) : m_initialized(true), m_value(value) {}
    optional(T value, bool available) : m_initialized(available), m_value(available ? new T(value) : nullptr) {}
    optional() : m_initialized(false) {}
    optional(const nil_t x) : m_initialized(false) { (void)x /*silence unused warning*/; }
    T& operator*() { assert(m_initialized); return m_value; }
    const T& operator*() const { assert(m_initialized); return m_value; }
    bool available() const { return m_initialized; }
    operator bool() const { return available(); }
    bool operator==(const T& other) const { if(!available()) return false; else return m_value == other; }
    bool operator==(const type& other) const { if(available() && other.available()) return m_value == *other; else return available() == other.available(); }
private:
    bool m_initialized;
    T m_value;
};

template<typename T>
class optional <T, false> {
public:
    typedef optional<T, false> type;

    optional(T value) : m_initialized(true), m_value(new T(value)) {}
    optional(T value, bool available) : m_initialized(available), m_value(available ? new T(value) : nullptr) {}
    optional() : m_initialized(false), m_value(nullptr) {}
    optional(const nil_t x) : m_initialized(false) { (void)x /*silence unused warning*/; }
    optional(const optional<T>& source) : m_initialized(source.available()), m_value(source.available() ? new T(*source.m_value) : nullptr) {}
    T& operator*() { assert(m_initialized); return *m_value; }
    const T& operator*() const { assert(m_initialized); return *m_value; }
    bool available() const { return m_initialized; }
    operator bool() const { return available(); }
    bool operator==(const T& other) const { if(!available()) return false; else return *m_value == other; }
    bool operator==(const type& other) const { if(available() && other.available()) return *m_value == *other; else return available() == other.available(); }
private:
    bool m_initialized;
    std::unique_ptr<T> m_value;
};

template<>
class optional <QString, false> {
public:
    typedef optional<QString, true> type;

    optional(QString value) : m_initialized(true), m_value(value) {}
    optional(QString value, bool available) : m_initialized(available), m_value(available ? value : "") {}
    optional() : m_initialized(false) {}
    optional(const nil_t x) : m_initialized(false) { (void)x /*silence unused warning*/; }
    QString& operator*() { assert(m_initialized); return m_value; }
    const QString& operator*() const { assert(m_initialized); return m_value; }
    bool available() const { return m_initialized; }
    operator bool() const { return available(); }
    bool operator==(const QString& other) const { if(!available()) return false; else return m_value == other; }
    bool operator==(const type& other) const { if(available() && other.available()) return m_value == *other; else return available() == other.available(); }
private:
    bool m_initialized;
    QString m_value;
};

#endif // OPTIONAL_H
