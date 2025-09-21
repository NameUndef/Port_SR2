#ifndef DEBUG_HELPER_HPP_
#define DEBUG_HELPER_HPP_

#include <string>
#include <typeinfo>

std::string demangle(const char* name);

template <class T>
std::string get_demangled_type_name(const T& t) {

    return demangle(typeid(t).name());
}

template <class T>
std::string get_demangled_type_name() {

    return demangle(typeid(T).name());
}

class ObjectHelper {

    std::string value_;

public:
    ObjectHelper();
    explicit ObjectHelper(const std::string& value);
    ~ObjectHelper();
    ObjectHelper(const ObjectHelper& other);
    ObjectHelper(ObjectHelper&& other);
    ObjectHelper& operator=(const ObjectHelper& other);
    ObjectHelper& operator=(ObjectHelper&& other);

    std::string get_value() const;
    void set_value(const std::string& value);
    void print_value(void) const;
};

void print_line(void);

#endif  // DEBUG_HELPER_HPP_
