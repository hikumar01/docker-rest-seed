#include "compare/Diff.h"

#include <iostream>

std::string Diff::get_operation_string() const {
    switch (operation) {
        case Operation::DELETE: return "DELETE";
        case Operation::INSERT: return "INSERT";
        case Operation::EQUAL: return "EQUAL";
    }
    return "";
}

std::string Diff::get_text() const {
    return text;
}

std::ostream& operator<<(std::ostream& os, const Diff& diff) {
    os << "{" << diff.get_operation_string() << ", " << diff.get_text() << "}";
    return os;
}
