#include "testSuite.h"
#include "testSuiteConfig.h"
#include "hardwareRegisterNames.h"
#include "debug.h"
#include "tools.h"
#include "ugui.h"


var testButtonInput(){
   static Boolean  firstRun = true;
   static Boolean  polaritySwap;
   static uint8_t frameCount;
   static uint8_t  portDOriginalPolarity;
   uint16_t y = 0;
   
   if(firstRun){
      debugSafeScreenClear(C_WHITE);
      polaritySwap = false;
      frameCount = 0;
      portDOriginalPolarity = readArbitraryMemory8(HW_REG_ADDR(PDPOL));
      firstRun = false;
   }
   
   frameCount++;
   if(frameCount >= 30){
      writeArbitraryMemory8(HW_REG_ADDR(PDPOL), ~readArbitraryMemory8(HW_REG_ADDR(PDPOL)) & 0x07);
      frameCount = 0;
   }
   
   if(getButton(buttonLeft) && getButton(buttonRight) && getButton(buttonUp) && getButton(buttonBack) && !getButton(buttonDown) && !getButton(buttonSelect)){
      firstRun = true;
      writeArbitraryMemory8(HW_REG_ADDR(PDPOL), portDOriginalPolarity);
      exitSubprogram();
   }

   
   UG_PutString(0, y, "Press Left,Right,Up and Back to exit this test.");
   y += (FONT_HEIGHT + 1) * 2;
   
   UG_PutString(0, y, "This requirement is to allow button testing.");
   y += (FONT_HEIGHT + 1) * 2;
   
   StrPrintF(sharedDataBuffer, "PDPOL:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PDPOL)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "PDDATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PDDATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "PDKBEN:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PDKBEN)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   
   
   return makeVar(LENGTH_0, TYPE_NULL, 0);
}

var listDataRegisters(){
   static Boolean firstRun = true;
   uint16_t y = 0;
   
   if(firstRun){
      firstRun = false;
      debugSafeScreenClear(C_WHITE);
   }
   
   if(getButtonPressed(buttonBack)){
      firstRun = true;
      exitSubprogram();
   }
   
   StrPrintF(sharedDataBuffer, "PADATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PADATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "PBDATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PBDATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "PCDATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PCDATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "PDDATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PDDATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "PEDATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PEDATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "PFDATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PFDATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "PGDATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PGDATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "PJDATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PJDATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "PKDATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PKDATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "PMDATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PMDATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   
   return makeVar(LENGTH_0, TYPE_NULL, 0);
}

var interrogateSpi2(){
   static Boolean firstRun = true;
   uint16_t y = 0;
   
   if(firstRun){
      firstRun = false;
      debugSafeScreenClear(C_WHITE);
   }
   
   if(getButtonPressed(buttonBack)){
      firstRun = true;
      exitSubprogram();
   }
   
   StrPrintF(sharedDataBuffer, "PADATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PADATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "PBDATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PBDATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "PCDATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PCDATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   //PDDATA is buttons, not relevent to the SPI
   StrPrintF(sharedDataBuffer, "PEDATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PEDATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "PFDATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PFDATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "PGDATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PGDATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "PJDATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PJDATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "PKDATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PKDATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "PMDATA:0x%02X", readArbitraryMemory8(HW_REG_ADDR(PMDATA)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "SPICONT2:0x%02X", readArbitraryMemory8(HW_REG_ADDR(SPICONT2)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "SPIDATA2:0x%02X", readArbitraryMemory8(HW_REG_ADDR(SPIDATA2)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   
   return makeVar(LENGTH_0, TYPE_NULL, 0);
}

var tstat1GetSemaphoreLockOrder(){
   static Boolean firstRun = true;
   uint16_t testWriteValue = 0xF0F1;
   uint16_t y = 0;
   
   if(firstRun){
      firstRun = false;
      debugSafeScreenClear(C_WHITE);
   }
   
   if(getButtonPressed(buttonBack)){
      firstRun = true;
      exitSubprogram();
   }
   
   StrPrintF(sharedDataBuffer, "TSTAT1:0x%02X", readArbitraryMemory16(HW_REG_ADDR(TSTAT1)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   writeArbitraryMemory16(HW_REG_ADDR(TSTAT1), testWriteValue);
   StrPrintF(sharedDataBuffer, "Write TSTAT1:0x%02X", testWriteValue);
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "New TSTAT1:0x%02X", readArbitraryMemory16(HW_REG_ADDR(TSTAT1)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   writeArbitraryMemory16(HW_REG_ADDR(TSTAT1), 0xFFFF);
   StrPrintF(sharedDataBuffer, "Clear Semaphore");
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   StrPrintF(sharedDataBuffer, "New TSTAT1:0x%02X", readArbitraryMemory16(HW_REG_ADDR(TSTAT1)));
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   
   return makeVar(LENGTH_0, TYPE_NULL, 0);
}

var ads7846Read(){
   static Boolean firstRun = true;
   static Boolean referenceMode;
   static Boolean mode8Bit;
   uint8_t ads7846Channel;
   uint16_t y = 0;
   
   if(firstRun){
      firstRun = false;
      referenceMode = false;
      mode8Bit = false;
      debugSafeScreenClear(C_WHITE);
   }
   
   if(getButtonPressed(buttonSelect)){
      if(!referenceMode && !mode8Bit){
         referenceMode = true;
      }
      else if(referenceMode && !mode8Bit){
         referenceMode = false;
         mode8Bit = true;
      }
      else if(!referenceMode && mode8Bit){
         referenceMode = true;
      }
      else{
         referenceMode = false;
         mode8Bit = false;
      }
   }
   
   if(getButtonPressed(buttonBack)){
      firstRun = true;
      exitSubprogram();
   }
   
   StrPrintF(sharedDataBuffer, "Ref Mode:%d, 8bit:%d", referenceMode, mode8Bit);
   UG_PutString(0, y, sharedDataBuffer);
   y += FONT_HEIGHT + 1;
   
   for(ads7846Channel = 0; ads7846Channel < 8; ads7846Channel++){
      StrPrintF(sharedDataBuffer, "Ch:%d Value:0x%04X", ads7846Channel, ads7846GetValue(ads7846Channel, referenceMode, mode8Bit));
      UG_PutString(0, y, sharedDataBuffer);
      y += FONT_HEIGHT + 1;
   }
   
   return makeVar(LENGTH_0, TYPE_NULL, 0);
}
