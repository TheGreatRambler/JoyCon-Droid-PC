const WebSocket = require('ws');
const Gamepad = require("node-gamepad");
const Ip = require("ip");

const PORT = 6954;
// Milliseconds betwen update
const UPDATE_RATE = 16;

// Start server
const wss;

function setupServer() {
	wss = new WebSocket.Server({
		port: PORT
	});

	// Listen for gamepad connections
	wss.on("connection", function(ws) {
		ws.on("message", function(message) {
			console.log('received: %s', message);
		});

		ws.send('something');
	});
}

function listGamepads() {
	for (let i = 0; i < Gamepad.numDevices(); i++) {
		// Print device here
		console.log(i, gamepad.deviceAtIndex());
	}
}

function init() {
	// Setup WS server
	setupServer();
	Gamepad.init();
	// Start main loop
	setInterval(function() {

	})
}