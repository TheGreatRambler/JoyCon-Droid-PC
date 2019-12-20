#pragma once

#include <SDL.h>
#include <map>

struct Loc {
	uint8_t x;
	uint8_t y;
}

std::map<SDL_GameControllerButton, std::string>
	gamepadToProcon {
		{ SDL_CONTROLLER_BUTTON_A, "B" },
		{ SDL_CONTROLLER_BUTTON_B, "A" },
		{ SDL_CONTROLLER_BUTTON_X, "Y" },
		{ SDL_CONTROLLER_BUTTON_Y, "X" },
		{ SDL_CONTROLLER_BUTTON_BACK, "MINUS" },
		{ SDL_CONTROLLER_BUTTON_GUIDE, "HOME" },
		{ SDL_CONTROLLER_BUTTON_START, "PLUS" },
		{ SDL_CONTROLLER_BUTTON_LEFTSTICK, "LSTICK" },
		{ SDL_CONTROLLER_BUTTON_RIGHTSTICK, "RSTICK" },
		{ SDL_CONTROLLER_BUTTON_LEFTSHOULDER, "LS" },
		{ SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, "RS" },
		{ SDL_CONTROLLER_BUTTON_DPAD_UP, "DUP" },
		{ SDL_CONTROLLER_BUTTON_DPAD_DOWN, "DDOWN" },
		{ SDL_CONTROLLER_BUTTON_DPAD_LEFT, "DLEFT" },
		{ SDL_CONTROLLER_BUTTON_DPAD_RIGHT, "DRIGHT" },
	};

// clang-format off
std::map<std::string, Loc> inputLocations {
	{"B", {}},
	{"A", {}},
	{"Y", {}},
	{"X", {}},
	{"MINUS", {}},
	{"HOME", {}},
	{"PLUS", {}},
	{"LSTICK", {}},
	{"RSTICK", {}},
	{"LS", {}},
	{"RS", {}},
	{"DUP", {}},
	{"DDOWN", {}},
	{"DLEFT", {}},
	{"DRIGHT", {}},
}
// clang-format on