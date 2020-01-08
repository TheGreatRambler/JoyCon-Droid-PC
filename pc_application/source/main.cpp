#define RAPIDJSON_HAS_STDSTRING 1

#include "libfort/fort.hpp"
#include <SDL.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cxxopts.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <rang.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <signal.h>
#include <string>
#include <termcolor.hpp>
#include <thread>

#include "getIpAddress.hpp"
#include "sdlGamepadToProcon.hpp"
#include "terminalhelpers.hpp"
#include "websocketServer.hpp"

// This needs to be defined
WebsocketServer serverInstance;
SDL_GameController* joy;
rapidjson::Document configObject;

bool usingKeyboard;
bool usingInputsDisplay;
bool usingFancy;

std::thread* websocketServerThread;

TerminalHelpers::TerminalSize terminalSize;
// Random port
const int port  = 9664;
bool shouldStop = false;

// Use map to store keycodes
// Despite the names, these are the only converters
std::map<SDL_Scancode, std::string> keycodeConverterReverse;
std::map<SDL_GameControllerButton, std::string> gamepadConverterReverse;

// Libfort printer
fort::char_table libfortPrinter;
// For the input display
fort::char_table inputDisplay;

std::string readFile(const std::string fileName) {
	// https://stackoverflow.com/a/43009155
	std::ifstream ifs(fileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

	std::ifstream::pos_type fileSize = ifs.tellg();
	if(fileSize < 0) {
		// Error was encountered
		// Return empty string
		return std::string();
	}
	ifs.seekg(0, std::ios::beg);

	std::vector<char> bytes(fileSize);
	ifs.read(bytes.data(), fileSize);

	return std::string(bytes.data(), fileSize);
}

void printGamepadButtonNames() {
	for(auto const& button : gamepadToProcon) {
		printf("%s: %s\n", SDL_GameControllerGetStringForButton(button.first), button.second.c_str());
	}
}

void printInputsTable() {
	// Approximate location
	TerminalHelpers::goToLocation(3, terminalSize.y - InputLocationsHeight * 2 - 2);
	std::istringstream iss(inputDisplay.to_string());
	for(std::string line; std::getline(iss, line);) {
		std::cout << line;
		TerminalHelpers::incrementLine();
	}
}

void inputHandlingFunction() {
	puts("Starting input handling");
	SDL_Event event;
	while(SDL_PollEvent(&event) && !shouldStop) {
		uint32_t type         = event.type;
		bool somethingChanged = false;

		// TODO handle drawing here
		Loc inputLoc;
		bool newState;
		std::string inputName;
		if(usingKeyboard) {
			puts("accepting inputs");
			// https://www.libsdl.org/release/SDL-1.2.15/docs/html/guideinputkeyboard.html
			// Check for keyboard events
			if(type == SDL_KEYDOWN) {
				// Need to draw something
				somethingChanged = true;
				inputName        = keycodeConverterReverse[event.key.keysym.scancode];
				newState         = true;
				if(usingFancy) {
					inputLoc                             = inputLocations[inputName];
					inputDisplay[inputLoc.y][inputLoc.x] = inputName;
				}
			} else if(type == SDL_KEYUP) {
				// Need to erase something
				somethingChanged = true;
				inputName        = keycodeConverterReverse[event.key.keysym.scancode];
				newState         = false;
				if(usingFancy) {
					inputLoc = inputLocations[inputName];
					// Empty it
					inputDisplay[inputLoc.y][inputLoc.x] = "";
				}
			}
		} else {
			// https://www.libsdl.org/release/SDL-1.2.15/docs/html/guideinput.html
			// Check for gamepad events
			if(type == SDL_JOYBUTTONDOWN) {
				// Need to draw something
				somethingChanged = true;
				inputName        = gamepadConverterReverse[(SDL_GameControllerButton)event.cbutton.button];
				newState         = true;
				if(usingFancy) {
					inputLoc                             = inputLocations[inputName];
					inputDisplay[inputLoc.y][inputLoc.x] = inputName;
				}
			} else if(type == SDL_JOYBUTTONDOWN) {
				// Need to erase something
				somethingChanged = true;
				inputName        = gamepadConverterReverse[(SDL_GameControllerButton)event.cbutton.button];
				newState         = false;
				if(usingFancy) {
					inputLoc = inputLocations[inputName];
					// Empty it
					inputDisplay[inputLoc.y][inputLoc.x] = "";
				}
			}
		}

		if(somethingChanged) {
			puts(inputName.c_str());
			// Send inputs over websocket
			serverInstance.sendGamepadButtonData(inputName, newState);
			// Redraw table now
			if(usingFancy) {
				printInputsTable();
			}
		}
	}
}

void runWebsocketServer() {
	// The problem is WebSocketpp blocks, so this has to be in a separate thread
	serverInstance.run(port);
}

void setUpInputFancyViewer() {
	for(uint8_t i = 0; i < InputLocationsHeight; i++) {
		inputDisplay.row(i).set_cell_text_align(fort::text_align::center);
		inputDisplay.row(i).set_cell_min_width(InputLocationsLargestString);
		for(uint8_t j = 0; j < InputLocationsWidth; j++) {
			inputDisplay[i][j] = "";
		}
	}
	inputDisplay.set_border_style(FT_BASIC_STYLE);
	// Print it while it's empty
	printInputsTable();
}

void createKeyConverter() {
	// Get the JSON
	if(configObject.HasMember("keyboardKeys")) {
		for(auto& m : configObject["keyboardKeys"].GetObjectA()) {
			// These match SDL_Keycode (SDL_Keycode is just the Ascii code)
			int key               = (int)m.name.GetString()[0];
			SDL_Scancode scancode = SDL_GetScancodeFromKey(key);
			// Add the key to the keyboard mapping
			keycodeConverterReverse.insert(std::pair<SDL_Scancode, std::string>(scancode, std::string(m.value.GetString())));
		}
	}
	if(configObject.HasMember("gamepadButtons")) {
		for(auto& m : configObject["gamepadButtons"].GetObjectA()) {
			SDL_GameControllerButton button = SDL_GameControllerGetButtonFromString(m.name.GetString());
			// Create the gamepad converter
			gamepadConverterReverse.insert(std::pair<SDL_GameControllerButton, std::string>(button, m.value.GetString()));
		}
	}
}

void listJoysticks(int8_t num_joysticks) {
	if(usingFancy) {
		// clang-format off
		libfortPrinter << fort::header
			<< "Index" << "Controller" << "GUID" << fort::endr
			<< "0" << "Keyboard" << "" << fort::endr;
		// clang-format on
	} else {
		puts("0: Keyboard");
	}
	// I think this handles all cases
	if(num_joysticks != -1) {
		for(uint8_t i = 0; i < num_joysticks; i++) {
			SDL_GameController* tempJoy;
			tempJoy = SDL_GameControllerOpen(i);
			char guid[40];
			SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(SDL_GameControllerGetJoystick(tempJoy)), guid, 40);
			if(usingFancy) {
				libfortPrinter << std::to_string(i + 1) << SDL_JoystickNameForIndex(i) << guid << fort::endr;
			} else {
				printf("%d: %s %s\n", i + 1, SDL_JoystickNameForIndex(i), guid);
			}
			// No longer needed
			SDL_GameControllerClose(tempJoy);
		}
	}
	if(usingFancy) {
		TerminalHelpers::goToLocation(3, 4);
		std::istringstream iss(libfortPrinter.to_string());
		for(std::string line; std::getline(iss, line);) {
			std::cout << line;
			// This is used to preserve the column
			TerminalHelpers::incrementLine();
		}
	}
}

