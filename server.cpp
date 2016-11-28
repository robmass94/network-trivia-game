#include <iostream>
#include <pthread.h>
#include <map>
#include <signal.h>
#include "shared.h"
#include "stringutils.h"
#include "user.h"
#include "location.h"

// globals
int      server_descriptor;
location self;
std::map<int, pthread_t> active_descriptors;

// server commands
void HandleExit ();

// server messages
void ReceiveMessage(const int& fd, const std::string& msg);

// server maintenance
void AddClient    (const int fd);
void RemoveClient (const int fd);

// thread functions
void* ProcessStdin       (void *);
void* ProcessConnections (void *);
void* ProcessMessages    (void *);
//void* ProcessGame        (void *); // <--- maybe here, might go into messages we'll see

// other functions
void BroadcastMessage(const int& fd, const std::string& msg);
void ServerInterruptHandler(int) {
	HandleExit();
	exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
	pthread_t stdin_thread, server_thread;
	struct sigaction sigIntHandler;

	// start the server
	self = StartServer(server_descriptor);
	printf("\033[0;31madmin: started server on '%s' at '%d'.\033[0m\n", self.addr.c_str(), self.port);

	// setup ctrl+c interceptor
	sigIntHandler.sa_handler = ServerInterruptHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	// ignore SIGPIPE
	signal(SIGPIPE, SIG_IGN);

	// start the main threads
	pthread_create(&stdin_thread,  NULL, ProcessStdin,       (void *)NULL);
	pthread_create(&server_thread, NULL, ProcessConnections, (void *)NULL);

	// run while the server lives!
	pthread_join(server_thread, NULL);

	// cleanup before exiting
	HandleExit();
	return 0;
}

void* ProcessStdin(void *) {
	std::string command;

	// process commands until time to exit
	do {
		// fetch input
		std::string input;
		std::getline(std::cin, input);

		// split input into arguments
		auto tokenized_cmd = stringutils::TokenizeString(input);
		command            = tokenized_cmd[0];

		// process command
		if (command == "exit") HandleExit();
	} while(command != "exit");

	return NULL;
}

void* ProcessConnections(void *) {
	int fd;
	
	// accept connections while the server lives
	while((fd = accept(server_descriptor, NULL, NULL)) != -1) {
		AddClient(fd);
		
		// broadcast that someone connected (maybe here, maybe only if signed in)
	}

	printf("\033[0;31madmin: no longer accepting new connections.\033[0m\n");

	return NULL;
}

void* ProcessMessages(void* fd) { 
	int nread, len;
	char sizeb[4], buffer[BUFSIZ];

	// read while the connection exists
	while((nread = recv(*(int *)fd, sizeb, sizeof(sizeb), 0)) > 0) {
		// get the message
		std::string message = "";
		memcpy(&len, sizeb, sizeof(int));
		for(int total_read = 0; total_read < len; total_read += nread) {
			nread = recv(*(int *)fd, buffer, len, 0);
			message.append(buffer, nread);
		}


		// collect some information about the message for logging
		time_t current_time  = time(NULL);
		struct tm* timeinfo  = localtime(&current_time);
		std::string username = "TODO";

		// display and broadcast the message
		printf(
			"\033[0;35m[%02d:%02d:%02d] <%s> (%d): %s\033[0m\n",
			timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
			username.c_str(),
			*(int *)fd,
			message.c_str()
		);
		BroadcastMessage(*(int *)fd, message);

		// do any additional work with the message
		ReceiveMessage(*(int *)fd, message);
	}

	// erase records of the client if they disconnect
	RemoveClient(*(int *)fd);

	return NULL;
}

void AddClient(const int fd) {
	struct sockaddr_in ca;
	char               domain[NI_MAXHOST];
	socklen_t          len = sizeof(ca);

	// add connection to our records
	auto itr = active_descriptors.insert(std::pair<int, pthread_t>(fd, pthread_t())).first;
	pthread_create(&itr->second, NULL, ProcessMessages, (void*)&itr->first);

	// output new connection details
	getpeername(fd, (struct sockaddr *)&ca, &len);
	getnameinfo((struct sockaddr *)&ca, sizeof(ca), domain, sizeof(domain), 0, 0, NI_NAMEREQD);
	printf("\033[0;31madmin: connect from '%s' at '%d'.\033[0m\n", domain, ntohs(ca.sin_port));
}

void RemoveClient(const int fd) {
	struct sockaddr_in ca;
	char               domain[NI_MAXHOST];
	socklen_t          len = sizeof(ca);

	// output removed connection details
	getpeername(fd, (struct sockaddr *)&ca, &len);
	getnameinfo((struct sockaddr *)&ca, sizeof(ca), domain, sizeof(domain), 0, 0, NI_NAMEREQD);
	printf("\033[0;31madmin: disconnect from '%s' at '%d'.\033[0m\n", domain, ntohs(ca.sin_port));

	// remove connection from our records
	shutdown(fd, SHUT_RDWR);
	active_descriptors.erase(fd);
}

void HandleExit() {
	shutdown(server_descriptor, SHUT_RDWR);
}

void ReceiveMessage(const int& fd, const std::string& msg) {
	// broadcast message to all clients
}

void BroadcastMessage(const int& fd, const std::string& msg) {
	std::string formatted_usn = "<TODO>: ";
	for(auto itr = active_descriptors.begin(); itr != active_descriptors.end(); ++itr) {
		if(itr->first != fd) {
			SendMessage(itr->first, formatted_usn + msg);
		}
	}
}
