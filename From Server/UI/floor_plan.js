var version = 3;
var ON_COLOR = "#ffff00";
var OFF_COLOR = "#444444";
var ws = null;
var ws_reconnect_interval = null;

//Respond to WebSocket messages
function onMessage(msg)
{
	console.log(msg);
	if (msg.type == "reload")
	{
		location.reload(true);
	}
	else if (msg.type == "status")
	{
		processStatusMap(msg.status_map);
	}
}

//Respond to click events
function onClick (evt) 
{
	var zone = evt.target.id;
	console.log("Clicked Zone: " + zone);
	var current_color = evt.target.getAttribute("fill");
	var command = {};
	command.version = version;
	command.type = "command"
	command.zone = zone;
	if (current_color == OFF_COLOR)
	{
		command.command = "ON";	
	}
	else
	{
		command.command = "OFF";
	}
	
	ws.sendMessage(command);
}


function valueToStatus(value)
{
  //Infer the meaning of the payload
  if (value === 0) return 0;
  if (value === 1) return 1;
  if (value === '0') return 0;
  if (value === '1') return 1;
  if (typeof value == "string")
  {
	if (value.toLowerCase() == "on") return 1;
	if (value.toLowerCase() == "true") return 1;
	if (value.toLowerCase() == "high") return 1;
  }
  return 0;
}

function processStatusMap(status_map)
{
	for(var key in status_map)
	{
		var elem = document.getElementById(key);
		if (elem != null)
		{
			if (valueToStatus(status_map[key]) == 1) 
			{
				elem.setAttribute("fill", ON_COLOR);
			}
			else
			{
				elem.setAttribute("fill", OFF_COLOR);
			}
		}
	}
}

function createWebSocket(path) {
    var protocolPrefix = (window.location.protocol === 'https:') ? 'wss:' : 'ws:';
    return new WebSocket(protocolPrefix + '//' + location.host + path);
}

function connectWebSocket()
{
	ws = createWebSocket("/ws/lighting");
	ws.onmessage = function(evt)
	{
		onMessage(JSON.parse(evt.data));
	}

	ws.sendMessage = function(msg)
	{
		this.send(JSON.stringify(msg));
	}

	ws.onopen = function(evt)
	{
		if (ws_reconnect_interval != null)
		{
			window.clearInterval(ws_reconnect_interval);
			ws_reconnect_interval = null;
		}
		var status_request = {};
		status_request.type = "status_request";
		status_request.version = version;
		this.sendMessage(status_request);
	}

	ws.onclose = function(evt)
	{
		console.log("The websocket closed!");
		if (ws_reconnect_interval == null)
		{
			ws_reconnect_interval = window.setInterval(connectWebSocket, 5000);
		}
	}
}

function onAllOff(evt)
{
	document.querySelectorAll('.LightSwitch').forEach((currentValue, currentIndex, listObj) =>
	{
		var command = {};
		command.version = version;
		command.type = "command"
		command.zone = currentValue.id;
		command.command = "OFF";
		ws.sendMessage(command);
	});
}

window.addEventListener('DOMContentLoaded', () => {
	document.querySelectorAll('.LightSwitch').forEach((currentValue, currentIndex, listObj) =>
	{
		currentValue.addEventListener('click', onClick);
	});
	
	var allOff = document.getElementById('AllOff');
	allOff.addEventListener('click', onAllOff);
	
	connectWebSocket();
});
