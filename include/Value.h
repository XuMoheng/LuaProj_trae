#ifndef VALUE_H
#define VALUE_H

#include <variant>
#include <string>
#include <iostream>

// Simple Value representation
// Supports: nil, boolean, double, string
struct Nil {};

using Value = std::variant<Nil, bool, double, std::string>;

// Helper to print values
inline void printValue(const Value& value) {
    if (std::get_if<Nil>(&value) != nullptr) {
        std::cout << "nil";
    } else if (const bool* b = std::get_if<bool>(&value)) {
        std::cout << (*b ? "true" : "false");
    } else if (const double* d = std::get_if<double>(&value)) {
        std::cout << *d;
    } else if (const std::string* s = std::get_if<std::string>(&value)) {
        std::cout << *s;
    }
}

// Helper to check truthiness (Lua rules: only false and nil are false)
inline bool isFalsey(const Value& value) {
    if (std::get_if<Nil>(&value) != nullptr) return true;
    if (const bool* b = std::get_if<bool>(&value)) return !*b;
    return false;
}

#endif // VALUE_H
