#include "LiquidCrystal.h"
#include "ButtonDebounce.h"
#include <EEPROM.h>


//These are all pins that don't conflict with the LCD Display (i.e D5,D6,D7,D8)
#define ButtonBlue 2 //D2
#define LEDBlue 3 //D3
#define ButtonGreen 10 // D10
#define LEDGreen 9 // D9
#define ButtonYellow 13 // D13
#define LEDYellow 11 // D11
#define ButtonRed 4 // D4
#define LEDRed 12 // D12

//Is only used to start the game
#define ButtonStart 4 // D4


//How long they have to input a sequence
#define TIME_LIMIT 5000 // (1000ms is one second)

//Time to have LEDS toggle
#define TIME_OFF 500

#define eeAddress 0 //Location of where to store the highscore in eeprom

int moves[32] = {0}; // An array which will be used to store the moves
int maxMoves = 0; /* The current maximum amount of moves in the array. Since the array is technically filled up with zeroes,
                    we don't know what the maximum "set" value is just by looking at the array. We have to have a varialbe to let us know */
int currentMoves = 0; /* The current play that the player is at. Like above, without a variable we wouldn't know how far the user is into the array. There could be 23 out of 32 steps, and the player could only be on step 3 so far.
                         or the current moves and max moves could be the same as in the player has reached the current max and now they are adding new moves to the array. */
int highestMoves = 0;
//The various gameStates
enum gameStates  {stateStart, statePlay, stateError}
currentState = stateStart; // The current state of the game

//The timer we use to see if a player has inputed a value im time/when things should turn off
unsigned long Timer;

//unsigned long DisplayStateTimer = 0;

//Holds the previously pressed button number
int debounceHold;
//Timer used to see how long it has been since a button was pressed
unsigned long debounceTimer = 0;


LiquidCrystal LcdDriver(A5, A4, 5, 6, 7, 8); // The LCD display that we will use to output the current max moves (score)


//Method to see which of the four buttons were pressed on the circuit given buttons numbered 0-3
int debounce(int buttonInput) {

   //By default the movesToButton method sends -1s if no button was pressed. If it is -1 just ignore.
  if (buttonInput != -1) {
    if (buttonInput != debounceHold) { //If the button is a different one than what we previously though
      debounceTimer = millis(); //Reset the timer
      debounceHold = buttonInput; //Reset our button input to the new input
    } else if (millis() - debounceTimer >= 100) { //If we have been pressing it for 100ms
      debounceHold = -1;
      return buttonInput; //Return the input
    }
  }
  return -1;
}

//Method to convert leds numbered 0-3 to the actual header assigned pins for the buttons.
int movesToLED(int movesNum) {
  switch (movesNum) {
    case 0:
      return LEDBlue;
      break;
    case 1:
      return LEDGreen;
      break;
    case 2:
      return LEDYellow;
      break;
    case 3:
      return LEDRed;
      break;
  }
}

///Method to convert buttons pressed to numbers 0-3, or -1 if no buttons were pressed.
int movesToButton() {
  if (digitalRead(ButtonBlue) == LOW)
  {
    return 0;
  }
  else if (digitalRead(ButtonGreen) == LOW) {
    return 1;
  }
  else if (digitalRead(ButtonYellow) == LOW) {
    return 2;
  }
  else if (digitalRead(ButtonRed) == LOW) {
    return 3;
  } else {
    //Since this function is called repetedly even if there is no button press just return a -1 to signial there is no button press
    return -1;
  }
}




//Called once the program
void setup() {
    //Read in value from eeprom- this is our saved hire score
    EEPROM.get(eeAddress, highestMoves);

  //16 wide 2 tall
  LcdDriver.begin(16, 2);
  LcdDriver.clear();
  //At start just display the default statistics
  LcdDriver.setCursor(0,0);
  LcdDriver.print("Max:" + String(highestMoves));
  LcdDriver.setCursor(0,1);
  LcdDriver.print("Current:" + String(maxMoves));

  //Initalize ButtonDebounce
  ButtonInitalize();
  Serial.begin(9600);

  //Set all the pins used by the LEDS as outputs
  pinMode(LEDBlue, OUTPUT);
  pinMode(LEDYellow, OUTPUT);
  pinMode(LEDGreen, OUTPUT);
  pinMode(LEDRed, OUTPUT);
 
  //Set all the pins used by the buttons as inputs
  pinMode(ButtonRed, INPUT);
  pinMode(ButtonYellow, INPUT);
  pinMode(ButtonGreen, INPUT);
  pinMode(ButtonBlue, INPUT);

}


