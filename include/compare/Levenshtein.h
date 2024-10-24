#pragma once

#include "compare/Diff.h"

class Levenshtein {
public:
    std::vector<Diff> levenshtein_distance(const std::string& s1, const std::string& s2);
};
