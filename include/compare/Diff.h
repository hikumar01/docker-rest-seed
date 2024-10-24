#pragma once

#include <string>
#include <vector>

enum class Operation {
    DELETE, INSERT, EQUAL
};

class Diff {
    Operation operation;
    std::string text;

public:
    Diff(Operation op, const std::string& t) : operation(op), text(t) {}

    std::string get_operation_string() const;

    std::string get_text() const;

    friend std::ostream& operator<<(std::ostream& os, const Diff& diff);
};

std::ostream& operator<<(std::ostream& os, const Diff& diff);
