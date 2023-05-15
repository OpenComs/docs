#include <FastLED.h>

#define CONF_CALL_BLINK_DURATION 1500
#define CONF_CALL_BLINK_SPEED 100

#define CONF_CALL_VIBRATE_AFTER 5000
#define CONF_CALL_BUZZ_AFTER 10000

#define CONF_TALK_LATCH_THRESHOLD 250

#define PIN_BUZZER 2
#define PIN_MOTOR 3
#define PIN_RMK 4
#define PIN_MUTE 5
#define PIN_CALL_SEND 6
#define PIN_CALL_RECV 7
#define PIN_BUTTON_TALK 8
#define PIN_BUTTON_CALL 9
#define PIN_LEDS 10

#define LED_COUNT 16
CRGB leds[LED_COUNT];

uint64_t call_first_received = 0;
uint64_t last_call = 0;

uint64_t talk_first_pressed = 0;
bool talking = false;

void setup() {
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_MOTOR, OUTPUT);
  pinMode(PIN_RMK, INPUT);
  pinMode(PIN_MUTE, OUTPUT);
  pinMode(PIN_CALL_SEND, OUTPUT);
  pinMode(PIN_CALL_RECV, INPUT);
  pinMode(PIN_BUTTON_TALK, INPUT_PULLUP);
  pinMode(PIN_BUTTON_CALL, INPUT_PULLUP);

	FastLED.addLeds<WS2812,PIN_LEDS,GRB>(leds,LED_COUNT);
	FastLED.setBrightness(127);
}


void loop() {
    fill_solid(leds, LED_COUNT, CRGB::Black);

    uint8_t value = beatsin8(10)/18;
    //leds[14] = CRGB(15, 4, 0);
    leds[8] = CRGB(15, 4, 0);
    //leds[14].subtractFromRGB(value);  
    leds[8].subtractFromRGB(value);  

    //}
    // Talk button latch mechanism.
    // Short press to toggle the state, long press does either push-to-talk or push-to-mute

    // Check if the button is pressed
    if(!digitalRead(PIN_BUTTON_TALK)) {
      // If pressed for the first time in a while
      if(talk_first_pressed == 0) {
        // Invert the talking state
        talking = !talking;
        // Store the time of the press
        talk_first_pressed = millis();
      }
    } else {
      // If was pressed and not yet cleared
      if(talk_first_pressed != 0) {
        
        // If the press was more than 250ms ago,
        // Invert the state
        if((millis() - talk_first_pressed) > CONF_TALK_LATCH_THRESHOLD) {
          talking = !talking;
        }

        // Clear the press
        talk_first_pressed = 0;
        delay(25);
      }
    }

    // If we are talking, set the led color and write unmute the analog part
    if(talking) fill_solid(leds, LED_COUNT, CRGB::Green);
    digitalWrite(PIN_MUTE, talking);

    // If the call button is pressed pass that signal through the bus
    digitalWrite(PIN_CALL_SEND, !digitalRead(PIN_BUTTON_CALL));

    // If the call signal is received
    if(!digitalRead(PIN_CALL_RECV)) { 
      // Start a timer 
      last_call = millis(); 
      // Set when the call signal was first received
      if(call_first_received==0) call_first_received=millis(); 
      // If the call signal has been present for more than CONF_CALL_BUZZ_AFTER, start the buzzer
      if((millis()-call_first_received) > CONF_CALL_BUZZ_AFTER) tone(PIN_BUZZER, beatsin8(255)*2+800, 10);
      if((millis()-call_first_received) > CONF_CALL_VIBRATE_AFTER) digitalWrite(PIN_MOTOR, beatsin8(150)<100);
      
    } else { 
      if(call_first_received!=0) call_first_received=0;
      // Stop the buzzer
      noTone(PIN_BUZZER);
      digitalWrite(PIN_MOTOR, false);
    }
    // Blink the leds red
    if(last_call != 0 && (millis() - last_call) < CONF_CALL_BLINK_DURATION) {
        uint8_t valueR = ease8InOutCubic(beatsin8(255));
        uint8_t valueG = (talking ? 255-valueR : 0) / 10;
        fill_solid(leds, LED_COUNT, CRGB(valueR, valueG, 0));
    }

    
	  FastLED.show(); 
}
