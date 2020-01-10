const IoHook = require("iohook");

// When it isn't connected
var wsInstance;

module.exports.keyboardHandling = function(config) {
	IoHook.start(false);
	var keys = config.keyboardKeys;
	IoHook.on("keydown", function(event) {
		//var key = String.fromCharCode(event.keycode + 69);
		//console.log(key);
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