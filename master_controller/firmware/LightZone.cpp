#include "LightZone.h"
#include <Arduino.h>

const char* LightZone::State_text[] =
{
	"Off",
	"On",
	"Pending_Off",
	"Overriden"
};

LightZone::LightZone(ArdGPIO& io, int cmd_pin, int status_pin, int button_pin, int led_pin):
	gpio(io),
	cmdPin(cmd_pin),
	statusPin(status_pin),
	buttonPin(button_pin),
	ledPin(led_pin),
  last_status(Off),
  cmd(false),
  pending_start(0),
  button_status(false),
  button_pre_status(false),
	led_state(0),
  dirty_status(false)
{

}

const char* LightZone::GetStatusText() const
{
	return State_text[last_status];
}

void LightZone::Setup()
{

  if (gpio.digitalRead(statusPin))
  {
    last_status = On;
    led_state = 1;
    cmd = true;
    dirty_status = true;
  }
  
  gpio.pinMode(cmdPin, OUTPUT);
  if (cmd) gpio.digitalWrite(cmdPin, HIGH);
	gpio.pinMode(ledPin, OUTPUT);
  gpio.pinMode(statusPin, INPUT);
  gpio.pinMode(buttonPin, INPUT);
	gpio.pullupMode(statusPin, 0);
	gpio.pullupMode(buttonPin, 1);
  
	//if we are here due to a WDT reset we can recover previous status
	
}

bool LightZone::Update()
{
	ProcessButton();
	UpdateLED();
	CheckPendingOff();

	return UpdateStatus();
}

bool LightZone::ProcessButton()
{
	bool button_input = gpio.digitalRead(buttonPin);
  
	if (button_input && !button_pre_status && !button_status)
	{
	  button_pressed.Update();
	  button_pre_status = true;
	}
	else if (button_input && button_pre_status && !button_status)
	{
		if (button_pressed.Elapsed() > debounce)
		{
		  button_status = true;
		  if (last_status == On)
		  {
				StartPendingOff();
		  }
		  else
		  {
				TurnOn();
		  }
		} 
	}
	else if (!button_input)
	{
		button_status = false;
    button_pre_status = false;
	}
}

void LightZone::UpdateLED()
{
	switch(last_status)
	{
	case Off:
		gpio.digitalWrite(ledPin, LIGHT_OFF);
		led_state = 0;
		break;
	case On:
	case Overriden:
		led_state = 1;
		gpio.digitalWrite(ledPin, LIGHT_ON);
		break;
	case Pending_Off:
		//dec led_toggle_time by elapsed time since last update()
		gpio.digitalWrite(ledPin, (millis() >> 7) & 1);

		break;
	}
}

bool LightZone::UpdateStatus()
{
	//TODO this should have a "dirty" concept so that a
	//status change due to a direct command
	//is reported on the next call the UpdateStatus
  bool force_update = dirty_status;
  dirty_status = false;
	bool status_input = gpio.digitalRead(statusPin) == 0 ? false : true;
	Status_States old_status = last_status;
	if (!status_input)
	{
	  last_status = Off;
	}
	else
	{
	  if (!cmd)
	  {
			last_status = Overriden;
	  }
	  else
	  {
      if (last_status != Pending_Off)
		    last_status = On;
	  }
	}

	return ((last_status != old_status) || force_update);
}



bool LightZone::StartPendingOff()
{
  
	if (last_status == On)
	{
    Serial.println("StartPendingOff");
		last_status = Pending_Off;
    dirty_status = true;
		pending_start.Update();
		return true;
	}
	return false;
}

void LightZone::CheckPendingOff()
{
	if (last_status == Pending_Off)
	{
		if (pending_start.Elapsed() > pending_time)
		{
      Serial.println("CheckPendingOff says it time to turn off");
		  TurnOff();
		}
	}
}

bool LightZone::TurnOn()
{
	if (last_status != On)
	{
    last_status = On;
    dirty_status = true;
    Serial.println("Turn on");
		cmd = true;
		led_state = 1;
		gpio.digitalWrite(cmdPin, HIGH);
		gpio.digitalWrite(ledPin, LIGHT_ON);
		return true;
	}
	return false;
}

void LightZone::TurnOff()
{
  Serial.println("Turn off");
	cmd = false;
	led_state = 0;
	gpio.digitalWrite(cmdPin, LOW);
	gpio.digitalWrite(ledPin, LIGHT_OFF);
  
  
}
