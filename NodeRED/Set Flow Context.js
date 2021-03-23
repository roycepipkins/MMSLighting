var count = msg.payload.length;
var topic_map = msg.payload;
var topic_to_zone = {};
var zone_to_topic = {};
//One loop thru for Web_ID<->Topic maps
for(i = 0; i < count; ++i)
{
    if ("Web_ID" in topic_map[i])
    {
        topic_to_zone[topic_map[i]["MQTT_Topic_Prefix"]+"/status"] = topic_map[i]["Web_ID"];
        zone_to_topic[topic_map[i]["Web_ID"]] = topic_map[i]["MQTT_Topic_Prefix"] + "/command";    
    }
}
//One loop thru for Lux Sensor mapping
var lux_setup_by_tracked_status_topic = {};
var lux_setup_by_lux_sensor = {};
for(i = 0; i < count; ++i)
{
    if ("Lux_Sensor_Topic" in topic_map[i])
    {
        var lux_sensor_topic = topic_map[i]["Lux_Sensor_Topic"];
		var first_tracked_web_id = topic_map[i]["First_Tracked_Web_ID"];
		var second_tracked_web_id = topic_map[i]["Second_Tracked_Web_ID"];
		var first_tracked_topic =  "<none>";
		if (typeof first_tracked_web_id == "string" && first_tracked_web_id.length > 0) 
		{
			first_tracked_topic = zone_to_topic[first_tracked_web_id];
			if (typeof first_tracked_topic == "string") 
			   first_tracked_topic = 
			      first_tracked_topic.replace("command$", "status");
		}
		var second_tracked_topic = "<none>";
		if (typeof second_tracked_web_id == "string" && second_tracked_web_id.length > 0) 
		{
			second_tracked_topic = zone_to_topic[second_tracked_web_id];
			if (typeof second_tracked_topic == "string") 
			   second_tracked_topic = 
			      second_tracked_topic.replace("command$", "status");
		}
		
		var lux_setup = {};
		lux_setup["lux_sensor_topic"] = lux_sensor_topic;
		lux_setup["first_tracked_web_id"] = first_tracked_web_id;
		lux_setup["second_tracked_web_id"] = second_tracked_web_id;
		lux_setup["first_tracked_topic"] = first_tracked_topic;
		lux_setup["second_tracked_topic"] = second_tracked_topic;
		
		lux_setup_by_tracked_status_topic[first_tracked_topic] = lux_setup;
		lux_setup_by_lux_sensor[lux_sensor_topic] = lux_setup;
    }
}

flow.set("topic_to_zone", topic_to_zone);
flow.set("zone_to_topic", zone_to_topic);
flow.set("lux_setup_by_tracked_status_topic", lux_setup_by_tracked_status_topic);
flow.set("lux_setup_by_lux_sensor", lux_setup_by_lux_sensor);
flow.set("version", 3);

