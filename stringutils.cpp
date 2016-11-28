#include "stringutils.h"

namespace stringutils {

// ********************************************************************************************************************
// TokenizeString
//
// Parameters:
// 	str       - the string to tokenize
// 	separator - the character that separates tokens (defaults to ' ');
//
// Returns:
// 	a vector containaing all of the tokens in the passed in str
// ********************************************************************************************************************
std::vector<std::string> TokenizeString(const std::string& str, const char separator) {
	std::vector<std::string> tokens;
	std::stringstream        ss(str);
	std::string              token;
	
	while(std::getline(ss, token, separator)) {
		tokens.push_back(token);
	}

	return tokens;
}

// ********************************************************************************************************************
// Join
//
// Parameters:
// 	strs      - a vector of strings to join together
// 	start     - where to begin joining in the vector of strings (defaults to 0)
// 	separator - the character to separate strings with (defaults to ' ');
//
// Returns:
// 	the string created by combining all the strings from the passed in vector
// ********************************************************************************************************************
std::string Join(const std::vector<std::string>& strs, const int& start, const char separator) {
	std::string joined_str = strs[start];

	for(size_t i = start+1; i < strs.size(); ++i) {
		joined_str += separator + strs[i];
	}

	return joined_str;
}

}
