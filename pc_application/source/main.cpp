#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_THREAD_
#define RAPIDJSON_HAS_STDSTRING 1

#include "libfort/fort.hpp"
extern "C" {
#include <SDL.h>
}
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
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "getIpAddress.hpp"
#include "sdlGamepadToProcon.hpp"
#include "terminalhelpers.hpp"

SDL_GameController* joy;
rapidjson::Document configObject;
bool usingKeyboard;
bool usingFancy;
std::thread* inputPrintingThread;
TerminalHelpers::TerminalSize terminalSize;
// Random port
const int port  = 9664;
bool shouldStop = false;
// Use map to store keycodes
std::map<std::string, SDL_Scancode> keycodeConverter;
std::map<SDL_Scancode, std::string> keycodeConverterReverse;
std::map<std::string, SDL_GameControllerButton> gamepadConverter;
std::map<SDL_GameControllerButton, std::string> gamepadConverterReverse;

// Libfort printer
fort::char_table libfortPrinter;

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

void printInputDataFancy() {
	SDL_Event event;
	while(SDL_PollEvent(&event) && !shouldStop) {
		uint32_t type = event.type;

		// TODO handle drawing here
		if(usingKeyboard) {
			// https://www.libsdl.org/release/SDL-1.2.15/docs/html/guideinputkeyboard.html
			// Check for keyboard events
			if(type == SDL_KEYDOWN) {
				// Need to draw something
			} else if(type == SDL_KEYUP) {
				// Need to erase something
			}
		} else {
			// https://www.libsdl.org/release/SDL-1.2.15/docs/html/guideinput.html
			// Check for gamepad events
			if(type == SDL_JOYBUTTONDOWN) {
				// Need to draw something
			} else if(type == SDL_JOYBUTTONDOWN) {
				// Need to erase something
			}
		}
	}
}

void createKeyConverter() {
	// Get the JSON
	if(configObject.HasMember("keyboardKeys")) {
		for(auto& m : configObject["keyboardKeys"].GetObjectA()) {
			// These match SDL_Keycode (SDL_Keycode is just the Ascii code)
			int key               = (int)m.name.GetString()[0];
			SDL_Scancode scancode = SDL_GetScancodeFromKey(key);
			// Add the key to the keyboard mapping
			keycodeConverter.insert(std::pair<std::string, SDL_Scancode>(std::string(m.value.GetString()), scancode));
			keycodeConverterReverse.insert(std::pair<SDL_Scancode, std::string>(scancode, std::string(m.value.GetString())));
		}
	}
	if(configObject.HasMember("gamepadButtons")) {
		for(auto& m : configObject["gamepadButtons"].GetObjectA()) {
			SDL_GameControllerButton button = SDL_GameControllerGetButtonFromString(m.name.GetString());
			// Create the gamepad converter
			gamepadConverter.insert(std::pair<std::string, SDL_GameControllerButton>(m.value.GetString(), button));
			gamepadConverterReverse.insert(std::pair<SDL_GameControllerButton, std::string>(button, m.value.GetString()));
		}
	}
}

std::string constructReturnInputs(SDL_GameController* joystick) {
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);
	// Get data from SDL and write it to the output data
	// Set the flag to be return data
	// JSON starts here
	writer.StartObject();
	// Button data
	writer.Key("buttons");
	writer.StartObject();
	// https://wiki.libsdl.org/SDL_GameControllerButton
	// https://wiki.libsdl.org/SDL_GameControllerGetBindForButton
	// https://wiki.libsdl.org/SDL_GameControllerButtonBind
	// And so on
	// Get keyboard state if needed
	SDL_PumpEvents();
	const uint8_t* keyState = SDL_GetKeyboardState(NULL);
	for(auto const& button : gamepadToProcon) {
		writer.Key(button.second);
		if(usingKeyboard) {
			// Get the value from SDL
			if(gamepadConverter.count(button.second)) {
				writer.Bool(SDL_GameControllerGetButton(joystick, gamepadConverter[button.second]));
			} else {
				writer.Bool(SDL_GameControllerGetButton(joystick, button.first));
			}
		} else {
			if(keycodeConverter.count(button.second)) {
				// This button is remapped
				bool isOn = keyState[keycodeConverter[button.second]];
				writer.Bool(isOn);
			} else {
				// Always off
				writer.Bool(false);
			}
		}
	}
	// Buttons end here
	writer.EndObject();
	// JSON ends here
	writer.EndObject();
	return s.GetString();
}

