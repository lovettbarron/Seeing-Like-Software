#include "SPI.h"
#include <avr/interrupt.h> 
#include <avr/io.h> 

// Constants
const int NUMLIGHTS = 8;    // Number of lights per arm
bool connected;

int ledPin[8] = {13,12,11,10,9,8,7,6};

// Light struct
typedef struct {
    int id; // The id must match that of the OF program
    int pin; // What pin on the arduino?
    int pwr; // 0 or 1 from OF int
} Light;

Light Lights[NUMLIGHTS]; // Declare lights array


// Serial communication
char inString[6];
int buffer;
int curLight = 0;
int curPwr = 0;
int inByte = 0;

bool loopCounter = false;

void setup() {
    // Setup all lights and arms
    for(int i=0;i<NUMLIGHTS;i++) {
        Lights[i].id = i;
        Lights[i].pin = ledPin[i];
        Lights[i].pwr = 0;
        pinMode(ledPin[i],OUTPUT);
    }

    setupSerial();
    connected = false;
    delay(500);
}


void setupSerial() {
    Serial.begin(115200);
}

void loop() {
    if (Serial.available() > 0) {
        // read the oldest byte in the serial buffer:
        inByte = Serial.read();
        parseSerial(inByte);
    }
    
    for( int i=0;i<NUMLIGHTS;i++) {
        if(Lights[i].pwr != 0)
            digitalWrite(Lights[i].pin, HIGH);
        else
            digitalWrite(Lights[i].pin, LOW);
        }

}

void parseSerial( unsigned char buffer) {
    connected = true;
    
    // Moves pointer to next light
    if (buffer == ',') {
        curLight++;
    } else
    
    // Last light
    if (buffer == '\n') {
        curLight = 0;
        buffer = 0;
    } else
    
    if (buffer == 'e') {
        connected = false;
    }
    
    else {
        Lights[curLight].pwr = buffer;
    }
}




