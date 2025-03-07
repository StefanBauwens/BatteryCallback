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

    // Send settings values to watch side
    Pebble.sendAppMessage(dict, function(e) {
        console.log('Sent config data to Pebble');
    }, function(e) {
        console.log('Failed to send config data!');
        console.log(JSON.stringify(e));
    });

    //console.log("apikey: " + dict[messageKeys.apiKey]);
    //localStorage.setItem("OPENWEATHER_APIKEY", dict[messageKeys.apiKey]);
    //localStorage.setItem("USEFAHRENHEIT", dict[messageKeys.useFahrenheit]);
  }
);
