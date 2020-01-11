import React, { Component } from 'react';
import { withStyles } from "@material-ui/core/styles";
import Button from '@material-ui/core/Button';
import TextField from '@material-ui/core/TextField';
import './App.css';

const styles = {
  root: {
    background: "white",

  },
  button: {
    height: 40,
    margin: 4
  }

};

const methodMap = {
  Up: "onUp",
  Down: "onDown",
  Left: "onLeft",
  Right: "onRight",
  Capture: "onCapture",
  Minus: "onMinus",
  Sync: "onSync",
  L: "onL",
  ZL: "onZL",
  LeftJoystick: "onLeftJoystickPressed",
  B: "onB",
  A: "onA",
  X: "onX",
  Y: "onY",
  Home: "onHome",
  Plus: "onPlus",
  R: "onR",
  ZR: "onZR",
  RightJoystick: "onRightJoystickPressed",
};

class App extends Component {

  
  constructor(props) {
    super(props);
    this.ws = null;
    this.connect = this.connect.bind(this);
    this.handleAddressChange = this.handleAddressChange.bind(this);
    this.connected = this.connected.bind(this);
    this.onMessage = this.onMessage.bind(this);
    this.onError = this.onError.bind(this);
    this.onClose = this.onClose.bind(this);
  }


  connect() {
    if (this.state.address && !this.ws) {
      try {
        this.ws = new WebSocket(this.state.address);
        this.ws.onopen = this.connected;
        this.ws.onmessage = this.onMessage;
        this.ws.onerror = this.onError;
        this.ws.onclose = this.onClose;
      } catch(exp) {
        console.log(exp);
      }
    } else {

    }
  }

 connected(e) {
  console.log(e);
  this.ws.send({
    "flag": "startListening" // Start sending input data
  });
 }

 onMessage(e) {
  console.log(e);
  const msg = e.data;
  // {
  //   "flag": "button",
  //   "type": "A", // Keyname for the button (eg. A, X, ZL, Plus, Down, LeftJoystick)
  //   "state": true // Whether on or off
  // }
  // {
  //   "flag": "joystick",
  //   "stick": "L", // L for left joystick, R for right joystick
  //   "angle": pi/2, // Angle in radians of the stick movement
  //   "power" 70 // The power of the stick, from 0 to 100
  // }
  if (msg) {
    if (!window.joyconJS) {
        return;
    }
    if(msg.flag === 'button' && msg.type) {
        const func = methodMap[msg.type];
        if (func) {
          window.joyconJS[func](msg.state);
        }
    } else if(msg.flag === 'joystick') {
      let func = 'onLeftJoystick';
      if(msg.stick === 'r') {
        func = 'onRightJoystick';
      }
      window.joyconJS[func](msg.power, msg.angle);
    }
  }
 }

 onError(err) {
  console.error(
      "Socket encountered error: ",
      err.message,
      "Closing socket"
  );

  this.ws.close();
 }

 onClose(e) {
  console.log(e);
  this.ws = null;
 }

  handleAddressChange(e) {
    this.setState({address: e.target.value});
  }

  render() {
    return (
      <div className='App'>
        <div className='App-header'>
          <TextField 
            id="standard-basic"
            label="WS Address"
            className={this.props.classes.root}
            onChange={this.handleAddressChange}
          />
          <Button 
            variant="contained"
            color="primary"
            className={this.props.classes.button}
            onClick={this.connect}
          >
            Connect
          </Button>
        </div>
        <p className='App-intro'>
        </p>
      </div>
    );
  }
}

export default withStyles(styles)(App);
