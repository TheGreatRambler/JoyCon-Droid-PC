#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_THREAD_
#define RAPIDJSON_HAS_STDSTRING 1

#include "libfort/fort.hpp"
#include <SDL.h>
#include <cstdint>
#include <cstdio>
#include <cxxopts.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "getIpAddress.hpp"

SDL_GameController* joy;
rapidjson::Document configObject;
bool usingKeyboard;
// Random port
const int port = 9664;
// Use map to store keycodes TODO
std::map<SDL_GameControllerButton, SDL_Scancode> keycodeConverter;

std::string readFile (const std::string fileName) {
	// https://stackoverflow.com/a/43009155
	std::ifstream ifs (fileName.c_str (), std::ios::in | std::ios::binary | std::ios::ate);

	std::ifstream::pos_type fileSize = ifs.tellg ();
	if (fileSize < 0) {
		// Error was encountered
		// Return empty string
		return std::string ();
	}
	ifs.seekg (0, std::ios::beg);

	std::vector<char> bytes (fileSize);
	ifs.read (bytes.data (), fileSize);

	return std::string (bytes.data (), fileSize);
}

void createKeyConverter () {
	// Get the JSON
	for (auto& m : configObject["keyboardKeys"].GetObject ()) {
		// These match SDL_Keycode
		int key                             = (int)m.name.GetString ()[0];
		SDL_GameControllerButton gamepadKey = SDL_GameControllerGetButtonFromString (m.value.GetString ());
		SDL_Scancode scancode               = SDL_GetScancodeFromKey (key);
		// Add the key to the keyboard mapping
		keycodeConverter.insert (std::pair<SDL_GameControllerButton, SDL_Scancode> (gamepadKey, scancode));
	}
}

void addInput (SDL_GameController* joystick, const uint8_t* keyState, rapidjson::Writer<rapidjson::StringBuffer>& writer,
	std::string name, SDL_GameControllerButton buttonType) {
	writer.Key (name);
	if (!usingKeyboard) {
		// Read the gamepad, normal behavior
		// https://wiki.libsdl.org/SDL_GameControllerGetButton
		writer.Bool (SDL_GameControllerGetButton (joystick, buttonType));
	} else {
		// The user wants the keyboard, so time to shake things up
		// Check if the value exists in the map
		if (keycodeConverter.count (buttonType) == 1) {
			bool isOn = keyState[keycodeConverter[buttonType]];
			writer.Bool (isOn);
		}
	}
}

std::string constructReturnInputs (SDL_GameController* joystick) {
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer (s);
	// Get data from SDL and write it to the output data
	// Set the flag to be return data
	// JSON starts here
	writer.StartObject ();
	// Button data
	writer.Key ("buttons");
	writer.StartObject ();
	// https://wiki.libsdl.org/SDL_GameControllerButton
	// https://wiki.libsdl.org/SDL_GameControllerGetBindForButton
	// https://wiki.libsdl.org/SDL_GameControllerButtonBind
	// And so on
	// Get keyboard state if needed
	SDL_PumpEvents ();
	const uint8_t* keyState = SDL_GetKeyboardState (NULL);
	addInput (joy, keyState, writer, "a", SDL_CONTROLLER_BUTTON_A);
	// Buttons end here
	writer.EndObject ();
	// JSON ends here
	writer.EndObject ();
	return s.GetString ();
}
class WebsocketServer {
private:
	websocketpp::server<websocketpp::config::asio> m_endpoint;

public:
	WebsocketServer () {
		// Set logging settings
		m_endpoint.set_error_channels (websocketpp::log::elevel::all);
		m_endpoint.set_access_channels (websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);

		// Initialize Asio
		m_endpoint.init_asio ();

		// Set the default message handler to the echo handler
		m_endpoint.set_message_handler (websocketpp::lib::bind (&WebsocketServer::messageHandler, this, std::placeholders::_1, std::placeholders::_2));

		m_endpoint.set_open_handler (
			websocketpp::lib::bind (&WebsocketServer::openHandler, this, std::placeholders::_1));
	}

