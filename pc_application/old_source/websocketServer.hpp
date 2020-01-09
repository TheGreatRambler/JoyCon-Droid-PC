#pragma once

#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_THREAD_

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <exception>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

class WebsocketServer {
private:
	websocketpp::server<websocketpp::config::asio> m_endpoint;
	websocketpp::connection_hdl mainHdl;
	bool haveMainHDL;

	void sendJSON(websocketpp::connection_hdl hdl, rapidjson::Document& d);

public:
	WebsocketServer();

	void messageHandler(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg);

	void openHandler(websocketpp::connection_hdl hdl) {
		// Do something now that a new connection has opened
	}

	void sendGamepadButtonData(std::string inputName, bool state);

	void run(int port);

	void closeEverything();
};