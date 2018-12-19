#ifndef BUTTON_DEBOUNCER
#define BUTTON_DEBOUNCER
#include <inttypes.h>
#include "Timestamp.h"

class DBButton
{
public:
    DBButton(uint8_t pin, uint8_t debounce_time, bool invert = true, bool one_shot_mode = false);
    bool isPressed();
protected:
    uint8_t button_pin;
    uint8_t db_time;
    Timestamp timer;
    bool last_state;
    bool invert_button;
    bool triggered;
    bool one_shot;
};

#endif