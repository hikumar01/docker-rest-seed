#include "compare/Levenshtein.h"
#include <algorithm>
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

std::vector<Diff> Levenshtein::levenshtein_distance(const std::string& s1, const std::string& s2) {
    size_t len1 = s1.size();
    size_t len2 = s2.size();

    if (len1 > len2) {
        return levenshtein_distance(s2, s1);
    }

    std::vector<size_t> prev(len2 + 1), curr(len2 + 1);

    // Initialize the first row (transformation from empty string to s2)
    for (size_t j = 0; j <= len2; ++j) prev[j] = j;

    // Fill the dp table using only two rows
    for (size_t i = 1; i <= len1; ++i) {
        curr[0] = i;  // Deleting all characters from s1 to empty string
        for (size_t j = 1; j <= len2; ++j) {
            if (s1[i - 1] == s2[j - 1]) {
                curr[j] = prev[j - 1];  // No change if characters match
            } else {
                curr[j] = std::min({prev[j] + 1,    // Deletion
                                    curr[j - 1] + 1, // Insertion
                                    prev[j - 1] + 1});// Substitution
            }
        }
        std::swap(prev, curr);  // Move current row to previous
    }

    // Backtrack to find operations
    std::vector<Diff> diffs;
    size_t i = len1, j = len2;

    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 && s1[i - 1] == s2[j - 1]) {
            diffs.emplace_back(Operation::EQUAL, std::string(1, s1[i - 1]));
            --i;
            --j;
        } else if (i > 0 && (j == 0 || prev[j] == prev[j - 1] + 1)) {
            diffs.emplace_back(Operation::DELETE, std::string(1, s1[i - 1]));
            --i;
        } else if (j > 0 && (i == 0 || prev[j] == prev[j - 1] + 1)) {
            diffs.emplace_back(Operation::INSERT, std::string(1, s2[j - 1]));
            --j;
        } else {
            diffs.emplace_back(Operation::DELETE, std::string(1, s1[i - 1]));
            diffs.emplace_back(Operation::INSERT, std::string(1, s2[j - 1]));
            --i;
            --j;
        }
    }

    std::reverse(diffs.begin(), diffs.end());
    return diffs;
}
