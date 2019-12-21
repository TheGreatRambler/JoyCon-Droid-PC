#pragma once

#include <SDL.h>
#include <map>

struct Loc {
	uint8_t x;
	uint8_t y;
};

std::map<SDL_GameControllerButton, std::string> gamepadToProcon {
	{ SDL_CONTROLLER_BUTTON_A, "B" },
	{ SDL_CONTROLLER_BUTTON_B, "A" },
	{ SDL_CONTROLLER_BUTTON_X, "Y" },
	{ SDL_CONTROLLER_BUTTON_Y, "X" },
	{ SDL_CONTROLLER_BUTTON_BACK, "MINUS" },
	{ SDL_CONTROLLER_BUTTON_GUIDE, "HOME" },
	{ SDL_CONTROLLER_BUTTON_START, "PLUS" },
	{ SDL_CONTROLLER_BUTTON_LEFTSTICK, "LSTICK" },
	{ SDL_CONTROLLER_BUTTON_RIGHTSTICK, "RSTICK" },
	{ SDL_CONTROLLER_BUTTON_LEFTSHOULDER, "ZL" },
	{ SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, "ZR" },
	{ SDL_CONTROLLER_BUTTON_DPAD_UP, "DUP" },
	{ SDL_CONTROLLER_BUTTON_DPAD_DOWN, "DDOWN" },
	{ SDL_CONTROLLER_BUTTON_DPAD_LEFT, "DLEFT" },
	{ SDL_CONTROLLER_BUTTON_DPAD_RIGHT, "DRIGHT" },
};

// Fancy input displaying locations
// https://www.tablesgenerator.com/text_tables
//      0   1    2    3   4  5   6   7   8    9  10
//   +----+---+----+----+---+--+---+---+----+---+---+
// 0 | LS |   | ZL | L  |   |  |   | R | ZR |   |   |
//   +----+---+----+----+---+--+---+---+----+---+---+
// 1 |    | ^ |    | -  |   |  |   | + |    | X |   |
//   +----+---+----+----+---+--+---+---+----+---+---+
// 2 | <  |   | >  |    | C |  | H |   | Y  |   | A |
//   +----+---+----+----+---+--+---+---+----+---+---+
// 3 |    | v |    | RS |   |  |   |   |    | B |   |
//   +----+---+----+----+---+--+---+---+----+---+---+
// clang-format off
std::map<std::string, Loc> inputLocations {
	{"B", {9, 3}},
	{"A", {10, 2}},
	{"Y", {8, 2}},
	{"X", {9, 1}},
	{"MINUS", {3, 1}},
	{"HOME", {6, 2}},
	{"PLUS", {7, 1}},
	{"LSTICK", {0, 0}},
	{"RSTICK", {3, 3}},
	{"ZL", {2, 0}},
	{"ZR", {8, 0}},
	{"DUP", {1, 1}},
	{"DDOWN", {1, 3}},
	{"DLEFT", {0, 2}},
	{"DRIGHT", {2, 2}},
};
const uint8_t InputLocationsWidth = 11;
const uint8_t InputLocationsHeight = 4;
// Size of the largest input name, 6 for RSTICK and LSTICK
const uint8_t InputLocationsLargestString = 6;
// clang-format on