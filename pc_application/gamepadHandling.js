// When it isn't connected;
var wsInstance;

module.exports.gamepadHandling = function(gamepadInstance) {

};

module.exports.setWs = function(ws) {
	wsInstance = ws;
};

module.exports.clearWs = function() {
	// Set the variable as undefined
	wsInstance = undefined;
};