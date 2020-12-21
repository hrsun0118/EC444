// Written by Hairuo Sun & Chen-Yu Chang
////////////////////////////////////////////////////////////////////////////////

var express = require('express');
var app = express();
var path = require('path');
var fs = require('fs');
var csv = require("csv-parse");

// Variables' declaration for UDP Client
var dgram = require('dgram');
var PORT = 1131;
var HOST = '10.0.0.138';
var server = dgram.createSocket('udp4');  // Create socket
var data = [];  // Array to hold incoming sensors' data
var led_signal = "false";

app.use(
  express.urlencoded({
    extended: true
  })
);
app.use(express.json());

// viewed at http://10.0.0.138:1132
app.get('/', function(req, res) {
  res.sendFile(path.join(__dirname + '/sensor_ajax.html'));
});

// request data at http://localhost:8080/data or just "/data"
app.get('/data', function(req, res) {
  console.log("get - data: " + data);
  console.log("Data sent to client!");
  res.send(data);
});

app.post('/led', function(req, res) {
  res.send("ok"); // send "ok" acknowledgement to client "sensor_ajax.html"
  led_signal = req.body.led;
});


// For UDP getting data
// Create server
server.on('listening', function () {
    var address = server.address();
    console.log('UDP Server listening on ' + address.address + ":" + address.port);
});

// On connection, print out received message
server.on('message', function (message, remote) {
    console.log(remote.address + ':' + remote.port +' - ' + message);

    // parse the data
    var data_str = message.toString();
    data = data_str.split(","); // get an array of strings
    console.log("led_signal to be sent (Check TRUE/FALSE):" + led_signal);
    // Send led_signal throught udp to esp32 (original: send "Ok!" acknowledgement)
    server.send(led_signal,remote.port,remote.address,function(error){
      if(error){
        console.log('MEH!');
      }
      else{
        console.log('Sent: Ok!');
      }
    });
});

server.bind(PORT, HOST);  // Bind server to port and IP

app.listen(1132);   // port to listen to
