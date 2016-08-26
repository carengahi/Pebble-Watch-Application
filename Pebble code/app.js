//send different request to server according to different message listened from pebble watch
//e.g. if it gets a 'timer', which maps to 2, from the watch, it will send key 2 to the server

Pebble.addEventListener("appmessage", function(e) {
    // sendToServer();
  if (e.payload){
    if (e.payload.Temp) {
        sendToServer(0);
      } else if (e.payload.timer) {
        sendToServer(1);
      } else if (e.payload.Proximity) {
        sendToServer(2);
      } else if (e.payload.Statistics) {
         sendToServer(3);
      } else if (e.payload.Standby) {
         sendToServer(4);
      } else if (e.payload.Action) {
         sendToServer(5);
      } else if (e.payload.Celsius) {
         sendToServer(6);
      } else if (e.payload.Fahrenheit) {
         sendToServer(7);
      } else {
        Pebble.sendAppMessage({ "0": "No field in Payload!" });
      } 
    } else {
      Pebble.sendAppMessage({ "0": "Null Payload!!!" });
    }
  }
);

//send requests to server, get response and send back to pebble watch
function sendToServer(key) {
  var req = new XMLHttpRequest();
  var ipAddress = "10.0.0.47"; // Hard coded IP address
  var port = "3005"; // Same port specified as argument to server
  var url = "http://" + ipAddress + ":" + port + "/" + key;
  var method = "GET";
  
  req.onload = function(e) {
        // see what came back
        var msg = "no response";
        var response = JSON.parse(req.responseText);
        if (response) {
          if (response.Temp) {
            Pebble.sendAppMessage({"0": response.Temp});
          } else if (response.timer) {
              msg = response.timer;
              Pebble.sendAppMessage({ "1": msg});
          } else if (response.Proximity) {
              msg = response.Proximity;
              Pebble.sendAppMessage({ "2": msg});
           } else if (response.Statistics) {
              msg = response.Statistics;
              Pebble.sendAppMessage({ "3": msg});
           } else if (response.Disconnected_from_Arduino) {
               Pebble.sendAppMessage({ "8": response.Disconnected_from_Arduino});
           } else if (response.not_connected_to_server) {
               Pebble.sendAppMessage({ "9": response.Cannot_Connected_Server });
           }
          else {
            Pebble.sendAppMessage({"404": "Unidentified Key"});
          }
        }
        
  };
  
//send server error message when it is shut down
     req.onerror = function(e) {
       Pebble.sendAppMessage({"9" : "This is an error msg"});
     };
      req.open(method, url, true);
      req.send(null);
       
}
