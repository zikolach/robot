var serialport = require("serialport");
var SerialPort = serialport.SerialPort;
var sockjs = require('sockjs');

var sp = new SerialPort("/dev/ttyACM0", {
  baudrate: 9600,
  // defaults for Arduino serial communication
  dataBits: 8,
  parity: 'none',
  stopBits: 1,
  flowControl: false,
  parser: serialport.parsers.readline("\n")
});

var connections = [];

var buffer = '';

sp.on("open", function () {
  console.log('opened serial port');
  sp.on('data', function(data) {
    for (var ii=0; ii < connections.length; ii++) {
      connections[ii].write(data);
    }
  });
});

var robot = sockjs.createServer();

robot.on('connection', function(conn) {
    connections.push(conn);
    var number = connections.length;
    conn.write("Welcome, User " + number);
    conn.on('data', function(message) {
      sp.write(message, function(err, results) {
        console.log('err> ' + err);
        console.log('results> ' + results);
      });
      for (var ii=0; ii < connections.length; ii++) {
        connections[ii].write("User " + number + " says: " + message);
      }
    });
    conn.on('close', function() {
        for (var ii=0; ii < connections.length; ii++) {
            connections[ii].write("User " + number + " has disconnected");
        }
    });
});

module.exports = robot;
