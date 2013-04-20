#ifndef STANDARD_DATA_HPP
#define STANDARD_DATA_HPP

#include <vector>
#include <unordered_set>
#include <utility>
#include <string>

// EndOfFile: synthetic "character" to represent the end of source file
constexpr int EndOfFile = -1;

// See C++ standard 2.11 Identifiers and Appendix/Annex E.1
extern const std::vector<std::pair<int, int>> AnnexE1_Allowed_RangesSorted;

// See C++ standard 2.11 Identifiers and Appendix/Annex E.2
extern const std::vector<std::pair<int, int>> AnnexE2_DisallowedInitially_RangesSorted;

// See C++ standard 2.13 Operators and punctuators
extern const std::unordered_set<std::string> Digraph_IdentifierLike_Operators;

// See `simple-escape-sequence` grammar
extern const std::unordered_set<int> SimpleEscapeSequence_CodePoints;

#endif //STANDARD_DATA_HPP

