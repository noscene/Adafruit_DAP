#include "DopplerDAP.h"
//
//  This is a doppler Cloud, self cloning patch
//  Connect Pins 3,4,5 To SWD on next Doppler and so on
//  3 -> SWDIO
//  4 -> SWCLK
//  5 -> SWRST 
//

DopplerDAP dap;
#include <ICEClass.h>

ICEClass ice40;

uint16_t led4x4=0;;

const char * d = __DATE__;
const char * t = __TIME__;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  delay(100);
  // put your setup code here, to run once:
  ice40.upload(); // Upload BitStream Firmware to FPGA -> see variant.h
  delay(100);
  // start SPI runtime Link to FPGA
  ice40.initSPI();
  ice40.sendSPI16(led4x4++ );
  for(int i = 0 ; i< 10 ; i++){
      ice40.sendSPI16(0xff00);
      delay(50); 
      ice40.sendSPI16(0x00ff);
      delay(50);   
  }

  Serial.begin(9600);
  delay(500);
  
  /*
  while(!Serial) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200); 
      digitalWrite(LED_BUILTIN, LOW);
      delay(10);  
      ice40.sendSPI16(led4x4++ );
  }*/

  if(dap.connect()) { // Blink 10 Times the Remote LED its like using the SAMD as PortExpander
    for(int i = 0 ; i< 10 ; i++){
      dap.LED(true);
      ice40.sendSPI16(0x1);
      delay(100);
      ice40.sendSPI16(0x2);
      dap.LED(false);
      delay(100);
    }
    dap.CPUID();  // DUMP CPU ID / serial number into debug
    dap.setFuseBootloader16KB();  
    dap.disconnect();
  
    ice40.sendSPI16(0x5555);
    dap.connect();
    dap.disableBootLoaderProtection();
    dap.clone();  // Clone full CPU to next SAMD51 in Chain
    dap.disconnect();
  }
  
  Serial.print(dap.getDebug());
}

void loop() {
  // put your main code here, to run repeatedly:
  ice40.sendSPI16(0xf0f0);
  delay(250); 
  ice40.sendSPI16(0x0f0f);
  delay(250);  
}
