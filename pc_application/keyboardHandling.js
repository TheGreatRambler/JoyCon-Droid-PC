const IoHook = require("iohook");
IoHook.start(false);

// IoHook likes to call keydown a lot, so this changes that
var keyChecker = [];

// When it isn't connected
var wsInstance;

module.exports.keyboardHandling = function(config) {
	var keys = config.keyboardKeys;
	IoHook.on("keydown", function(event) {
		// Check to see if key has been held before
		if (!keyChecker[event.rawcode]) {
			var key = String.fromCharCode(event.rawcode);
			if (wsInstance && keys[key]) {
				// Send the data
				wsInstance.send(JSON.stringify({
					flag: "button",
					type: keys[key],
					state: true
				}));
			}
			// It has been held, don't spam
			keyChecker[event.rawcode] = true;
		}
	});
	IoHook.on("keyup", function(event) {
		var key = String.fromCharCode(event.rawcode);
		if (wsInstance && keys[key]) {
			// Send the data
			wsInstance.send(JSON.stringify({
				flag: "button",
				type: keys[key],
				// Same thing as before, just the button has been unpressed
				state: false
			}));
		}
		// Automatically set to off
		keyChecker[event.rawcode] = false;
	});
};

module.exports.setWs = function(ws) {
	wsInstance = ws;
};

module.exports.clearWs = function() {
	// Set the variable as undefined
	wsInstance = undefined;
};