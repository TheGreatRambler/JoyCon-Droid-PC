#include <SDL.h>
#include <cstdint>
#include <cstdio>
#include <cxxopts.hpp>
#include <iostream>

void listJoysticks () {
	int8_t num_joysticks = SDL_NumJoysticks ();
	if (num_joysticks < 0) {
		printf ("No joysticks were found\n");
	} else {
		for (uint8_t i = 0; i < num_joysticks; i++) {
			printf ("%d: %s\n", i, SDL_JoystickNameForIndex (i));
		}
	}
}

int main (int argc, char* argv[]) {
	puts ("Starting up SDL2...");
	// SDL2 will only report events when the window has focus, so set
	// this hint as we don't have a window
	SDL_SetHint (SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

	// FIXME: We don't need video, but without it SDL will fail to work in
	// SDL_WaitEvent()
	if (SDL_Init (SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) < 0) {
		fprintf (stderr, "Unable to init SDL: %s\n", SDL_GetError ());
		exit (1);
	} else {
		atexit (SDL_Quit);
		cxxopts::Options commandLineOptions ("JoyCon Droid PC", "Use PC gamepads with JoyCon Droid");
		// clang-format off
		commandLineOptions.add_options ()
			// https://github.com/jarro2783/cxxopts
			("l,list", "List all connected gamepads");
		// clang-format on
		cxxopts::ParseResult commandLineResult = commandLineOptions.parse (argc, argv);
		if (commandLineResult["list"].as<bool> ()) {
			// List the avaliable gamepads and then exit
			puts ("Finding gamepads...");
			listJoysticks ();
		} else {
			// Run the normal application
			puts ("Type the index of the gamepad you wish to use");
			listJoysticks ();
			std::cout << "Please enter the index: ";
			uint8_t chosenIndex;
			std::cin >> chosenIndex;
			printf ("Chose joystick %d", chosenIndex);
		}
		// End with no error
		return 0;
	}
}