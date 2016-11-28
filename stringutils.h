#ifndef _STRINGUTILS_H_
#define _STRINGUTILS_H_

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

namespace stringutils {

std::vector<std::string> TokenizeString(const std::string& str, const char separator = ' ');
std::string              Join(const std::vector<std::string>& v, const int& start = 0, const char separator = ' ');

}

#endif
