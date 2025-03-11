# Battery Callback
This is a battery tool I created for the Rebble Hackathon #002.

It gives you the possibility to (automatically) post your Pebble's battery status to a REST server.

<img src ="BatteryCallback/screenshot_config.png" width= 200 alt = "Battery Callback Phone Config screenshot">

![Battery Callback app screenshot](BatteryCallback/screenshot2.png)
![Battery Callback app screenshot](BatteryCallback/screenshot3.png)

## Features:
v1.0:
- send battery charge state to custom endpoint
- manual and event-driven callback
  
v1.1:
- fix opening via intent (from tasker) not sending

## Ingration with Tasker
### Tasker as server
Don't want to set up your own server for this endpoint? You can just use Tasker on your mobile phone with the HTTP-Request event. 

Set the Method to `POST` and set the endpoint in the Battery Callback watch app settings to `http://localhost:PORT` (PORT being the port you specified in Tasker).

Set the Quick Response to 200.

From there you can create a Task that processes the `%http_request_body`. 

### Request battery state
You can also use Tasker to trigger a battery callback on demand! Using intents you can trigger the app to open on the watch, which will call the endpoint automatically.

1. Create a new task and select the 'Send Intent' action (Under System). 
2. Set Action to: `com.getpebble.action.app.START`
3. Set Extra to: `uuid:bfa0936b-a7ab-433c-8092-a93e934b9944`

## Payload
The payload of the POST-request is JSON-formatted and has the same structure as <a href="https://developer.rebble.io/developer.pebble.com/docs/c/Foundation/Event_Service/BatteryStateService/index.html#BatteryChargeState">BatteryStateService</a>.

Example output:
```
{"charge_percent":30,"is_charging":false,"is_plugged":false}
```

## Debugging endpoint issues
Not adding `http://` before your endpoint URL may result in a timeout.

The timeout time is set to 10 seconds. After that you should see the `Fail! Timeout` message on your Pebble screen.

If there's a status error (e.g `404`) it will show as `Fail! 404`.

Everytime you open the app manually a request is attempted so you can easily test this way. 