void loop() {

  // we are starting in the start state
  switch (currentState) {
    case stateStart:
      //Turn off all of our LEDS by default
      digitalWrite(LEDBlue, LOW);
      digitalWrite(LEDRed, LOW);
      digitalWrite(LEDGreen, LOW);
      digitalWrite(LEDYellow, LOW);
      //If the big button on the arduino was pressed.
      if (ButtonNextState(digitalRead(4)) == 2) {
        //Serial.println("Button 4 was pressed");

        //Set a random seed so the game is different every time you play it
        randomSeed(micros());
        //Change our state to statePlay
        currentState = statePlay;
        Timer = millis();
      }
      break;
    //The main state of our game. Handles displaying the current sequence, checking for input, and seeing if it is right or wrong or if the user times out.
    case statePlay:

      //Set the next index in the array to a random number between 0-3
      moves[maxMoves] = random(0, 4);
      //Loop through all the total moves in the array and flash the LED
      for (int i = 0; i <= maxMoves; i++)
      {
        //FLash the LED
        delay(TIME_OFF);
        digitalWrite(movesToLED(moves[i]), HIGH);
        delay(TIME_OFF);

        digitalWrite(movesToLED(moves[i]), LOW);
      }
      //Reset the timer so the user has a total of 5 seconds to press a button or they will time out.
      Timer = millis();
     
      //Loop through all the total moves in the array and check for input
      for (int i = 0; i <= maxMoves; i++)
      {

        while (millis() - Timer < TIME_LIMIT)
        {
            //Returns an integer corresponding to 0-3 if a button is pressed
            int buttonPress = debounce(movesToButton());

             //If the right ubutton was pressed
            if (buttonPress == moves[i]) {

              //Let the user to know that their input was taken by quickly flashing the button they pressed
              digitalWrite(movesToLED(buttonPress),HIGH);
              delay(1000);
              digitalWrite(movesToLED(buttonPress),LOW);

              //Reset the timer so they still have 5 seconds to press the next one
              Timer = millis();
              //We are onto the next move so increment currentMoves
              currentMoves++;
            break;
            //If they press the wrong button
          } else if (buttonPress != moves[i] && buttonPress != -1) {

            //Let the user know their input was taken by quickly flashing the button they pressed
            digitalWrite(movesToLED(buttonPress),HIGH);
            delay(1000);
            digitalWrite(movesToLED(buttonPress),LOW);
            //They are wrong so just reset them to stateError
            currentState = stateError;
            maxMoves = 0;
            break;

            }
        }
      }
      //Increment the pointer (maxMoves) of the array so we can assign the next light in the sequence
      maxMoves++;
      //If the user times out send them to stateError
      if (currentMoves != maxMoves) {
        currentState = stateError;
        currentMoves=0;
        maxMoves=0;
      } else {
        currentMoves = 0;
      }

      //Update the scores on the LCD
      highestMoves = max(maxMoves,highestMoves);
      LcdDriver.clear();
      LcdDriver.setCursor(0,0);
      LcdDriver.print("Max:" + String(highestMoves));
      LcdDriver.setCursor(0,1);
      LcdDriver.print("Current:" + String(maxMoves));
      EEPROM.put(eeAddress, highestMoves);

      break;

    //State plays if the user inputs the wrong button or they take to long to input the sequence- lets the user know the game is over by flashing LEDS
    case stateError:
      //Reset the timer to the current time so we can reset to stateStrt after the TIME_LIMIT time
      Timer = millis();

      //For five seconds randomly flash LEDS
      while (millis() - Timer < TIME_LIMIT) {

      //Flash random leds to make an interesting pattern
      digitalWrite(movesToLED(random(0,4)), HIGH);
        delay(10);
        digitalWrite(movesToLED(random(0,4)), LOW);
      }
      //Reset the variables and go back to the starting state so the user can play again if they want to.
      currentState = stateStart;
      maxMoves = 0;
      currentMoves=0;
     
      break;
 
  }

}
