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

class App extends Component {

  render() {
    return (
      <div className='App'>
        <div className='App-header'>
          <TextField id="standard-basic" label="WS Address" className={this.props.classes.root}/>
          <Button variant="contained" color="primary" className={this.props.classes.button}>
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
