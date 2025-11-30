var xmlhttp = new XMLHttpRequest();   // new HttpRequest instance 
var theUrl = "http://10.4.32.10:1880/button";
var payload = {};
payload.button_device = "atom-22";
payload.button = "a";
payload.action="SingleClick";
xmlhttp.open("POST", theUrl);
xmlhttp.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
xmlhttp.send(JSON.stringify(payload));
