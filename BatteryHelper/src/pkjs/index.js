// clay initialization
var Clay = require('pebble-clay');
var clayConfig = require('./config.json');
// default config
var clay = new Clay(clayConfig, null, {autoHandleEvents: false});
var messageKeys = require('message_keys');
var configOpen = false;

const TIMEOUT_MS = 1000;

var xhrRequest = function (url, type, body, callback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
      console.log("status: " + xhr.status);
      callback(xhr.status);
    };
    xhr.open(type, url);
    //xhr.send();
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.send(JSON.stringify(body));
};
  
// Listen for when the watchapp is opened
Pebble.addEventListener('ready', 
    function(e) {
        console.log('PebbleKit JS ready!');
        Pebble.sendAppMessage({'JSReady': 1}); // notify watch app we're ready to receive
    }   
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    var dict = e.payload;
    console.log('Got message: ' + JSON.stringify(dict));
    if (dict["battery"])
    {
        console.log(dict["battery"]);
        var endpoint = localStorage.getItem("ENDPOINT");
        xhrRequest(endpoint, "POST", dict, function(statusCode) {
            if (statusCode >= 200 && statusCode < 300)
            {
                if (!configOpen) // don't close app if config is open
                {
                    Pebble.sendAppMessage({permissionToCloseApp: 1});
                }
            } else {
                Pebble.sendAppMessage({httpError: statusCode});
            }
        });
    }
  }                     
);

Pebble.addEventListener('showConfiguration', //we need to implement this since we are overriding events in webviewclosed
  function(e) {
    clay.config = clayConfig;
    configOpen = true;
    Pebble.openURL(clay.generateUrl());
});

// Listen for when web view is closed
Pebble.addEventListener('webviewclosed',
    function(e) {
    if (e && !e.response) { return; }
  
    configOpen = false;
    var dict = clay.getSettings(e.response);

    localStorage.setItem("ENDPOINT", dict[messageKeys.endpoint]); // store endpoint

    // handle everything sent as 24h string
    var timeStr = dict[messageKeys.fixedTime]; //can be "2:30 PM" or "14:30" for example
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
        dict[messageKeys.fixedTime] = ("0" + hours).slice(-2) + ":" + ("0" + minutes).slice(-2);
    }

    dict[messageKeys.permissionToCloseApp] = 1; // tell the watch-app its safe to close now

    // Send settings values to watch side
    Pebble.sendAppMessage(dict, function(e) {
        console.log('Sent config data to Pebble');
    }, function(e) {
        console.log('Failed to send config data!');
        console.log(JSON.stringify(e));
    });

    console.log("fixedTime: " + dict[messageKeys.fixedTime]);
  }
);
