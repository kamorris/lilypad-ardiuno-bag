#include <EEPROM.h>

/*
 *
 * Lilypad arduino bag
 *
 *
 * Use this one when you don't want to remember a lace pattern.
 *
 * LED patterns are:
 *
 *  1) chasing - LEDs light up in turn, round and round
 *  2) random  - occasionally LEDs light up then stop (one at a time, sometimes two)
 *  3) dancing - many LEDs light up at once, still random
 *
 */

#define SWITCH_PIN   4
// Button must be held for a while to count (we use debounce to remove key bounce.)
#define DEBOUNCE     200
#define TOTAL_LEDS   15
#define NUMERIC_LEDS 10
#define RANDOM_LEDS  5
#define THRESHOLD    1000
#define DELAYER      500
#define STEPS        4
#define STEP_INCR	 255 / 4

typedef enum display {starting, circling, dancing, singles} display;
int currentAction = 0;
int buttonVal = 0;
int buttonLast = HIGH;
long btnDnTime = 0;
long btnUpTime = 0;
int currentRow;
display currentDisplay;
int dancingLEDIndex = 0;
int singleStep = 0;
int currentCycle = 0;
unsigned long lastUpdate;

int CurrentLEDBehavior = 0;

int allLEDs[] = { 7, 6, 5, 3, 2, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8 };
int currentLEDBehaviour = 0;

byte dancingLedStates[TOTAL_LEDS];
int curLED = TOTAL_LEDS;

byte fades[STEPS] = { 0, 127, 255, 127 };
byte chaser[TOTAL_LEDS] = { 0, 7, 31, 127, 255, 127, 31, 7, 0, 0, 0, 0, 0, 0, 0 };

void setLED(int LED, int Brightness)
{
	analogWrite(LED, Brightness);
}


 void setup()
 {
 	randomSeed(analogRead(18));
 	for (int i = 0; i < TOTAL_LEDS; ++i) {
 		pinMode(allLEDs[i], OUTPUT);
 	}

 	pinMode(SWITCH_PIN, INPUT);
 	digitalWrite(SWITCH_PIN, HIGH);

 	currentRow = EEPROM.read(0);
 	if (currentRow < 1 || currentRow > 200) {
 		EEPROM.write(0, 1);
 		currentRow = 1;
 	}

 	for (int i = 0; i < TOTAL_LEDS; ++i) 
 	{
	 	dancingLedStates[i] = (byte)1 << int(random(0, 7));
	 }
 	btnDnTime = btnUpTime = millis();
 }

 void loop()
 {
 	currentAction = 0;
 	buttonVal = digitalRead(SWITCH_PIN);

 	if (buttonVal == LOW && buttonLast == HIGH && (millis() - btnUpTime) > DEBOUNCE) {
 		btnDnTime = millis();
 	}

 	if (buttonVal == HIGH && buttonLast == LOW && (millis() - btnDnTime) > DEBOUNCE) {
 		currentAction++;
 		btnUpTime = millis();
 	}

 	if (btnUpTime - btnDnTime > THRESHOLD)  { 
 		currentAction++;  
        btnDnTime = btnUpTime;  // so we don't get stuck while nothing is happening.
    }
    performAction(currentAction);
    buttonLast = buttonVal;
}

void performAction(int actionSelection) {
	switch (actionSelection) {
		case 1:
			clickEvent();
			break;
		case  2:
			holdEvent();
			break;
		default:
			standardEvent();	// i.e., zero.
			break;
	}
}


void clickEvent()  {
    // swap between displaying mostly bright and random flashes, chasing etc. 

    switch (currentDisplay)
    {
    	case starting:
    	{
    		currentDisplay = circling;
    		break;
    	}
		case circling:
    	{
    		currentDisplay = dancing;
    		break;
    	}
		case dancing:
    	{
    		currentDisplay = singles;
    		break;
    	}
		case singles:
    	{
    		currentDisplay = circling;
    		break;
    	}

    }

	//        Serial.print("click event: currentDisplay ");
	//        Serial.println(currentDisplay);
	if (currentDisplay == dancing) {
		for (int i = 0; i < TOTAL_LEDS; ++i) {
			dancingLedStates[i] = 1 << int(random(0, 7));
		} 
	}
	for (int i = 0; i < TOTAL_LEDS; ++i) {
		digitalWrite(allLEDs[i], LOW);
	}
}

void holdEvent() {
        //  printlin("holdEvent() not implemented");
        //  increment the row number  
        
}

void standardEvent() {
    // dancing!  or pattern, depending on what happens when you click
    if (millis() - lastUpdate < DELAYER)
    	return;

    lastUpdate = millis();

    switch (currentDisplay) {
    	case dancing:
    	{
	   		for (int i = 0; i < TOTAL_LEDS; ++i) {
	   			/* fade out but not in. */
	   			setLED(allLEDs[i], dancingLedStates[i]);  
	   			dancingLedStates[i] = (dancingLedStates[i] > 0 ) ? dancingLedStates[i] >> currentDisplay : 128;
	   		}
            break;
        }
        case circling:
        {
        	for (int i = 0; i < TOTAL_LEDS; ++i) {
        		setLED(allLEDs[i], chaser[(i + currentCycle) % TOTAL_LEDS]);
        	}
        	if (++currentCycle == TOTAL_LEDS) {
        		currentCycle = 0;
        	}
            break;
        }
        case singles:
        {
        	for (int i = 0; i < TOTAL_LEDS; ++i) {
        		setLED(allLEDs[i], 0);
        	}

        	if (curLED == TOTAL_LEDS) {
        		curLED = int(random(0, TOTAL_LEDS));
        		singleStep = 0;
        		break;
        	}
        	// do this slowly, ramp up and down,
        	setLED(curLED, fades[singleStep++]);

        	if (singleStep == STEPS) 
        	{
        		curLED = TOTAL_LEDS;
        	}
        	break;
        }
        default:
        	break;
    }
}

















