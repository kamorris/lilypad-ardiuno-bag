#include <EEPROM.h>

/*
 *
 * Blink
 *
 */

#define SWITCH_PIN   4
#define DEBOUNCE     200
#define HOLD_TIME    2000
#define TOTAL_LEDS   15
#define NUMERIC_LEDS 10
#define RANDOM_LEDS  5
#define THRESHOLD    1000
#define PATTERN_ROWS 16
#define PATTERN_STS  10

int currentAction = 0;
int buttonVal = 0;
int buttonLast = HIGH;
long btnDnTime = 0;
long btnUpTime = 0;
int currentRow;
int currentDisplay = 0;
int dancingLEDIndex = 0;
int currentCycle = 0;

int CurrentLEDBehavior = 0;

int assignedLEDNumbers[] = { 
        17, 16, 15, 14, 13, 12, 11, 10, 9, 8 };
int randomLEDs[] = { 
        2, 3, 5, 6, 7};
int allLEDs[] = { 
        7, 6, 5, 3, 2, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8 };
int currentLEDBehaviour = 0;

byte dancingLedStates[TOTAL_LEDS];

//	KNITTING PATTERN ARRAY SETUP
int Pattern[PATTERN_ROWS][PATTERN_STS] = {
        { 
                0, 5, 3, 0, 0, 0, 0, 0, 2, 5                                                                                                           }
        ,
        { 
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                                                                                           }
        ,
        { 
                0, 5, 0, 3, 0, 0, 0, 2, 0, 5                                                                                                           }
        ,
        { 
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                                                                                           }
        ,
        { 
                0, 5, 0, 0, 3, 0, 2, 0, 0, 5                                                                                                           }
        ,
        { 
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                                                                                           }
        ,
        { 
                0, 5, 0, 0, 0, 4, 0, 0, 0, 5                                                                                                           }
        ,
        { 
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                                                                                           }
        ,
        { 
                0, 0, 0, 2, 5, 0, 5, 3, 0, 0                                                                                                           }
        ,
        { 
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                                                                                           }
        ,
        { 
                0, 0, 2, 0, 5, 0, 5, 0, 3, 0                                                                                                           }
        ,
        { 
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                                                                                           }
        ,
        { 
                0, 2, 0, 0, 5, 0, 5, 0, 0, 3                                                                                                           }
        ,
        { 
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                                                                                           }
        ,
        { 
                4, 0, 0, 0, 5, 0, 5, 0, 0, 0                                                                                                           }
        ,
        { 
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                                                                                           }
        ,
};

byte chaser[TOTAL_LEDS] = { 
        0, 7, 31, 127, 255, 127, 31, 7, 0, 0, 0, 0, 0, 0, 0 };

//	STITCH-TO-LED-BEHAVIOR ARRAY SETUP
/*
0 = K k On
 1 = P p Off
 2 = K2tog / fast blink 500, 500 (1000)
 3 = SSK \ slow blink 1000, 1000 (2000)
 4 = Sk2p T 2 slow blinks 1 long blink 1000, 500, 1000, 500, 500, 500 (4000)
 5 = YO o flicker 100, 100 (200)
 Total cycle: 4000 ms, 100 each loop
 */

int Stitches[6][40] = {
        {	
                HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH	                                                                                                          }
        ,
        {	
                LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW	                                                                                                          }
        ,
        {	
                HIGH, HIGH, HIGH, HIGH, HIGH, LOW, LOW, LOW, LOW, LOW, HIGH, HIGH, HIGH, HIGH, HIGH, LOW, LOW, LOW, LOW, LOW, HIGH, HIGH, HIGH, HIGH, HIGH, LOW, LOW, LOW, LOW, LOW, HIGH, HIGH, HIGH, HIGH, HIGH, LOW, LOW, LOW, LOW, LOW	                                                                                                          }
        ,
        {	
                HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW		                                                                                                          }
        ,
        {	
                HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW, LOW, LOW, LOW, LOW, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW, LOW, LOW, LOW, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW	                                                                                                          }
        ,
        {	
                HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW	                                                                                                          }
        ,
};