class WebsocketServer {
private:
	websocketpp::server<websocketpp::config::asio> m_endpoint;

public:
	WebsocketServer() {
		// Set logging settings
		m_endpoint.set_error_channels(websocketpp::log::elevel::all);
		m_endpoint.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);

		// Initialize Asio
		m_endpoint.init_asio();

		// Set the default message handler to the echo handler
		m_endpoint.set_message_handler(websocketpp::lib::bind(&WebsocketServer::messageHandler, this, std::placeholders::_1, std::placeholders::_2));

		m_endpoint.set_open_handler(websocketpp::lib::bind(&WebsocketServer::openHandler, this, std::placeholders::_1));
	}

	void messageHandler(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg) {
		rapidjson::Document d;
		rapidjson::Document returnJson;
		d.Parse(msg->get_payload().c_str());
		std::string flagRecieved = d["flag"].GetString();
		if(flagRecieved == "frame") {
			SDL_GameControllerUpdate();
			// The endpoint wants the next frame, send it as json
			returnJson["flag"] = "frameInput";
			rapidjson::Value inputs(constructReturnInputs(joy), d.GetAllocator());
			returnJson["data"] = inputs;
		}
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		returnJson.Accept(writer);
		// Send the JSON data
		m_endpoint.send(hdl, buffer.GetString(), websocketpp::frame::opcode::text);
	}

	void openHandler(websocketpp::connection_hdl hdl) {
		// Do something now that a new connection has opened
	}

	void run() {
		// Listen on port specified earlier
		m_endpoint.listen(port);

		// Queues a connection accept operation
		m_endpoint.start_accept();

		// Start the Asio io_service run loop
		m_endpoint.run();
	}
};

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
		inputPrintingThread->join();
		exit(1);
	});

	// Set libfort printing method
	libfortPrinter.set_border_style(FT_BASIC_STYLE);

	// FIXME: We don't need video, but without it SDL will fail to work in
	// SDL_WaitEvent()
	if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) < 0) {
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	} else {
		atexit(SDL_Quit);
		cxxopts::Options commandLineOptions("JoyCon Droid PC", "Use PC gamepads with JoyCon Droid");
		// clang-format off
		commandLineOptions.add_options ()
			// https://github.com/jarro2783/cxxopts
			("l,list", "List all connected gamepads")
			("b,buttons", "Print the names of gamepad keys to use in config.json")
			("f,fancy", "Use an extra fancy UI")
	  		("c,config", "Set the path of the config file");
		// clang-format on
		cxxopts::ParseResult commandLineResult = commandLineOptions.parse(argc, argv);
		int8_t num_joysticks                   = SDL_NumJoysticks();
		usingFancy                             = commandLineResult.count("fancy") == 1;
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
			TerminalHelpers::incrementLineLarge();
			puts("Type the index of the gamepad you wish to use");
			listJoysticks(num_joysticks);
			std::cout << "Please enter the index: ";
			std::string index;
			std::getline(std::cin, index);
			if(!index.empty()) {
				int8_t chosenIndex = std::stoi(index);
				if(chosenIndex < num_joysticks + 1 && chosenIndex > -1) {
					TerminalHelpers::incrementLineLarge();
					printf("Chosen joystick %d\n", chosenIndex);

					if(usingFancy) {
						SDL_JoystickEventState(SDL_ENABLE);
					}

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
					TerminalHelpers::incrementLineLarge();
					puts("Please allow the Windows Defender popup to access your private and public network, if applicable");
#endif

					// Get IP data
					// This is about how large an IP address will ever be
					char IPAddress[17];
					GetIP::getMyIP(IPAddress, port);
					TerminalHelpers::incrementLineLarge();
					printf("Insert this IP address into JoyCon Droid: %s\n", IPAddress);

					// Handle gamepad and keyboard updating, not because it is needed to update values, but only to display
					// Run this in a thread
					if(usingFancy) {
						// Start thread
						// Create from thread so it can be global
						inputPrintingThread = new std::thread(printInputDataFancy);
					}

					// Open up the websocket server
					WebsocketServer server;
					// This will run for as long as needed
					server.run();
				} else {
					TerminalHelpers::incrementLineLarge();
					printf("Index %d is not in the correct bounds\n", chosenIndex);
				}
			} else {
				TerminalHelpers::incrementLineLarge();
				puts("Gamepad not chosen, aborting");
			}
		}
	}
	// On done, make sure everything is closed
	SDL_GameControllerClose(joy);
	SDL_Quit();
	// End with no error
	return 0;
}
