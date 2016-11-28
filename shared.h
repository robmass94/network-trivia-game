#ifndef _SHARED_H_
#define _SHARED_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include "location.h"

location StartServer     (int& sd);
bool     ConnectToServer (const int& sd, const char* server, const int& port);
void     SendMessage     (const int& fd, const std::string& msg);

#endif
