var ESC = "\u001B";

module.exports = {
	// http://ascii-table.com/ansi-escape-sequences-vt-100.php
	goToTerminalLocation: function(x, y) {
		console.log(`${ESC}[${x};${y}H`);
	},

	saveTerminalLocation: function() {
		console.log(`${ESC}7`);
	},

	restoreTerminalLocation: function() {
		console.log(`${ESC}8`);
	},

	deleteLastChar: function() {
		console.log("\u007F");
	}
};