#include "LPD8806.h"
#include "SPI.h"
#include <avr/interrupt.h> 
#include <avr/io.h> 

#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU/(USART_BAUDRATE*16UL)))-1)

// Constants
const int NUMLIGHTS = 8;    // Number of lights per arm
bool connected;

int ledPin[8] = {13,12,11,10,9,8,7,6};

// Light struct
typedef struct {
    int id; // The id must match that of the OF program
    bool pwr; // 0 or 1 from OF int
} Light;

Light Lights[NUMLIGHTS]; // Declare lights array


// Serial communication
char inString[6];
int buffer;
int curLight = 0;
int curPwr = 0;

bool loopCounter = false;

void setup() {
    // Setup all lights and arms
  for(int i=0;i<NUMLIGHTS;i++) {
      pinMode(ledPin[i],OUTPUT);
  }
    for(int i=0;i<NUMLIGHTS;i++) {
w
        Lights[i].id = i;
        Lights[i].pwr = 0;
        for(int a=0;a<NUMARMS;a++) {
            Lights[i].strip[a] = LPD8806(LEDPERARM, dataPins[i+a],   clockPins[i+a]);   
            Lights[i].strip[a].begin();
            Lights[i].strip[a].show();
            testStrip(&Lights[i].strip[a]);
            clearStrip(&Lights[i].strip[a]);
        }
    }
    
    
    
    
    setupSerial();
    connected = false;
    delay(500);
}


void loop() {
    // Just turns a light on and off to viz main loop speed
    if(loopCounter) { digitalWrite(testPin, LOW); }
    else { digitalWrite(testPin, HIGH); }
    loopCounter = !loopCounter;
    
    if(connected) { // This is with a connection
        
        for(int i=0; i<LEDPERARM; i++) {
            for(int l=0;l<NUMLIGHTS;l++) {
                for(int a=0;a<NUMARMS;a++) {
                    LPD8806 * strip = &Lights[l].strip[a];
                    int * pwr = &Lights[l].pwr;
                    strip->setPixelColor(i, getColor(*pwr) );
                }
            }
        }
        
        for(int i=0;i<NUMLIGHTS;i++) {
            for(int a=0;a<NUMARMS;a++) {
                LPD8806 * strip = &Lights[i].strip[a];
                strip->show();
            }
        }
        

            
            
            
        } else { // This is with no connection
            for(int i=0;i<NUMLIGHTS;i++) {
                for( int a=0;a<NUMARMS;a++) {
                   testStrip(&Lights[i].strip[a]);
                }
            }        
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
    
//    if (isDigit(buffer) {
    else {
        Lights[curLight].pwr = buffer;
    }
    UDR0 = buffer;
}


// We get signal
ISR(USART0_RX_vect) { // << What?
    digitalWrite(serialPin, HIGH); // Main Screen turn on
    parseSerial(UDR0); // It's you
    digitalWrite(serialPin, LOW); // How are you gentlemen?
}


void setupSerial() {
    
    cli();
    
    //REGISTRE UBRR0
    //9600 BAUD FOSC=16MHZ
    UBRR0 = 103;
    
    //REGISTRE USCR0C
    //COM ASYNCHRONE
    bitWrite(UCSR0C,UMSEL00,0);
    bitWrite(UCSR0C,UMSEL01,0);
    
    //PARITY NONE
    bitWrite(UCSR0C,UPM01,0);
    bitWrite(UCSR0C,UPM00,0);
    
    //8 DATA BIT
    bitWrite(UCSR0C,UCSZ02,0);
    bitWrite(UCSR0C,UCSZ01,1);
    bitWrite(UCSR0C,UCSZ00,1);
    
    //REGISTRE UCSR0B  
    //RECEIVE & TRANSMITT ENABLE
    bitWrite(UCSR0B,RXEN0,1);
    bitWrite(UCSR0B,TXEN0,1);
    
    //ENABLE RX COMPLETE INTERRUPT
    bitWrite(UCSR0B, RXCIE0,1);
    
    sei();
}






