# Websocket Protocol

Connection between the PC and JoyCon Droid is established with Websockets, with the PC being the server. Primarily, the server sends data to JoyCon Droid, but in certain cases, JoyCon Droid may send its own messages.

## From PC to JoyCon Droid
### Connected
```
{
  "flag": "gamepadConnected" // The gamepad just connected
}
```
### Disconnected
```
{
  "flag": "gamepadDisconnected" // The gamepad just disconnected, also triggers when the PC server closes
}
```
### Button data
```
{
  "flag": "button",
  "type": "A", // Keyname for the button (eg. A, X, ZL, Plus, Down, LeftJoystick)
  "state": true // Whether on or off
}
```
### Joystick data
```
{
  "flag": "joystick",
  "stick": "L", // L for left joystick, R for right joystick
  "angle": pi/2, // Angle in radians of the stick movement
  "power" 70 // The power of the stick, from 0 to 100
}
```
### Gamepad disgnostic data
```
{
  "flag": "gamepadData",
  "name": "Pro Controller", // The name of the gamepad connected now
  "guid": "7638463219" // The GUID of the gamepad
}
```
## From JoyCon Droid to PC
### Start listening
```
{
  "flag": "startListening" // Start sending input data
}
```
### Stop listening
```
{
  "flag": "stopListening" // Stop sending input data
}
```
