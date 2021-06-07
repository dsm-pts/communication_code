#include <SPI.h>
#include "RF24.h"

// Set this radio as radio number 0 or 1
bool radioNumber = 0;
// Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8
RF24 radio(7,8);

byte addresses[][6] = {"1Node","2Node"};

bool role = 0;

void setup() {
  Serial.begin(115200);
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));
  
  radio.begin();

  radio.setPALevel(RF24_PA_LOW);
  
  if (radioNumber) {
    radio.openWritingPipe((uint64_t)addresses[1]);
    radio.openReadingPipe(1,(uint64_t)addresses[0]);
  } else {
    radio.openWritingPipe((uint64_t)addresses[0]);
    radio.openReadingPipe(1,(uint64_t)addresses[1]);
  }
  
  radio.startListening();
}

void loop() {
  
  if (role == 1) {
    radio.stopListening();                               
    Serial.println(F("Now sending"));
    
    int signal = 3;                              
    if (!radio.write( &data, sizeof(int) )) {
      Serial.println(F("failed"));
    }
    radio.startListening();
                                         
    unsigned long started_waiting_at = micros();                
    boolean timeout = false;                                    
    
    while ( ! radio.available() ) {                           
      if (micros() - started_waiting_at > 200000 ) {        
        timeout = true;
        break;
      }      
    }
    
    if ( timeout ) {                                      
      Serial.println(F("time out"));
    } else {
      int got_signal;                                 
      radio.read( &got_signal, sizeof(int) );
      unsigned long time = micros();

      Serial.print(F("Sent "));
      Serial.print(signal);
      Serial.print(F(", Got response "));
      Serial.print(got_signal);
      }

    delay(1000);
  }

  if ( role == 0 ) {
    int got_signal;
    
    if ( radio.available()) {
      
      while (radio.available()) {                                
        radio.read( &got_signal, sizeof(int) );           
      }

      radio.stopListening();                                        

      radio.write( &got_signal, sizeof(int)) );

      radio.startListening();                                            
      Serial.print(F("Sent response "));
      Serial.println(got_signal);  
    }
  }

  // Change Roles via Serial Commands
  if ( Serial.available() ) {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == 0 ) {      
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      role = 1; 
    
    } else {
        if ( c == 'R' && role == 1 ) {
        Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));      
        role = 0;
        radio.startListening();
      }
    }
  }
}