void setup()
{
//        Serial.begin(9600); 
        randomSeed(analogRead(18));
        for (int i = 0; i < TOTAL_LEDS; ++i) {
                pinMode(allLEDs[i], OUTPUT);
                digitalWrite(randomLEDs[i], LOW);
        }

        pinMode(SWITCH_PIN, INPUT);
        digitalWrite(SWITCH_PIN, HIGH);

        currentRow = EEPROM.read(0);
        if (currentRow < 1 || currentRow > 200) {
                EEPROM.write(0, 1);
                currentRow = 1;
        }
//        Serial.print("current row ");
//        Serial.println(currentRow);
//        Serial.println();

        for (int i = 0; i < TOTAL_LEDS; ++i) 
                dancingLedStates[i] = (byte)1 << int(random(0, 7));
        btnDnTime = btnUpTime = millis();
//        Serial.print(btnDnTime);
//        Serial.println();
        DisplayRowNumber(currentRow);
}

void loop()
{
        currentAction = 0;
        buttonVal = digitalRead(SWITCH_PIN);

        if (buttonVal == LOW && buttonLast == HIGH && (millis() - btnUpTime) > DEBOUNCE) {
                btnDnTime = millis();
//                Serial.print("now the button is down ");
//                Serial.print(btnDnTime);
//                Serial.println();
        }

        if (buttonVal == HIGH && buttonLast == LOW && (millis() - btnDnTime) > DEBOUNCE) {
                currentAction++;
                btnUpTime = millis();
//                Serial.print("now the button is up ");
//                Serial.print(btnUpTime);
//                Serial.println();
        }

        if (btnUpTime - btnDnTime > THRESHOLD)  { 
//                Serial.print("we have a hold event");
//                Serial.println();
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
                standardEvent();
                break;
        }
}


void clickEvent()  {
        // swap between displaying the pattern and displaying something pretty. 

        currentDisplay++;   
//        Serial.print("click event: currentDisplay ");
//        Serial.println(currentDisplay);
        if (currentDisplay == 1)
                for (int i = 0; i < TOTAL_LEDS; ++i) {
                        dancingLedStates[i] = 1 << int(random(0, 7));
                } 
        for (int i = 0; i < TOTAL_LEDS; ++i)
                digitalWrite(allLEDs[i], LOW);
}

void holdEvent() {
        //  printlin("holdEvent() not implemented");
        //  increment the row number  
        currentRow++;
        //       if (currentRow > PATTERN_ROWS) {
        //                currentRow = 1;
        //       }
        EEPROM.write(0, currentRow);
//        Serial.print("hold event: row ");
//        Serial.print(currentRow);
//        Serial.println();
        DisplayRowNumber(currentRow);
}

void standardEvent() {
        //    println("standardEvent() not implemented");
        // dancing!  or pattern, depending on what happens when you click
        switch (currentDisplay) {
        case 0:
                int thisPatternRow;
                thisPatternRow = currentRow % PATTERN_STS;
//                Serial.print("this Pattern row ");
//                Serial.print(thisPatternRow);
//                Serial.print(" current LED behaviour ");
//                Serial.println(CurrentLEDBehavior);
                for (int i = 0; i < PATTERN_STS; ++i) {
                        digitalWrite(assignedLEDNumbers[i], Stitches[Pattern[thisPatternRow-1][i]][CurrentLEDBehavior]);
//                        Serial.print(i);
//                        Serial.print(" Setting LED ");
//                        Serial.print(assignedLEDNumbers[i]);
//                        Serial.print(" pattern ");
//                        Serial.println(Pattern[thisPatternRow-1][i]);
                }
                if (CurrentLEDBehavior > 38)   {	
                        CurrentLEDBehavior = 0;	
                } 
                else {	
                        CurrentLEDBehavior++;	
                }

                delay(100);
                break;
        case 1:
                for (int i = 0; i < TOTAL_LEDS; ++i) {
                        // fade out but not in.
                        setLED(allLEDs[i], dancingLedStates[i]);  
                        dancingLedStates[i] = (dancingLedStates[i] > 0 ) ? dancingLedStates[i] >> currentDisplay : 128;
                }
                delay(100);
                break;
        case 2:
                for (int i = 0; i < TOTAL_LEDS; ++i) {
                        setLED(allLEDs[i], chaser[(i + currentCycle) % TOTAL_LEDS]);
                }
                if (++currentCycle == TOTAL_LEDS)
                        currentCycle = 0;
                delay(100);
                break;
        case 3:
                currentDisplay = 0;
                break;
        }
}


