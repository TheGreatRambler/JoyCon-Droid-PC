#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_THREAD_

#include "libfort/fort.hpp"
#include <SDL.h>
#include <cstdint>
#include <cstdio>
#include <cxxopts.hpp>
#include <iostream>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "getIpAddress.hpp"

SDL_Joystick* joy;

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
		m_endpoint.set_message_handler (std::bind (
			&WebsocketServer::echo_handler, this,
			std::placeholders::_1, std::placeholders::_2));
	}

	void echo_handler (websocketpp::connection_hdl hdl, server::message_ptr msg) {
		// write a new message
		m_endpoint.send (hdl, msg->get_payload (), msg->get_opcode ());
	}

	void run () {
		// Listen on port 9002
		m_endpoint.listen (9002);

		// Queues a connection accept operation
		m_endpoint.start_accept ();

		// Start the Asio io_service run loop
		m_endpoint.run ();
	}
};

void listJoysticks (int8_t num_joysticks) {
	if (num_joysticks < 0) {
		printf ("No gamepads were found\n");
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
		int8_t num_joysticks                   = SDL_NumJoysticks ();
		if (commandLineResult["list"].as<bool> ()) {
			// List the avaliable gamepads and then exit
			puts ("Listing gamepads...");
			listJoysticks (num_joysticks);
		} else {
			// Run the normal application
			puts ("Type the index of the gamepad you wish to use");
			listJoysticks (num_joysticks);
			std::cout << "Please enter the index: ";
			std::string index;
			std::getline (std::cin, index);
			if (!index.empty) {
				int8_t chosenIndex = std::stoi (index);
				if (chosenIndex < num_joysticks && chosenIndex > -1) {
					printf ("Chosen joystick %d\n", chosenIndex);

					// Open up the Joystick
					joy = SDL_JoystickOpen (chosenIndex);

					// Get IP data
					// This is about how large an IP address will ever be
					char IPAddress[17];
					GetIP::getMyIP (IPAddress);
					printf ("Insert this IP address into JoyCon Droid: %s\n", IPAddress);

					// Open up the websocket server
					WebsocketServer server;
					server.run ();
				} else {
					printf ("Index %d is not in the correct bounds\n", chosenIndex);
				}
			} else {
				puts ("Gamepad not chosen, aborting");
			}
		}
	}
	// End with no error
	return 0;
}