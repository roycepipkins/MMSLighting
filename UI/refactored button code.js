var csv = msg.payload;
var button_to_channel = {};
var state_to_buttons = {};
csv.forEach((entry) => {
    var button_topic = entry["Button_Device_MQTT_Prefix"] + "/button_"+ entry["Button"];
    
    var button = button_to_channel[button_topic];
    if (button === undefined)
    {
        button = button_to_channel[button_topic] = {};
        button.actions = {};
    }
    
    var action = button.actions[entry["Action"]];
    if (action === undefined)
    {
        action = button.actions[entry["Action"]] = [];
    }
    
    action.push(entry["Channel_Prefix"]);
    
    
    var state_topic = entry["Channel_Prefix"] + "/state";
    var state_entry = state_to_buttons[state_topic];
    if (state_entry === undefined)
    {
        state_entry = state_to_buttons[state_topic] = {};
        state_entry.buttons = {};
		
	}
	
	if (state_entry.buttons[button_topic] === undefined)
	{
		var button_entry = state_entry.buttons[button_topic] = {}
		var button_entry.all_states = [];
	}
	
	state_entry.buttons[button_topic].all_states.push(state_topic);
});




flow.set("button_to_channel", button_to_channel);
flow.set("state_to_button", state_to_button);

return msg;