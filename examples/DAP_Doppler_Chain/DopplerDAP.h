
#ifndef dopplerDAP_H
#define dopplerDAP_H

#define BUFSIZE SAMx5_PAGE_SIZE  //don't change!

#define SWDIO 3
#define SWCLK 4
#define SWRST 5
// Achtung need redefine !!!!
#define SAMD51_REG_NVMCTRL_CTRLB          (0x41004004)
#define SAMD51_REG_PORT_DIR0              (0x41008000)
#define SAMD51_REG_PORT_OUT0              (0x41008010) 
#define DOPPLER_LED                23 // PA23

#include "Adafruit_DAP.h"

static String debugMsg;

class DopplerDAP : Adafruit_DAP_SAMx5 {

  public:
  char format[16];
      
  DopplerDAP(){
  }

  String  getDebug(){
    return debugMsg;
  }

  char debuggername[100];
  uint32_t flashSize    = 0;

  bool flashBootloader(){
    if (! dap_disconnect())
      dbg(error_message);
    
  }


  // Function called when there's an SWD error
  static void dbg(const char *text) {
    debugMsg+=text;
    debugMsg+="\r\n";
  }


  bool connect(){
    begin(SWCLK, SWDIO, SWRST, &dbg);
    debugMsg+="\r\nConnecting...\r\n";
    if (! dap_disconnect()) 
      debugMsg+=error_message;
  
    if (! dap_get_debugger_info(debuggername))   
      debugMsg+=error_message;

    debugMsg+=debuggername;
  
    if (! dap_connect())   {
        debugMsg+=error_message;
        return false;
    }

    if (! dap_transfer_configure(0, 128, 128))  {
        debugMsg+=error_message;
        return false;
    }
    
    if (! dap_swd_configure(0)) {
        debugMsg+=error_message;
        return false;
    }
    if (! dap_reset_link()) {
        debugMsg+=error_message;
        return false;
    }
    if (! dap_swj_clock(50))   {  
        debugMsg+=error_message;
        return false;
    }

    dap_target_prepare();

    uint32_t dsu_did;
    if (! select(&dsu_did)) {
      debugMsg+="\r\nUnknown device found 0x";
      debugMsg+=dsu_did;  
      // Serial.print(dsu_did, HEX);
      return false;
    }    

    for (device_t *device = devices; device->dsu_did > 0; device++) {
      if (device->dsu_did == dsu_did) {
        debugMsg+="\r\nFound Target: ";
        debugMsg+=device->name;
        debugMsg+="\r\nFlash size: ";
        debugMsg+=device->flash_size;
        flashSize = device->flash_size;
        debugMsg+="\r\nFlash pages: ";
        debugMsg+=device->n_pages;
      }
    }
    return true;
  }
  
  //
  //
  //
  void disconnect(){
    dap_set_clock(50);
    deselect();
    dap_disconnect();
    debugMsg+="\r\ndisconnect(); ";    
  }
  
  //
  //
  //
  void setFuseBootloader16KB(){
    debugMsg+="\r\nsetFuseBootloader16KB()";
    fuseRead(); //MUST READ FUSES BEFORE SETTING OR WRITING ANY
    uint32_t * um;
    um = (uint32_t*)&_USER_ROW;

    debugMsg+="\r\nFuse: ";
    sprintf(format, "%x ", um[0]);
    debugMsg+=format;
    sprintf(format, "%x ", um[1]);
    debugMsg+=format;
    sprintf(format, "%x ", um[2]);
    debugMsg+=format;
    sprintf(format, "%x ", um[3]);
    debugMsg+=format;        
    // return;
    um[0]=0xF69A9239;
    fuseWrite();  
  }

  
  //
  //
  //
  void clone(){
    erase();
    uint32_t addr = program_start();
    // Overwrite when also flash bootloader
    addr=0x4000;  // only sketch
    addr=0x0; // all including Bootloader

    while(addr < flashSize){
      programBlock(addr, (uint8_t*)addr );
      addr += BUFSIZE;
    }

  }



  //
  //
  //
  void CPUID(){
    // Read CPU ID!!!
    debugMsg+="\r\nCPU UniqueID (serial) 128bit : ";
    uint32_t rr = dap_read_word(0x008061FC);
    sprintf(format, "%x ", rr);
    debugMsg+=format;
    rr = dap_read_word(0x00806010);
    sprintf(format, "%x ", rr);
    debugMsg+=format;
    rr = dap_read_word(0x00806014);
    sprintf(format, "%x ", rr);
    debugMsg+=format;
    rr = dap_read_word(0x00806018);
    sprintf(format, "%x ", rr);
    debugMsg+=format;
  }

  //
  //
  //
  void LED(bool value){
      // Set Pin to Ouput
      uint32_t register_word = dap_read_word(SAMD51_REG_PORT_DIR0); 
      register_word |= (1<<DOPPLER_LED);
      dap_write_word(SAMD51_REG_PORT_DIR0,register_word); 
      if(value){
        register_word = dap_read_word(SAMD51_REG_PORT_OUT0); 
        register_word |= (1<<DOPPLER_LED);
        dap_write_word(SAMD51_REG_PORT_OUT0,register_word); 
      }else{
        register_word = dap_read_word(SAMD51_REG_PORT_OUT0); 
        register_word &= ~(1<<DOPPLER_LED);
        dap_write_word(SAMD51_REG_PORT_OUT0,register_word); 
      }
  }

  //
  //  
  //
  void disableBootLoaderProtection(){
    // The SAMD51s bootloader protection can be temporarily disabled through an NVM
    uint32_t register_word = dap_read_word(SAMD51_REG_NVMCTRL_CTRLB); // read
    register_word = (0xA5 << 8) | 0x1a;   // modify
    dap_write_word(SAMD51_REG_NVMCTRL_CTRLB,register_word);  // write back
  }

};

#endif
