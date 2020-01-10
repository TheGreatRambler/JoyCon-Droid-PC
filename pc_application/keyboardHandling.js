const IoHook = require("iohook");
IoHook.start(true);

// When it isn't connected
var wsInstance;

module.exports.keyboardHandling = function(config) {
	var keys = config.keyboardKeys;
	IoHook.on("keydown", function(event) {
		var key = String.fromCharCode(event.rawcode);
	});
	IoHook.on("keyup", function(event) {});
};

module.exports.setWs = function(ws) {
	wsInstance = ws;
};

module.exports.clearWs = function() {
	// Set the variable as undefined
	wsInstance = undefined;
};