// clay initialization
var Clay = require('pebble-clay');
var clayConfig = require('./config.json');
// default config
var clay = new Clay(clayConfig, null, {autoHandleEvents: false});
var messageKeys = require('message_keys');

const TIMEOUT_MS = 1000;

var xhrRequest = function (url, type, callback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
      callback(this.responseText);
    };
    xhr.open(type, url);
    xhr.send();
};
  
// Listen for when the watchapp is opened
Pebble.addEventListener('ready', 
    function(e) {
        console.log('PebbleKit JS ready!');
    }   
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
  }                     
);

Pebble.addEventListener('showConfiguration', //we need to implement this since we are overriding events in webviewclosed
  function(e) {
    clay.config = clayConfig;
    Pebble.openURL(clay.generateUrl());
});

// Listen for when web view is closed
Pebble.addEventListener('webviewclosed',
    function(e) {
    if (e && !e.response) { return; }
  
    var dict = clay.getSettings(e.response);

    // handle everything sent as 24h string
    var timeStr = settings[messageKeys.fixedTime]; //can be "2:30 PM" or "14:30" for example
    var match = timeStr.match(/(\d+):(\d+) ?(AM|PM)?/i);

    if (match) {
        var hours = parseInt(match[1], 10);
        var minutes = parseInt(match[2], 10);
        var period = match[3] ? match[3].toUpperCase() : null;

        if (period === "PM" && hours !== 12) {
            hours += 12;
        } else if (period === "AM" && hours === 12) {
            hours = 0; // Midnight case
        }

        // Format as HH:MM (always 24-hour format)
        settings[timeKey] = ("0" + hours).slice(-2) + ":" + ("0" + minutes).slice(-2);
    }

    // Send settings values to watch side
    Pebble.sendAppMessage(dict, function(e) {
        console.log('Sent config data to Pebble');
    }, function(e) {
        console.log('Failed to send config data!');
        console.log(JSON.stringify(e));
    });

    console.log("fixedTime: " + dict[messageKeys.fixedTime]);
    //localStorage.setItem("OPENWEATHER_APIKEY", dict[messageKeys.apiKey]);
    //localStorage.setItem("USEFAHRENHEIT", dict[messageKeys.useFahrenheit]);
  }
);
