// Created by Zen Maker Lab Inc.
// Website: zenmakerlab.com

#include "FastLED.h"
#include <inttypes.h>

#define NUM_LEDS 12
#define DATA_PIN 5
#define CLOCK_PIN 6

#define RED_BUTTON 4
#define GREEN_BUTTON 3
#define WHITE_BUTTON 2

CRGB leds[NUM_LEDS];

unsigned long half_seconds_irq = 0;
unsigned int seconds = 0;
unsigned int minutes = 0;
unsigned int hours = 0;

unsigned int minutes_offset = 0;
unsigned int hours_offset = 0;
bool showTime = true;

uint8_t buttonState = 0x0;
uint8_t buttonRead = 0x0;

//CLK FREQ = 16MHz

const uint16_t t1_comp = 31250;
const uint16_t t1_load = 0;

char buf[60];

void setup() {
  noInterrupts();
  pinMode(RED_BUTTON, INPUT_PULLUP);
  pinMode(GREEN_BUTTON, INPUT_PULLUP);
  pinMode(WHITE_BUTTON, INPUT_PULLUP);
  Serial.begin(9600);
  
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);  // BGR ordering is typical
  for (int i=0;i<NUM_LEDS-1;i++){
    leds[i] = CRGB (0,0,0);
    }
    FastLED.show();

  timerOneSetup();
  interrupts();
}

void loop() {
  updateStrip();
  updateTime();
  updateInputs();
}

void updateInputs(){
  buttonRead = 0x0;
  buttonRead |= !digitalRead(RED_BUTTON) << 0;
  buttonRead |= !digitalRead(GREEN_BUTTON) << 1;
  buttonRead |= !digitalRead(WHITE_BUTTON) << 2;

  buttonState |= buttonRead;
  //Serial.print(buttonState);
  //Serial.print(" , ");
  //Serial.println(buttonRead);
  Serial.println(showTime);

  if (!buttonRead){
    if(buttonState & 0x1){
      showTime = !showTime;
      buttonState &= ~0x1;
    }
  if(buttonState & 0x2){
      minutes_offset++;
      if(minutes_offset>59){
        minutes_offset = 0;
        }
      buttonState &= ~0x2;
    }
  if(buttonState & 0x4){
      hours_offset++;
      if(hours_offset>11){
        hours_offset = 0;
        }
      buttonState &= ~0x4;
    }
  }
}

void updateTime(){
  seconds = (half_seconds_irq / 2) % 60;
  minutes = ((half_seconds_irq / 120) + (minutes_offset % 60))%60;
  hours = ((half_seconds_irq / 7200) + (hours_offset %12))%12;
}

void updateStrip(){
  for (int i=0;i<NUM_LEDS;i++){
    leds[i] = CRGB (0,0,0);
  }
  if(showTime){
    int sec_light = seconds % 5;
    leds[seconds/5] = CRGB(0,0,50*(5-sec_light));
    leds[(seconds/5+1)%12] = CRGB(0,0,50*sec_light);
    int min_light = minutes % 5;
    leds[minutes/5] = leds[minutes/5] + CRGB(0,50*(5-min_light),0);
    leds[(minutes/5 + 1)%12] = leds[(minutes/5+1)%12] + CRGB(0,50*min_light,0);
    leds[hours] = leds[hours] + CRGB (255,0,0);
  }
  FastLED.show();
}

void timerOneSetup(){
  TCCR1A = 0;

  //ctc mode
  TCCR1B &= (1<<WGM13);
  TCCR1B |= (1<<WGM12);

  TCCR1B |= (1 << CS12);
  TCCR1B &= ~(1 << CS11);
  TCCR1B &= ~(1 << CS10);

  TCNT1 = t1_load;
  OCR1A = t1_comp;

  TIMSK1 = (1 << OCIE1A);

  sei();
}

ISR(TIMER1_COMPA_vect){
  half_seconds_irq++;
}
