#include <stdint.h>
#include <stdbool.h>

#include "emulator.h"


//command format:01IIIIIIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACCCCCCC1//I = index, A = argument, C = CRC

static inline void sdCardInvalidFormat(){
   palmSdCard.commandBitsRemaining = 63;
}


bool sdCardExchangeBit(bool bit){
   debugLog("sdcard exchange bit: %x", bit);
   //check validity of incoming bit
   switch(palmSdCard.commandBitsRemaining){
      case 47:
         if(bit) {
            debugLog("invalid 47 bits remaining\n");
            sdCardInvalidFormat();
         }
         return true;

      case 46:
      case 0:
         if(!bit) {
            debugLog("invalid null sd card type\n");
            sdCardInvalidFormat();
         }
         return true;
   }

   if(palmSdCard.commandBitsRemaining == 0){
      //process command
      uint8_t command = palmSdCard.command >> 40 & 0x3F;
      uint32_t argument = palmSdCard.command >> 8 & 0xFFFFFFFF;
      uint8_t crc = palmSdCard.command >> 1 & 0x7F;

      switch(command){

         default:
            debugLog("SD command:cmd:0x%02X, arg:0x%08X, CRC:0x%02X\n", command, argument, crc);
            break;
      }
   }
   else{
      debugLog("need more sd card data");
      //more data still needed
      palmSdCard.command <<= 1;
      palmSdCard.command |= bit;
      palmSdCard.commandBitsRemaining--;
   }

   //not done
   return true;//eek
}
