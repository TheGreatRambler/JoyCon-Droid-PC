const WebSocket = require("ws");
const Helpers = require("./helpers");
const Gamecontroller = require("gamecontroller");
const Ip = require("ip");
const IoHook = require("iohook");
const KeyboardHandling = require("./keyboardHandling");
const AsciiTable = require("ascii-table");
const readline = require("readline").createInterface({
	input: process.stdin,
	output: process.stdout
})

const PORT = 6954;
// Milliseconds betwen update
const UPDATE_RATE = 16;

// Start server
var wss;

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

async function listGamepads() {
	var table = new AsciiTable("Pick input method");
	var devices = Gamecontroller.getDevices();
	table.setHeading("Index", "Name");
	// Keyboard is always an option
	table.addRow(0, "Keyboard");
	devices.forEach(function(name, index) {
		table.addRow(index + 1, name);
	});
	table.setAlign(0, AsciiTable.CENTER);
	table.setAlign(1, AsciiTable.CENTER);
	table.setJustify();
	console.log(table.toString());
}

async function init() {
	// Setup WS server
	setupServer();
	await listGamepads();
	readline.question("Index: ", function(index) {
		if (index == "0") {
			console.log("Keyboard chosen");
			// Chose the keyboard
			IoHook.start(false);
			// Start listening
			KeyboardHandling(IoHook);
		} else {
			// Chose a gamepad
			var gamepadName = Gamecontroller.getDevices(Number(index));
			console.log(gamepadName + " chosen");
		}
	});
}

// Start everything
// This is async as well
init();
// Listen for ctrl+c
console.log("Use CTRL+C to exit");
process.on("exit", function() {
	// Needs to be unloaded correctly
	IoHook.unload();
});