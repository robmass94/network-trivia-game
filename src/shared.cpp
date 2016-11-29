#include "shared.h"

// ********************************************************************************************************************
// StartServer
//
// Parameters:
// 	sd - the socket the server will be started on
//
// Returns:
// 	location - the addr and port other can reach the server by
// ********************************************************************************************************************
location StartServer(int& sd) {
	socklen_t          len;
	struct sockaddr_in ma;
	char               name[255];
	struct addrinfo    hints, *info;

	// create the socket, bind, and listen
	sd                 = socket(AF_INET, SOCK_STREAM, 0);
	len                = sizeof(ma);
	ma.sin_family      = AF_INET;
	ma.sin_port        = htons(0);
	ma.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(sd, (struct sockaddr *)&ma, len);
	listen(sd, 5);

	// get server details
	gethostname(name, sizeof(name));
	memset(&hints, 0, sizeof(hints));
	hints.ai_family   = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags    = AI_CANONNAME;
	getsockname(sd, (struct sockaddr *)&ma, &len);
	getaddrinfo(name, NULL, &hints, &info);

	// return the location of the newly created server
	location loc = location(info->ai_canonname, ntohs(ma.sin_port));
	freeaddrinfo(info);
	return loc;
}

// ********************************************************************************************************************
// ConnectToServer
//
// Parameters:
// 	sd     - the socket descriptor the server will be connected to 
// 	server - the addr of the server to connect to
// 	port   - the port of the server to connect to
//
// Returns:
// 	true  - if the connection succeeded
// 	false - if the connection could not be created
// ********************************************************************************************************************
bool ConnectToServer(const int& sd, const char* server, const int& port) {
	socklen_t          len;
	int                result;
	struct sockaddr_in sa;
	struct addrinfo    hints, *info;

	// resolve ip of server passed in
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if((result = getaddrinfo(server, NULL, &hints, &info)) != 0) {
		//printf("getaddrinfo wrong: %s\n", gai_strerror(result));
		return false;
	}

	// attempt to connect
	len = sizeof(sa);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = ((struct sockaddr_in *)(info->ai_addr))->sin_addr.s_addr;
	if((result = connect(sd, (struct sockaddr *)&sa, sizeof(sa))) != 0) {
		printf("connect: %s\n", strerror(errno));
		return false;
	}

	// success!
	freeaddrinfo(info);
	return true;
}

// ********************************************************************************************************************
// SendMessage
//
// Parameters:
// 	fd  - the socket descriptor to send the message over
// 	msg - the message to send
// ********************************************************************************************************************
void SendMessage(const int& fd, const std::string& msg) {
	int msg_length = msg.length();
	send(fd, &msg_length, sizeof(int), 0);
	send(fd, msg.c_str(), msg_length,  0);
}
