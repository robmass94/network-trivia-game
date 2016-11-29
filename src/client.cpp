#include <pthread.h>
#include <signal.h>
#include <ncurses.h>
#include <sstream>
#include <string> 
#include "stringutils.h"
#include "shared.h"
#include "gui.cpp"

// globals
int       server_descriptor;
WINDOW    *mainWin, *inputWin, *chatWin, *chatWinBox, *inputWinBox, *infoLine, *infoLineBottom;
pthread_t message_thread;
bool isConnected;

// thread functions
void* ProcessMessages (void* fd);
int   HandleUserInput (std::string& input);

// helper stuffs
void ProcessUserCommand   (const std::string&              input);
void HandleConnect        (const std::vector<std::string>& argv);
void HandleHelp           ();
void HandleDisconnect     ();
void HandleUnknownCommand ();
void PrintServerMessage   (const std::string& msg);
void PrintUserMessage     (const std::string& msg);

int main(int argc, char** argv) {
	std::string input;
	int num_read;

	// start the gui!
	InitializeGUI();

	// ignore SIGPIPE
	signal(SIGPIPE, SIG_IGN);

	isConnected = false;

	// main loop
	while(true) {
		wcursyncup(inputWin);

		// get user input
		input.clear();
		num_read = HandleUserInput(input);
		
		if(num_read > 0) {
			if(input[0] == '/') ProcessUserCommand(input);
			else                SendMessage(server_descriptor, input.c_str());
		}
	}
	
	return 0;
}

void ProcessUserCommand(const std::string& input) {
	auto tokenized_cmd  = stringutils::TokenizeString(input);
	std::string command = tokenized_cmd[0];

	if      (command == "/connect")    HandleConnect(tokenized_cmd);
	else if (command == "/help")       HandleHelp();
	else if (command == "/disconnect") HandleDisconnect();
	else                               HandleUnknownCommand();
}

void HandleConnect(const std::vector<std::string>& argv) {
	// ensure proper input
	if(argv.size() != 3) {
		wattron(chatWin, COLOR_PAIR(4));
		wprintw(chatWin, "ERROR: syntax: /connect [server hostname] [server port]\n");
		wattroff(chatWin, COLOR_PAIR(4));
		wrefresh(chatWin);
		return;
	}

	// attempt connection
	std::string addr = argv[1];
	int         port = stoi(argv[2]);
	server_descriptor = socket(AF_INET, SOCK_STREAM, 0); 
	if(!ConnectToServer(server_descriptor, addr.c_str(), port)) {
		wattron(chatWin, COLOR_PAIR(4));
		wprintw(chatWin, "ERROR: failed to connect to '%s' on port '%d'.\n", addr.c_str(), port);
		wattroff(chatWin, COLOR_PAIR(4));
		wrefresh(chatWin);
	} else {
		// start monitoring messages on this socket
		pthread_create(&message_thread, NULL, ProcessMessages, &server_descriptor);

		// print connection details
		werase(infoLineBottom);
		wattron(infoLineBottom, COLOR_PAIR(3));
		wprintw(infoLineBottom, "Connected to '%s' on port '%d'", addr.c_str(), port);
		wattroff(infoLineBottom, COLOR_PAIR(3));
		wrefresh(infoLineBottom);

		isConnected = true;
	}
}

void HandleHelp() {
	wattron(chatWin, COLOR_PAIR(2));
	wprintw(chatWin, "/connect [server hostname] [server port]");
	wattroff(chatWin, COLOR_PAIR(2));
	wprintw(chatWin, " : connect to server with given hostname on given port\n");

	wattron(chatWin, COLOR_PAIR(2));
	wprintw(chatWin, "/disconnect");
	wattroff(chatWin, COLOR_PAIR(2));
	wprintw(chatWin, " : disconnect from server to which client is currently connected\n");

	wattron(chatWin, COLOR_PAIR(2));
	wprintw(chatWin, "/help");
	wattroff(chatWin, COLOR_PAIR(2));
	wprintw(chatWin, " : display this list of commands\n");
	
	wrefresh(chatWin);
}

void HandleDisconnect() {
	if (isConnected) {
		shutdown(server_descriptor, SHUT_RDWR);
		close(server_descriptor);
		werase(infoLineBottom);
		isConnected = false;
	} else {
		werase(infoLineBottom);
		wattron(infoLineBottom, COLOR_PAIR(3));
		wprintw(infoLineBottom, "Not connected to a server."); // ideally we just clear the line
		wattroff(infoLineBottom, COLOR_PAIR(3));
		wrefresh(infoLineBottom);
	}
}

void HandleUnknownCommand() {
	wattron(chatWin, COLOR_PAIR(4));
	wprintw(chatWin, "ERROR: unknown command.\n");
	wattroff(chatWin, COLOR_PAIR(4));
	wrefresh(chatWin);	
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
		time_t current_time = time(NULL);
		struct tm* timeinfo = localtime(&current_time);

		// display the message
		wprintw(chatWin, "[%02d:%02d:%02d] ", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		if      (message[0] == '0') PrintServerMessage(message.substr(2));
		else if (message[0] == '1') PrintUserMessage(message.substr(2));
		wcursyncup(inputWin);
	}

	HandleDisconnect();

	return NULL;
}

int HandleUserInput(std::string& input) {
	int i = 0;
	int ch;
	wmove(inputWin, 0, 0);
	wrefresh(inputWin);
	
	// read 1 char at a time till nelinw
	while((ch = getch()) != '\n') {
		if(ch == 8 || ch == 127 || ch == KEY_LEFT) {
			if(i > 0) {
				wprintw(inputWin, "\b \b\0");
				input.pop_back();
				wrefresh(inputWin);
				--i;
			}
		} else if(ch != ERR) {
			++i;
			input.append((char *)&ch);
			wprintw(inputWin, (char *)&ch);
			wrefresh(inputWin);
		}
	}
	wclear(inputWin);
	wrefresh(inputWin);
	return i;
}

void PrintServerMessage(const std::string& msg) {
	wattron(chatWin, COLOR_PAIR(2));
	wprintw(chatWin, "%s\n", msg.c_str());
	wattroff(chatWin, COLOR_PAIR(2));
	wrefresh(chatWin);
}

void PrintUserMessage(const std::string& msg) {
	auto tokenized_msg = stringutils::TokenizeString(msg);
	
	// print username
	wattron(chatWin, COLOR_PAIR(7));
	wprintw(chatWin, "<%s>: ", tokenized_msg[0].c_str());
	wattroff(chatWin, COLOR_PAIR(7));

	// print message
	wprintw(chatWin, "%s\n", stringutils::Join(tokenized_msg, 1).c_str());
	wrefresh(chatWin);	
}