	void messageHandler (websocketpp::connection_hdl hdl,
		websocketpp::server<websocketpp::config::asio>::message_ptr msg) {
		rapidjson::Document d;
		rapidjson::Document returnJson;
		d.Parse (msg->get_payload ().c_str ());
		std::string flagRecieved = d["flag"].GetString ();
		if (flagRecieved == "frame") {
			// The endpoint wants the next frame, send it as json
			returnJson["flag"] = "frameInput";
			rapidjson::Value inputs (constructReturnInputs (joy), d.GetAllocator ());
			returnJson["data"] = inputs;
		}
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer (buffer);
		returnJson.Accept (writer);
		// Send the JSON data
		m_endpoint.send (hdl, buffer.GetString (), websocketpp::frame::opcode::text);
	}

	void openHandler (websocketpp::connection_hdl hdl) {
		// Do something now that a new connection has opened
	}

	void run () {
		// Listen on port specified earlier
		m_endpoint.listen (port);

		// Queues a connection accept operation
		m_endpoint.start_accept ();

		// Start the Asio io_service run loop
		m_endpoint.run ();
	}
};

void listJoysticks (int8_t num_joysticks) {
	puts ("0: Keyboard");
	// I think this handles all cases
	if (num_joysticks != -1) {
		for (uint8_t i = 0; i < num_joysticks; i++) {
			printf ("%d: %s\n", i + 1, SDL_JoystickNameForIndex (i));
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
		cxxopts::Options commandLineOptions ("JoyCon Droid PC",
			"Use PC gamepads with JoyCon Droid");
		// clang-format off
		commandLineOptions.add_options ()
			// https://github.com/jarro2783/cxxopts
			("l,list", "List all connected gamepads")
	  		("c,config", "Set the path of the config file");
		// clang-format on
		cxxopts::ParseResult commandLineResult = commandLineOptions.parse (argc, argv);
		int8_t num_joysticks                   = SDL_NumJoysticks ();
		if (commandLineResult["list"].as<bool> ()) {
			// List the avaliable gamepads and then exit
			listJoysticks (num_joysticks);
		} else {
			// Run the normal application
			// Get config data
			// count will only be true if the argument exists
			std::string configPath = commandLineResult.count ("config") == 1 ? commandLineResult["config"].as<std::string> () : "config.json";
			configObject.Parse (readFile (configPath).c_str ());
			// Set up keyboard mapping
			createKeyConverter ();
			puts ("Type the index of the gamepad you wish to use");
			listJoysticks (num_joysticks);
			std::cout << "Please enter the index: ";
			std::string index;
			std::getline (std::cin, index);
			if (!index.empty ()) {
				int8_t chosenIndex = std::stoi (index);
				if (chosenIndex < num_joysticks + 1 && chosenIndex > -1) {
					printf ("Chosen joystick %d\n", chosenIndex);

					if (chosenIndex == 0) {
						// Index 0 is always just the keyboard
						usingKeyboard = true;
					} else {
						// Using a normal gamepad
						usingKeyboard = false;
						// Open up the Joystick
						joy = SDL_GameControllerOpen (chosenIndex - 1);
					}

					// Get IP data
					// This is about how large an IP address will ever be
					char IPAddress[17];
					GetIP::getMyIP (IPAddress, port);
					printf ("Insert this IP address into JoyCon Droid: %s\n", IPAddress);

					// Open up the websocket server
					WebsocketServer server;
					// This will run for as long as needed
					server.run ();
				} else {
					printf ("Index %d is not in the correct bounds\n", chosenIndex);
				}
			} else {
				puts ("Gamepad not chosen, aborting");
			}
		}
	}
	// On done, make sure everything is closed
	SDL_GameControllerClose (joy);
	SDL_Quit ();
	// End with no error
	return 0;
}
