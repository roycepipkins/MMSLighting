#include "DBButton.h"
#include "Arduino.h"

DBButton::DBButton(uint8_t pin, uint8_t debounce_time, bool invert, bool one_shot_mode):
button_pin(pin),
db_time(debounce_time),
invert_button(invert),
last_state(false),
triggered(false),
one_shot(one_shot_mode)
{
    pinMode(pin, INPUT_PULLUP);
}

bool DBButton::isPressed()
{
    bool current_state = digitalRead(button_pin) == 0 ? invert_button : !invert_button;
    if (current_state)
    {
        if (last_state)
        {
            if (timer.Elapsed() >= db_time)
            {
                if (one_shot)
                {
                    if (triggered) return false;
                    else 
                    {
                        triggered = true;
                        return true;
                    }
                }
            }
            else
            {
                return false;
            }
            
        }
        else
        {
            timer.Update();
            last_state = true;
            triggered = false;
            return false;
        }
    }
    else
    {
        last_state = false;
        triggered = false;
        return false;
    }
}