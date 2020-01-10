const WebSocket = require("ws");
const Helpers = require("./helpers");
const Gamecontroller = require("gamecontroller");
const Ip = require("ip");
const KeyboardHandling = require("./keyboardHandling");
const GamepadHandling = require("./gamepadHandling");
const AsciiTable = require("ascii-table");
const readline = require("readline").createInterface({
	input: process.stdin,
	output: process.stdout
});
const CONFIG = require("./config.json");

const PORT = 6954;
// Milliseconds betwen update
const UPDATE_RATE = 16;

// Start server
var wss;

// Main ws
var mainWsInstance;

var usingKeyboard;

function setupServer() {
	wss = new WebSocket.Server({
		port: PORT
	});

	// Listen for gamepad connections
	wss.on("connection", function(ws) {
		// Let the keyboard and gamepad code know
		KeyboardHandling.setWs(ws);
		mainWsInstance = ws;
		ws.on("message", function(message) {
			console.log('received: %s', message);
		});

		ws.on("close", function() {
			// Unset the instance
			KeyboardHandling.clearWs();
			mainWsInstance = undefined;
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
	await listGamepads();
	readline.question("Index: ", function(index) {
		if (index == "0") {
			usingKeyboard = true;
			console.log("Keyboard chosen");
			KeyboardHandling.keyboardHandling(CONFIG);
		} else {
			usingKeyboard = false;
			console.log(gamepadName + " chosen");
			var gamepadName = Gamecontroller.getDevices(Number(index));
			GamepadHandling.gamepadHandling(Gamecontroller);
		}
		// Setup server last
		setupServer();
	});
}

// Start everything
// This is async as well
init();
// Listen for ctrl+c
console.log("Use CTRL+C twice to exit");
process.on("exit", function() {
	// Needs to be unloaded correctly
	IoHook.unload();
});