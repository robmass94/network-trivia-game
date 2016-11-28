#include <iostream>
#include <pthread.h>
#include <map>
#include <signal.h>
#include "shared.h"
#include "stringutils.h"
#include "user.h"
#include "location.h"

// globals
int server_descriptor = socket(AF_INET, SOCK_STREAM, 0);

// client commands
void HandleExit ();

// thread functions
void* ProcessStdin       (void *);
void* ProcessMessages    (void *);

// other functions
void ClientInterruptHandler(int) {
	HandleExit();
	exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
	pthread_t        stdin_thread, message_thread;
	struct sigaction sigIntHandler;

	// connect to the server in the parameters
	if(!ConnectToServer(server_descriptor, argv[1], atoi(argv[2]))) {
		printf("\033[0;31msystem: ERROR - could not connect to the specified server.\033[0m\n");
		return 0;
	}

	// setup ctrl+c interceptor
	sigIntHandler.sa_handler = ClientInterruptHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	// ignore SIGPIPE
	signal(SIGPIPE, SIG_IGN);

	// start the main threads
	pthread_create(&stdin_thread,   NULL, ProcessStdin,    (void *)NULL);
	pthread_create(&message_thread, NULL, ProcessMessages, &server_descriptor);

	// run while the server lives!
	pthread_join(message_thread, NULL);

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

		// process commands
		if   (command == "exit") HandleExit();
		else                     SendMessage(server_descriptor, input);
	} while(command != "exit");

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

		// collect some information about the message for output
		time_t current_time  = time(NULL);
		struct tm* timeinfo  = localtime(&current_time);

		// display the message
		printf(
			"\033[0;35m[%02d:%02d:%02d] %s\033[0m\n",
			timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
			message.c_str()
		);
	}

	printf("\033[0;31msystem: server died.\033[0m\n");

	return NULL;
}

void HandleExit() {
	shutdown(server_descriptor, SHUT_RDWR);
}
