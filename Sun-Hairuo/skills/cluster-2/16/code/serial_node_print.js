// Require the serialport node module
var SerialPort = require('serialport');
var Readline = require('@serialport/parser-readline');
// Open the port
var port = new SerialPort('/dev/cu.SLAB_USBtoUART', {
    autoOpen: false,
    baudRate: 115200
});

var parser = port.pipe(new Readline({delimiter: '\n'}));
console.log("test");

port.open(function (err){
  if (err) {
    return console.log('Error opening port:', err.message);
  }
  port.write('main screen turn on')
});

port.on('readable', data=>{
  port.read();
});

parser.on('data',data=>{
  console.log(data);
});






//var app = require('express')();
// var http = require('http').Server(app);
// var io = require('socket.io')(http);

/*
// Require the serialport node module
var serialport = require('serialport');
var SerialPort = serialport.SerialPort;
// Open the port
var port = new SerialPort("/dev/cu.SLAB_USBtoUART", {
    baudrate: 9600,
    parser: serialport.parsers.readline("\n")
});
// Read the port data
port.on("open", function () {
    console.log('open');
    port.on('data', function(data) {
        console.log(data);
    });
});
*/

/*
const SerialPort = require('serialport')
const Readline = require('@serialport/parser-readline')
const port = new SerialPort('/dev/cu.SLAB_USBtoUART', { baudRate: 256000 })

const parser = new Readline()
port.pipe(parser)

parser.on('data', line => console.log(`> ${line}`))
port.write('ROBOT POWER ON\n')
//> ROBOT ONLINE
*/
