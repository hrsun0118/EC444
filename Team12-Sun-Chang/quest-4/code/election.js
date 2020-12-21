// Written by Hairuo Sun & Chen-Yu Chang
////////////////////////////////////////////////////////////////////////////////
var dgram = require('dgram');
var PORT = 1131;
var HOST = '10.0.0.138';
var server = dgram.createSocket('udp4');  // Create socket
var express = require('express');
var app = require('express')();
var mongo = require('mongodb');
var http = require('http').Server(app);
var io = require('socket.io')(http);
var MongoClient = mongo.MongoClient;
var url = "mongodb://localhost:27017/mydb";
var data = [];  // Array to hold incoming sensors' data
//var reset_signal = "false";
var reset_string= "0";

app.use( express.urlencoded({ 
    extended: true 
    }) 
);
app.use(express.json());
// Points to index.html to serve webpage
app.get('/', function(req, res){
  res.sendFile(__dirname + '/election.html');
});

server.on('listening', function () {
          var address = server.address();
          console.log('UDP Server listening on ' + address.address + ":" + address.port);
});

server.on('message', function (message, remote) {
        console.log(remote.address + ':' + remote.port +' - ' + message);
        // parse the data
        var data_str = message.toString();
        data = data_str.split(" "); // get an array of strings
        data[2] = formattedTime.toString();
        console.log("reset_signal to be sent (Check TRUE/FALSE):" + reset_string);
        // Send reset_signal throught udp to esp32 (original: send "Ok!" acknowledgement)
          if (reset_signal=="true"){
          reset_string= "reset 1";
          }
          else{
          reset_string = "reset 0";
          }
        server.send(reset_string,remote.port,remote.address,function(error){
            if(error){
                console.log('MEH!');
            }
            else{
                console.log('Sent: Ok!');
            }
        });
});

// For: store data to database
MongoClient.connect(url, function(err, db) {
            if (err) throw err;
            var dbo = db.db("mydb");
            dbo.collection("votes").insertMany(data, function(err, res) {
            if (err) throw err;
            console.log("Number of documents inserted: " + res.insertedCount);
            // console.log(res.ops);
            db.close();
    });
});

// For: obtain data from database
MongoClient.connect(url, function(err, db) {
            if (err) throw err;
            var dbo = db.db("mydb");
            dbo.collection("votes").find({}, { projection: { id:1, candidate:1, time:1 } }).toArray(function(err, result) {
            if (err) throw err;
            console.log("Retreive all data in the database:")
            console.log(result);
            db.close();
        });
});

// When a new client connects
var clientConnected = 0; // this is just to ensure no new data is recorded during streaming
io.on('connection', function(socket){
      console.log('a user connected');
      clientConnected = 0;

      // Call function to stream database data
      clientConnected = 1;
      socket.on('disconnect', function(){
      console.log('user disconnected');
    });
});

/*
// Function to stream from database
function readDB(arg) {
  db.createReadStream()
    .on('data', function (data) {
      console.log(data.key, '=', data.value)
      // Parsed the data into a structure but don't have to ...
      var dataIn = {[data.key]: data.value};
      // Stream data to client
      io.emit('message', dataIn);
    })
    .on('error', function (err) {
      console.log('Oh my!', err)
    })
    .on('close', function () {
      console.log('Stream closed')
    })
    .on('end', function () {
      console.log('Stream ended')
    })
}
*/

// Write random information
function intervalFunc() {
  if (clientConnected == 1) {
      var currentTimeInSeconds=Math.floor(Date.now()/1000); //unix timestamp in seconds
      var date = new Date(currentTimeInSeconds * 1000);
      // Hours part from the timestamp
      var hours = date.getHours();
      // Minutes part from the timestamp
      var minutes = "0" + date.getMinutes();
      // Seconds part from the timestamp
      var seconds = "0" + date.getSeconds();
      // Will display time in 10:30:23 format
      var formattedTime = hours + ':' + minutes.substr(-2) + ':' + seconds.substr(-2);
      // Fill in data structure
      //var value = [{id: 1, temp: getRndInteger(30,80)}];
      // Put structure into database based on key == date, and value
      /*
      db.put([date], value, function (err) {
      if (err) return console.log('Ooops!', err) // some kind of I/O error
      })*/
      // Parse data to send to client
      //var msg = {[date]: value};
      // Send to client
      io.emit('message', data);
      // Log to console
      console.log(Object.keys(data));
    }
}
// Do every 1500 ms
setInterval(intervalFunc, 1500);

app.post('/reset', function(req, res) {
         res.send("ok"); // send "ok" acknowledgement to client "sensor_ajax.html"
         reset_signal = req.body.reset;
});

server.bind(PORT, HOST);  // Bind server to port and IP

app.listen(1132);   // port to listen to
