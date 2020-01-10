module.exports = function(ioHookInstance) {
	ioHookInstance.on("keydown", function(event) {
		console.log(event);
	});
	ioHookInstance.on("jeyup", function(event) {
		console.log(event);
	});
};