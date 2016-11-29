// ********************************************************************************************************************
//
// GUI is modelled after tbdchat
// https://github.com/mgeitz/tbdchat
//
// ********************************************************************************************************************
#include <ncurses.h>

extern WINDOW *mainWin, *inputWin, *chatWin, *chatWinBox, *inputWinBox, *infoLine, *infoLineBottom;

void InitColorPairs() {
	// tell ncurses that we will be using colors
	start_color();
	use_default_colors();

	// register some color pairs
	init_pair(1, -1,            -1);
	init_pair(2, COLOR_CYAN,    -1);
	init_pair(3, COLOR_YELLOW,  -1);
	init_pair(4, COLOR_RED,     -1);
	init_pair(5, COLOR_BLUE,    -1);
	init_pair(6, COLOR_MAGENTA, -1);
	init_pair(7, COLOR_GREEN,   -1);
}

void DrawChatWin() {
	// create a box to represent the chat area
	chatWinBox = subwin(mainWin, (LINES * 0.8), COLS, 0, 0);
	box(chatWinBox, 0, 0);

	// title our window
	mvwaddch(chatWinBox, 0, (COLS * 0.5) - 10, ACS_RTEE);
	wattron(chatWinBox, COLOR_PAIR(3));
	mvwaddstr(chatWinBox, 0, (COLS * 0.5) - 9, " Trivia Game ");
	wattroff(chatWinBox, COLOR_PAIR(3));
	mvwaddch(chatWinBox, 0, (COLS * 0.5) + 4, ACS_LTEE);
	wrefresh(chatWinBox);

	// create a sub window to hold output
	chatWin = subwin(chatWinBox, (LINES * 0.8 - 2), COLS - 2, 1, 1);

	// enable text scrolling
	scrollok(chatWin, TRUE);
}

void DrawInputWin() {
	// create a box to represent the user input area
	inputWinBox = subwin(mainWin, (LINES * 0.2) - 1, COLS, (LINES * 0.8) + 1, 0);
	box(inputWinBox, 0, 0);

	// create a sub window to hold user input
	inputWin = subwin(inputWinBox, (LINES * 0.2) - 3, COLS - 2, (LINES * 0.8) + 2, 1);
}

void DrawInfoLines() {
	infoLine = subwin(mainWin, 1, COLS, (LINES * 0.8), 0);

	// refer to help command
	wbkgd(infoLine, COLOR_PAIR(3));
	wprintw(infoLine, " Type /help to view a list of available commads");
	wrefresh(infoLine);

	// create lower line
	infoLineBottom = subwin(mainWin, 1, COLS, LINES - 1, 0);
}

void ResizeHandler(int sig) {
	// end current windows
	endwin();
	refresh();
	clear();

	// redraw windows
	DrawChatWin();
	DrawInputWin();
	DrawInfoLines();

	// refresh and move cursor to input window
	wrefresh(chatWin);
	wcursyncup(inputWin);
	wrefresh(inputWin);
}

void InitializeGUI() {
	mainWin = initscr();

	// set some ncurses settings
	noecho();
	cbreak();
	keypad(mainWin, TRUE);
	InitColorPairs();

	// draw stuff
	DrawChatWin();
	DrawInputWin();
	DrawInfoLines();
}
