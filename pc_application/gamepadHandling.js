const Gamecontroller = require("gamecontroller");

// When it isn't connected;
var wsInstance;

module.exports.gamepadHandling = function(gamepadName, config) {
	// Create gamepad with the name
	var controller = new Gamecontroller(gamepadName);
	controller.connect(function() {
		// Controller connected now
		// Use special binding or just plain default
		var gamepadKeys = config.gamepadKeys[gamepadName] ? config.gamepadKeys[gamepadName] : config.gamepadKeys.default;
		// Add function for each mapped XBOX key
		Object.keys(gamepadKeys).forEach(function(key) {
			// Create function for each one
			controller.on(key + ":press", function() {
				console.log(key);
				// Key pressed
				if (wsInstance) {
					// Send the data
					wsInstance.send(JSON.stringify({
						flag: "button",
						type: gamepadKeys[key],
						state: true
					}));
				}
			});
			controller.on(key + ":release", function() {
				console.log(key);
				// Key released
				if (wsInstance) {
					// Send the data
					wsInstance.send(JSON.stringify({
						flag: "button",
						type: gamepadKeys[key],
						state: false
					}));
				}
			});
		});
	});
};

module.exports.setWs = function(ws) {
	wsInstance = ws;
};

module.exports.clearWs = function() {
	// Set the variable as undefined
	wsInstance = undefined;
};