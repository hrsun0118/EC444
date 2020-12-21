var dgram = require('dgram');
var express = require('express');
var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);

// Port and IP
var PORT = 1131;
var HOST = '10.0.0.138';
var server = dgram.createSocket('udp4');  // Create socket

server.on('listening', function () {
          var address = server.address();
          console.log('UDP Server listening on ' + address.address + ":" + address.port);
          }); // Create server

var msg = "n";
// var send_flag = false;
// var startMsg = "n";
// var stopMsg = "n";


// On connection, print out received message
server.on('message', function (message, remote) {
    console.log(remote.address + ':' + remote.port +' - ' + message);

    // Send start/stop through udp to esp32 (original: send "Ok!" acknowledgement)
    console.log("Send start/stop instructions");
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
        res.sendFile(__dirname + '/index.html');
        });

// io.on to get data
//when the server receives clicked message, do this
io.on('connection', socket => {
  socket.on('start', data => {
    msg = "1";
    // send_flag = true;
    console.log(data);
  });
});

io.on('connection', socket => {
  socket.on('stop', data => {
    msg = "0";
    // send_flag = true;
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
server.bind(PORT, HOST);
