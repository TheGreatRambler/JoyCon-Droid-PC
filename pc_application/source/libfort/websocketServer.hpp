#pragma once

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

class WebsocketServer {
private:
	websocketpp::server<websocketpp::config::asio> m_endpoint;
	websocketpp::connection_hdl mainHdl;
	bool haveMainHDL;

	void sendJSON(websocketpp::connection_hdl hdl, rapidjson::Document d) {
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		d.Accept(writer);
		// Send the JSON data
		m_endpoint.send(hdl, buffer.GetString(), websocketpp::frame::opcode::text);
	}

public:
	WebsocketServer() {
		haveMainHDL = false;
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
		if(flagRecieved == "pair") {
			// The endpoint has initiated pairing, this is now the main connection
			mainHdl            = hdl;
			haveMainHDL        = true;
			returnJson["flag"] = "returnPair";
		}
		// Send JSON back
		sendJSON(hdl, returnJson);
	}

	void openHandler(websocketpp::connection_hdl hdl) {
		// Do something now that a new connection has opened
	}

	void sendGamepadButtonData(std::string inputName, bool state) {
		// Send back input data right now
		if(haveMainHDL) {
			rapidjson::Document returnJson;
			returnJson["flag"] = "button";
			rapidjson::Value button(inputName, returnJson.GetAllocator());
			returnJson["button"] = button;
			returnJson["state"]  = state;
			// Send back this button data
			sendJSON(mainHdl, returnJson);
		}
	}

	void run(int port) {
		// Listen on port specified earlier
		m_endpoint.listen(port);

		// Queues a connection accept operation
		m_endpoint.start_accept();
	}

	void closeEverything() {
		// Close it up cleanly
		m_endpoint.stop_listening();
		// Start the Asio io_service run loop
		// This will block till done
		m_endpoint.run();
		m_endpoint.close();
	}
};