int main(int argc, char* argv[]) {
	// SDL2 will only report events when the window has focus, so set
	// this hint as we don't have a window
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

	// Quit on Ctrl + C, VERY IMPORTANT
	signal(SIGINT, [](int s) {
		shouldStop = true;
		// Wait for it to end
		if(websocketServerThread) {
			// Calling this from another thread, so there might be race conditions
			serverInstance.closeEverything();
		}
		// No need to join the thread, it has been informed and it will stop
		// Whether it likes it or not
		exit(0);
	});
	if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) < 0) {
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	} else {

		// Set libfort printing method
		libfortPrinter.set_border_style(FT_BASIC_STYLE);

		// FIXME: We don't need video, but without it SDL will fail to work in
		// SDL_WaitEvent()
		atexit(SDL_Quit);
		cxxopts::Options commandLineOptions("JoyCon Droid PC", "Use PC gamepads with JoyCon Droid");
		// clang-format off
		commandLineOptions.add_options ()
			// https://github.com/jarro2783/cxxopts
			("l,list", "List all connected gamepads")
			("b,buttons", "Print the names of gamepad keys to use in config.json")
			("f,fancy", "Use an extra fancy UI")
			("i,inputs", "Display inputs in real time, requires fancy UI")
	  		("c,config", "Set the path of the config file");
		// clang-format on
		cxxopts::ParseResult commandLineResult = commandLineOptions.parse(argc, argv);
		int8_t num_joysticks                   = SDL_NumJoysticks();
		usingFancy                             = commandLineResult.count("fancy") == 1;
		usingInputsDisplay                     = commandLineResult.count("inputs") == 1;
		if(usingFancy) {
			puts("Using fancy UI");
			// Use fancy method of displaying text
			terminalSize = TerminalHelpers::getTerminalDimensions();
			// Clear screen for the next stuff
			TerminalHelpers::clearScreen();
		}
		if(commandLineResult["buttons"].as<bool>()) {
			// Print the buttons then exit
			printGamepadButtonNames();
		} else if(commandLineResult["list"].as<bool>()) {
			// List the avaliable gamepads and then exit
			listJoysticks(num_joysticks);
		} else {
			// Run the normal application
			// Get config data
			// count will only be true if the argument exists
			std::string configPath = commandLineResult.count("config") == 1 ? commandLineResult["config"].as<std::string>() : "config.json";
			configObject.Parse(readFile(configPath).c_str());
			// Set up keyboard mapping
			createKeyConverter();
			if(usingFancy) {
				TerminalHelpers::goToLocation(3, 3);
			}
			puts("Type the index of the gamepad you wish to use");
			listJoysticks(num_joysticks);
			std::cout << "Please enter the index: ";
			std::string index;
			std::getline(std::cin, index);
			if(!index.empty()) {
				int8_t chosenIndex = std::stoi(index);
				if(chosenIndex < num_joysticks + 1 && chosenIndex > -1) {
					if(usingFancy) {
						TerminalHelpers::incrementLineLarge();
					}

					printf("Chosen joystick %d\n", chosenIndex);

					SDL_JoystickEventState(SDL_ENABLE);

					if(chosenIndex == 0) {
						// Index 0 is always just the keyboard
						usingKeyboard = true;
					} else {
						// Using a normal gamepad
						usingKeyboard = false;
						// Open up the Joystick
						joy = SDL_GameControllerOpen(chosenIndex - 1);
					}

// Windows Defender is gonna pop up here
#ifdef _WIN32
					if(usingFancy) {
						TerminalHelpers::incrementLineLarge();
					}

					puts("Please allow the Windows Defender popup to access your private and public network, if applicable");
#endif

					// Get IP data
					// This is about how large an IP address will ever be
					char IPAddress[17];
					GetIP::getMyIP(IPAddress, port);
					if(usingFancy) {
						TerminalHelpers::incrementLineLarge();
					}

					printf("Insert this IP address into JoyCon Droid: %s\n", IPAddress);

					// This will run for as long as needed
					// The idea is that this doesn't block
					serverInstance.run(port);
					// Start thread
					setUpInputFancyViewer();
					// Create from thread so it can be global
					websocketServerThread = new std::thread(runWebsocketServer);

					// Finally, start handing the inputs, no thread
					inputHandlingFunction();
				} else {
					if(usingFancy) {
						TerminalHelpers::incrementLineLarge();
					}
					printf("Index %d is not in the correct bounds\n", chosenIndex);
				}
			} else {
				if(usingFancy) {
					TerminalHelpers::incrementLineLarge();
				}

				puts("Gamepad not chosen, aborting");
			}
		}
		// On done, make sure everything is closed
		SDL_GameControllerClose(joy);
		SDL_Quit();
	}
	// End with no error
	return 0;
}
