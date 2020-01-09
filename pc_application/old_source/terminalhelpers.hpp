#pragma once

#ifdef _WIN32
#include <windows.h>
// Atrociously messes with RapidJson
#undef GetObject
#else
#include <sys/ioctl.h>
#endif
#include <cstdint>
#include <cstdio>
#include <unistd.h>

#define ESC char(0x1B)

namespace TerminalHelpers {
	uint8_t lastX;
	uint8_t lastY;
	struct TerminalSize {
		uint8_t x;
		uint8_t y;
	};
	TerminalSize getTerminalDimensions() {
		TerminalSize returnSize;
#ifdef _WIN32
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		returnSize.x = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		returnSize.y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
		struct winsize w;
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
		returnSize.x = w.ws_col;
		returnSize.y = w.ws_row;
#endif
		return returnSize;
	}

	void clearScreen() {
		printf("%c[2J", ESC);
	}

	void goToLocation(uint8_t x, uint8_t y) {
		// http://ascii-table.com/ansi-escape-sequences-vt-100.php
		lastX = x;
		lastY = y;
		printf("%c[%d;%dH", ESC, y, x);
	}

	void incrementLine() {
		// Go to next row
		goToLocation(lastX, lastY + 1);
	}

	void incrementLineLarge() {
		// Go extra far
		goToLocation(lastX, lastY + 2);
	}
}