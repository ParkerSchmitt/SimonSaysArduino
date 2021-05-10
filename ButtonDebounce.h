#ifndef ButtonDebounce_H
#define ButtonDebounce_H

int buttonPin = 4;

//Various states of the button
enum buttonStates {buttonIdle,buttonWait,buttonLow,buttonHigh};
buttonStates buttonState = buttonIdle;

unsigned long buttonTimer;


void ButtonInitalize() {
  pinMode(buttonPin, INPUT);
  pinMode(13, OUTPUT);
  PORTB &= ~0x20; // write 13 LO;
}

//Function to be used in loop to detect state and action
int ButtonNextState (int Press) {
  switch (buttonState) {
    //If nothing has been happening
    case buttonIdle:
      if(Press == LOW) {
        buttonTimer = millis();
        buttonState = buttonWait;
        PORTB |= 0x20; // write 13 HI;
      }
      break;
      //If it has been low  wait for give miliseconds
    case buttonWait:
      if (Press == HIGH) {
        buttonState = buttonIdle;
        PORTB &= 0x20; // write 13 LO;

      } else {
        if (millis() - buttonTimer >= 5) {
          buttonState = buttonLow;
          PORTB &= 0x20;
          return 1;
        }
      }
      break;
      //If button state has been low for 5 seconds
    case buttonLow:
      if (Press == HIGH) {
        buttonState = buttonIdle;
        return 2;
      }
      break;
  }
  return 0;
}
