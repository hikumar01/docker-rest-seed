#pragma once

#include "compare/Diff.h"

class LongestCommonSubsequence {
    std::vector<std::string> splitWords(const std::string& str);
    std::vector<Diff> stringDiffutil(const std::vector<std::string>& words1, const std::vector<std::string>& words2);
public:
    std::vector<Diff> stringDiff(const std::string& str1, const std::string& str2);
};
