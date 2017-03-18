#include "ArdGPIO.h"
#include "Timestamp.h"

class LightZone
{
public:
  LightZone(ArdGPIO& io, int cmd_pin, int status_pin, int button_pin, int led_pin);

  void Setup();
  bool Update();

  bool StartPendingOff();
  bool TurnOn();

  const char* GetStatusText() const;

private:

  void TurnOff();
  
  ArdGPIO& gpio;
  int cmdPin;
  int statusPin;
  int buttonPin;
  int ledPin;

  static const int LIGHT_ON = 0;
  static const int LIGHT_OFF = 1;

  enum Status_States
  {
    Off,
    On,
    Pending_Off,
    Overriden
  };

  static const char* State_text[];

  Status_States last_status;
  bool dirty_status;
  bool cmd;

  Timestamp pending_start;
  int led_state;

  bool button_pre_status;
  bool button_status;
  Timestamp button_pressed;

  


  static const uint32_t debounce = 100;
  static const uint32_t pending_time = 300000;

  bool UpdateStatus();
  bool ProcessButton();
  void UpdateLED();
  void CheckPendingOff();
};
