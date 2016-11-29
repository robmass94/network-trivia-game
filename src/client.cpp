#include <pthread.h>
#include <signal.h>
#include <ncurses.h>
#include <sstream>
#include <string> 
#include "shared.h"
#include "gui.cpp"

// globals
int    server_descriptor = socket(AF_INET, SOCK_STREAM, 0);
WINDOW *mainWin, *inputWin, *chatWin, *chatWinBox, *inputWinBox, *infoLine, *infoLineBottom;

// thread functions
void* ProcessMessages (void* fd);
int   HandleUserInput (std::string& input);

int main(int argc, char** argv) {
	std::string input;
	int num_read;
	pthread_t message_thread;

	// start the gui!
	InitializeGUI();

	// ignore SIGPIPE
	signal(SIGPIPE, SIG_IGN);

	// start the server thread
	pthread_create(&message_thread, NULL, ProcessMessages, &server_descriptor);

	// main loop
	while(true) {
		wcursyncup(inputWin);

		// get user input
		input.clear();
		num_read = HandleUserInput(input);
		if(num_read > 0) {
			std::istringstream strm(input);
			std::string token;
			strm >> token;
			if (token == "/connect") {
				std::string host;
				std::string s_port;
				strm >> host;
				strm >> s_port;
				if (host.empty() || s_port.empty()) {
					wattron(chatWin, COLOR_PAIR(4));
					wprintw(chatWin, "Syntax: /connect [server hostname] [server port]\n");
					wattroff(chatWin, COLOR_PAIR(4));
				} else if (!ConnectToServer(server_descriptor, host.c_str(), stoi(s_port))) {
					wattron(chatWin, COLOR_PAIR(4));
					wprintw(chatWin, "Failed to connect to %s on port %s\n", host.c_str(), s_port.c_str());
					wattroff(chatWin, COLOR_PAIR(4));
				} else {
					wattron(chatWin, COLOR_PAIR(7));
					wprintw(chatWin, "You are now connected to %s on port %s\n", host.c_str(), s_port.c_str());
					wattroff(chatWin, COLOR_PAIR(7));
				}
				wrefresh(chatWin);
			} else {
				SendMessage(server_descriptor, input.c_str());
			}
		}
	}
	
	return 0;
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
		wattron(chatWin, COLOR_PAIR(6));
		wprintw(chatWin, 
			"[%02d:%02d:%02d] %s\n", 
			timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
			message.c_str()
		);
		wattroff(chatWin, COLOR_PAIR(6));
		wrefresh(chatWin);
		wcursyncup(inputWin);
	}

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
