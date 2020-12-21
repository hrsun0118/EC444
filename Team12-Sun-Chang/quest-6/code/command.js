//Created by Hairuo Sun & Chen-Yu Chang
var dgram = require('dgram');
var express = require('express');
var app = require('express')();
var path = require('path');
var http = require('http').Server(app);
var io = require('socket.io')(http);

// Port and IP
var PORT = 1131;
var HOST = '10.0.0.138';
var server = dgram.createSocket('udp4');  // Create socket
var sensor_data = [];

server.on('listening', function () {
          var address = server.address();
          console.log('UDP Server listening on ' + address.address + ":" + address.port);
          }); // Create server

var msg = "n";

app.use(
    express.urlencoded({
        extended: true
    })
);
app.use(express.json());


// On connection, print out received message
server.on('message', function (message, remote) {
    console.log(remote.address + ':' + remote.port +' - ' + message);
    // parse the data
    var data_str = message.toString();
    sensor_data = data_str.split(","); // get an array of strings
    // Send start/stop through udp to esp32 (original: send "Ok!" acknowledgement)
    console.log("Sending instructions");
    server.send(msg,remote.port,remote.address,function(error){
      if(error){
        console.log('MEH!');
      }
      else{
        console.log('Sent: Ok!');
      }
    });
    msg = "n";
});

// Points to index.html to serve webpage
app.get('/', function(req, res){
        //console.log("get - data: " + data);
        //console.log("Data sent to client!");
        res.sendFile(path.join(__dirname + '/index.html'));
});

// request data at http://localhost:8080/data or just "/data"
app.get('/data', function(req, res) {
        console.log("get - data: " + sensor_data);
        console.log("Data sent to client!");
        res.send(sensor_data);
});

// io.on to get data
//when the server receives clicked message, do this
io.on('connection', socket => {
  socket.on('drop', data => {
    msg = "d";
    console.log(data);
  });
});


http.listen(1132, function() {
            console.log('running on :1132');
            });
// Bind server to port and IP

io.on('connection', function(socket){

      socket.on('disconnect', function(){
                });
      });

server.bind(PORT,HOST);
