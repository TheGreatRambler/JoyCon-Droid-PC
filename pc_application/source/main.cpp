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

SDL_GameController *joy;
// Random port
const int port = 9664;

void addInput(SDL_GameController *joystick, rapidjson::Writer writer,
              std::string name, SDL_GameControllerButton buttonType) {
  writer.Key(name);
  // https://wiki.libsdl.org/SDL_GameControllerGetButton
  writer.Bool(SDL_GameControllerGetButton(joystick, buttonType));
}

std::string constructReturnInputs(SDL_GameController *joystick) {
  rapidjson::StringBuffer s;
  rapidjson::Writer<StringBuffer> writer(s);
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
  addInput(joy, writer, "a", SDL_CONTROLLER_BUTTON_A);
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
    m_endpoint.set_access_channels(websocketpp::log::alevel::all ^
                                   websocketpp::log::alevel::frame_payload);

    // Initialize Asio
    m_endpoint.init_asio();

    // Set the default message handler to the echo handler
    m_endpoint.set_message_handler(std::bind(&WebsocketServer::messageHandler,
                                             this, std::placeholders::_1,
                                             std::placeholders::_2));

    m_endpoint.set_open_handler(
        std::bind(&WebsocketServer::openHandler, this, std::placeholders::_1));
  }

  void messageHandler(websocketpp::connection_hdl hdl,
                      server::message_ptr msg) {
    rapidjson::Document d;
    rapidjson::Document returnJson;
    d.parse(msg->get_payload());
    std::string flagRecieved = d["flag"].GetString();
    if (flagRecieved == "frame") {
      // The endpoint wants the next frame, send it as json
      returnJson["flag"] = "frameInput";
      returnJson["data"] = constructReturnInputs(joy);
    }
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    returnJson.Accept(writer);
    // Send the JSON data
    m_endpoint.send(hdl, buffer.GetString());
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
  if (num_joysticks < 0) {
    printf("No gamepads were found\n");
  } else {
    for (uint8_t i = 0; i < num_joysticks; i++) {
      printf("%d: %s\n", i, SDL_JoystickNameForIndex(i));
    }
  }
}

int main(int argc, char *argv[]) {
  puts("Starting up SDL2...");
  // SDL2 will only report events when the window has focus, so set
  // this hint as we don't have a window
  SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

  // FIXME: We don't need video, but without it SDL will fail to work in
  // SDL_WaitEvent()
  if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK |
               SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) < 0) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  } else {
    atexit(SDL_Quit);
    cxxopts::Options commandLineOptions("JoyCon Droid PC",
                                        "Use PC gamepads with JoyCon Droid");
    // clang-format off
		commandLineOptions.add_options ()
			// https://github.com/jarro2783/cxxopts
			("l,list", "List all connected gamepads");
    // clang-format on
    cxxopts::ParseResult commandLineResult =
        commandLineOptions.parse(argc, argv);
    int8_t num_joysticks = SDL_NumJoysticks();
    if (commandLineResult["list"].as<bool>()) {
      // List the avaliable gamepads and then exit
      puts("Listing gamepads...");
      listJoysticks(num_joysticks);
    } else {
      // Run the normal application
      puts("Type the index of the gamepad you wish to use");
      listJoysticks(num_joysticks);
      std::cout << "Please enter the index: ";
      std::string index;
      std::getline(std::cin, index);
      if (!index.empty) {
        int8_t chosenIndex = std::stoi(index);
        if (chosenIndex < num_joysticks && chosenIndex > -1) {
          printf("Chosen joystick %d\n", chosenIndex);

          // Open up the Joystick
          joy = SDL_GameControllerOpen(chosenIndex);

          // Get IP data
          // This is about how large an IP address will ever be
          char IPAddress[17];
          GetIP::getMyIP(IPAddress, port);
          printf("Insert this IP address into JoyCon Droid: %s\n", IPAddress);

          // Open up the websocket server
          WebsocketServer server;
          server.run();
        } else {
          printf("Index %d is not in the correct bounds\n", chosenIndex);
        }
      } else {
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