void DisplayRowNumber(int currentRowNumber)
{
        int totalDigits = digitCount(currentRowNumber);
        int currentDigit;
        int digitCalculation = ReverseDigits(currentRowNumber);
        int digitsToPrint[totalDigits];
        int currentLoop;

        for (int i = 0; i < TOTAL_LEDS; ++i) {
                digitalWrite(allLEDs[i], LOW);
        }

        delay(500);
        // could do better with this!  Use the extra 5 LEDs then you can display up to 59 without needing to use the flash technique.	
        if (currentRowNumber > 59)
        {
                for (int i = 0; i < totalDigits; i++)
                {
                        currentDigit = digitCalculation % 10;
                        digitCalculation = digitCalculation / 10;
                        DisplayRowDigit(currentDigit);
                }
        }
        else if (currentRowNumber > 10)
        {
//                Serial.print("Looking for currentRowNumber");
//                Serial.print(currentRowNumber);
//                Serial.println();
                for (int i = 0; i < currentRowNumber / 10; ++i) {
                        digitalWrite(randomLEDs[i], HIGH);
//                        Serial.print(randomLEDs[i]);
//                        if (i < currentRowNumber - 1) Serial.print(", ");
//                        Serial.println();
                }
                digitsToPrint[0] = currentRowNumber % 10;
                DisplayRowDigit(currentRowNumber % 10);
        }
        else  {
                digitsToPrint[0] = currentRowNumber;
                DisplayRowDigit(currentRowNumber);
        }

        for (int i = totalDigits; i > -1; i--)
        {
                currentLoop = digitsToPrint[i];
                if (currentLoop == 0)
                {	
                        currentLoop = 10;	
                }
        }

        for (int i = 0; i < NUMERIC_LEDS; i++)	
                setLED(assignedLEDNumbers[i], LOW);

        for (int i = 0; i < RANDOM_LEDS; ++i)
                setLED(randomLEDs[i], LOW);

        delay(2000);
}

int DisplayRowDigit(int currentDigit)
{
//        Serial.print("Displaying row digit for ");
//        Serial.print(currentDigit);
//        Serial.println();
        for (int i = 0; i < currentDigit;  i++)
        {
                digitalWrite(assignedLEDNumbers[i], HIGH);
//                Serial.print(assignedLEDNumbers[i]);
//                if (i < currentDigit - 1) Serial.print(", ");
        }
//        Serial.println();
        delay(4000);
        for (int i = 0; i < NUMERIC_LEDS; i++)
        {
                setLED(assignedLEDNumbers[i], LOW);
        }
        delay(500);
}

int digitCount(int currentRowNumber)
{
        return floor(log10(currentRowNumber)) + 1;
}

int ReverseDigits(int currentRowNumber)
{
        int ReverseRowNumber = 0;
        while(currentRowNumber>0) {
                ReverseRowNumber += currentRowNumber-((currentRowNumber/10)*10);
                currentRowNumber /= 10;
                if (currentRowNumber>0) {
                        ReverseRowNumber *= 10;
                }
        }
        return ReverseRowNumber;
}

void setLED(int LED, int Brightness)
{
        analogWrite(LED, Brightness);
}















