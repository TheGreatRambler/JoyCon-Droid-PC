#pragma once

#include "websocketServer.hpp"

void WebsocketServer::sendJSON(websocketpp::connection_hdl hdl, rapidjson::Document& d) {
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	d.Accept(writer);
	// Send the JSON data
	m_endpoint.send(hdl, buffer.GetString(), websocketpp::frame::opcode::text);
}

WebsocketServer::WebsocketServer() {
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

void WebsocketServer::messageHandler(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg) {
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

void WebsocketServer::sendGamepadButtonData(std::string inputName, bool state) {
	// Send back input data right now
	if(haveMainHDL) {
		rapidjson::Document returnJson;
		returnJson["flag"] = "button";
		rapidjson::Value button(inputName.c_str(), returnJson.GetAllocator());
		returnJson["button"] = button;
		returnJson["state"]  = state;
		// Send back this button data
		sendJSON(mainHdl, returnJson);
	}
}

void WebsocketServer::run(int port) {
	// Listen on port specified earlier
	m_endpoint.listen(port);

	// Queues a connection accept operation
	m_endpoint.start_accept();
	// Start the Asio io_service run loop
	// This will block till done
	m_endpoint.run();
}

void WebsocketServer::closeEverything() {
	// Close it up cleanly
	m_endpoint.stop_perpetual();
	m_endpoint.stop_listening();
	websocketpp::lib::error_code ec;
	// There should only be one connection, mainHdl
	if(haveMainHDL) {
		m_endpoint.close(mainHdl, websocketpp::close::status::going_away, "", ec);
		if(ec) {
			std::cout << "> Error closing connection " << ec.message() << std::endl;
		}
	}
}