#include "compare/LongestCommonSubsequence.h"

#include <algorithm>
#include <iostream>
#include <regex>

std::vector<std::string> LongestCommonSubsequence::splitWords(const std::string& str) {
    std::vector<std::string> result;
    // Regular expression to match words or delimiters (spaces, tabs, newlines)
    // std::regex wordDelimiterRegex(R"(( +|\t+|\S+))");
    std::regex wordDelimiterRegex(R"((\S+))");
    auto wordsBegin = std::sregex_iterator(str.begin(), str.end(), wordDelimiterRegex);
    auto wordsEnd = std::sregex_iterator();
    
    for (auto it = wordsBegin; it != wordsEnd; ++it) {
        result.push_back(it->str());
    }
    return result;
}


std::vector<Diff> LongestCommonSubsequence::stringDiff(const std::string& str1, const std::string& str2) {
    std::vector<std::string> words1 = splitWords(str1);
    std::vector<std::string> words2 = splitWords(str2);
    return stringDiffutil(words1, words2);
}

std::vector<Diff> LongestCommonSubsequence::stringDiffutil(const std::vector<std::string>& words1, const std::vector<std::string>& words2) {
    int m = words1.size();
    int n = words2.size();
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            if (words1[i - 1] == words2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }

    std::vector<Diff> diffs;
    int i = m, j = n;
    while (i > 0 && j > 0) {
        if (words1[i - 1] == words2[j - 1]) {
            diffs.emplace_back(Operation::EQUAL, words1[i - 1]);
            --i;
            --j;
        } else if (dp[i - 1][j] > dp[i][j - 1]) {
            diffs.emplace_back(Operation::DELETE, words1[i - 1]);
            --i;
        } else {
            diffs.emplace_back(Operation::INSERT, words2[j - 1]);
            --j;
        }
    }

    reverse(diffs.begin(), diffs.end());
    return diffs;
}
