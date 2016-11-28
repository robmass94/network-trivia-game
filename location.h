#ifndef _LOCATION_H_
#define _LOCATION_H_

#include <string>

struct location {
	location() {}
	location(const std::string& adr, const int& prt) {
		addr = adr;
		port = prt;
	}

	std::string addr;
	int         port;
};

#endif
