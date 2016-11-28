#ifndef _USER_H_
#define _USER_H_

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

struct user {
	user() { }
	user(const std::string& username) : usn(username) { }

	friend std::istream& operator>> (std::istream& is, user& usr) {
		std::string encoded_user, token;

		// set username
		std::getline(is, token, '|');
		usr.usn = token;

		// set password
		std::getline(is, token, '|');
		usr.pwd = token;

		return is;
	}

	friend std::ostream& operator<< (std::ostream& os, const user& usr) {
		return os << usr.usn << "(" << usr.pwd << ")";
	}

	friend bool operator< (const user& left, const user& right) {
		return left.usn < right.usn;
	}

	std::string              usn, pwd;
};

#endif
