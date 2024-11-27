#include "debug_helper.hpp"
#include <iostream>

#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>

std::string demangle(const char* name) {

    int status = -4; // some arbitrary value to eliminate the compiler warning

    // enable c++11 by passing the flag -std=c++11 to g++
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };

    return (status==0) ? res.get() : name ;
}

#else

// does nothing if not g++
std::string demangle(const char* name) {
    return name;
}

#endif

ObjectHelper::ObjectHelper() : value_("default")
{
    std::cout << value_ <<": ObjectHelper()" << std::endl;
}

ObjectHelper::ObjectHelper(const std::string& value) : value_(value)
{
    std::cout << value_ <<": explicit ObjectHelper(const std::string& value)" << std::endl;
}

ObjectHelper::~ObjectHelper()
{
    std::cout << value_ <<": ~ObjectHelper()" << std::endl;
}

ObjectHelper::ObjectHelper(const ObjectHelper& other) : value_(other.value_)
{
    std::cout << value_ <<": ObjectHelper(const ObjectHelper& other)" << std::endl;
}

ObjectHelper::ObjectHelper(ObjectHelper&& other) : value_(std::move(other.value_))
{
    std::cout << value_ <<": ObjectHelper(ObjectHelper&& other)" << std::endl;
}

ObjectHelper& ObjectHelper::operator=(const ObjectHelper& other)
{
    value_ = other.value_;
    std::cout << value_ <<": ObjectHelper& operator=(const ObjectHelper& other)" << std::endl;
    return *this;
}

ObjectHelper& ObjectHelper::operator=(ObjectHelper&& other)
{
    value_ = std::move(other.value_);
    std::cout << value_ <<": ObjectHelper& operator=(ObjectHelper&& other)" << std::endl;
    return *this;
}

std::string ObjectHelper::get_value() const
{
    std::cout << value_ <<": std::string get_value() const" << std::endl;
    return value_;
}

void ObjectHelper::set_value(const std::string& value)
{
    std::cout << value_ <<": void set_value(const std::string& value)" << std::endl;
    value_ = value;
}

void ObjectHelper::print_value(void) const
{
    std::cout << value_ <<": void print_value(void) const" << std::endl;
}

void print_line(void)
{
    std::cout << "--------------------" << std::endl;
}
