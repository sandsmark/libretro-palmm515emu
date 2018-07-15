#include <PalmOS.h>
#include <PalmUtils.h>
#include <stdint.h>
#include <Extensions/ExpansionMgr/VFSMgr.h>

#include "specs/hardwareRegisterNames.h"
#include "testSuite.h"
#include "viewer.h"
#include "debug.h"
#include "ugui.h"
#include "cpu.h"


Err makeFile(uint8_t* data, uint32_t size, char* fileName){
   uint16_t volRef = 0;
   uint32_t volIter = vfsIteratorStart;
   char fullPath[512];
   FileRef file;
   Err error;
   uint32_t count;
   
   StrCopy(fullPath, "/");
   StrCat(fullPath, fileName);
   
   error = VFSVolumeEnumerate(&volRef, &volIter);
   if(error != errNone)
      return error;
   
   error = VFSFileOpen(volRef, fullPath, vfsModeReadWrite | vfsModeCreate | vfsModeTruncate | vfsModeExclusive, &file);
   if(error != errNone)
      return error;
   
   /*the copy is used to anonymize the data source*/
   count = size;
   while(count != 0){
      uint32_t chunkSize = min(count, SHARED_DATA_BUFFER_SIZE);
      int32_t bytesWritten;
      uint32_t i;
      for(i = 0; i < chunkSize; i++){
         sharedDataBuffer[i] = data[i + size - count];
      }
      error = VFSFileWrite(file, chunkSize, sharedDataBuffer, &bytesWritten);
      if(bytesWritten != chunkSize || error != errNone){
         VFSFileClose(file);
         return error;
      }
      
      count -= chunkSize;
   }
   
   error = VFSFileClose(file);
   return error;
}

uint16_t ads7846GetValue(uint8_t channel, Boolean referenceMode, Boolean mode8Bit){
   uint8_t config = 0x80;
   uint16_t value;
   volatile uint32_t wasteTime;
   
   if(mode8Bit)
      config |= 0x08;
   
   if(referenceMode){
      config |= 0x04;
      config |= 0x03;/*force ADC and reference always on*/
   }
   
   config |= channel << 4 & 0x70;
   
   turnInterruptsOff();
   
   /*misc configs from HwrADC, ADS7846 dosent work without these*/
   writeArbitraryMemory8(HW_REG_ADDR(PGDATA), readArbitraryMemory8(HW_REG_ADDR(PGDATA)) & 0xFB);
   writeArbitraryMemory8(HW_REG_ADDR(PEDIR), readArbitraryMemory8(HW_REG_ADDR(PEDIR)) | 0x01);
   writeArbitraryMemory8(HW_REG_ADDR(PEDATA), readArbitraryMemory8(HW_REG_ADDR(PEDATA)) & 0xFE);
   writeArbitraryMemory8(HW_REG_ADDR(PEPUEN), readArbitraryMemory8(HW_REG_ADDR(PEPUEN)) & 0xFE);
   
   /*enable SPI 2 if disabled*/
   writeArbitraryMemory16(HW_REG_ADDR(SPICONT2), 0x4200);
   
   /*send data, only send the first 5 bits to leave the ADC enabled*/
   writeArbitraryMemory16(HW_REG_ADDR(SPIDATA2), config >> 3);
   writeArbitraryMemory16(HW_REG_ADDR(SPICONT2), 0x4304);
   while(readArbitraryMemory16(HW_REG_ADDR(SPICONT2)) & 0x0100);
   
   /*wait for the conversion to settle*/
   wasteTime = 0;
   while(wasteTime < 100)
      wasteTime++;
   
   /*send last 3 bits, wait 1 bit, get 8/12 bit result then pad the transfer to 16 bits*/
   writeArbitraryMemory16(HW_REG_ADDR(SPIDATA2), config << 14);
   writeArbitraryMemory16(HW_REG_ADDR(SPICONT2), 0x430F);
   while(readArbitraryMemory16(HW_REG_ADDR(SPICONT2)) & 0x0100);
   
   /*get value returned*/
   value = readArbitraryMemory16(HW_REG_ADDR(SPIDATA2)) & 0x0FFF;
   
   /*disable SPI2*/
   writeArbitraryMemory16(HW_REG_ADDR(SPICONT2), 0xE000);
   
   /*misc configs from HwrADC, ADS7846 dosent work without this*/
   writeArbitraryMemory8(HW_REG_ADDR(PGDATA), readArbitraryMemory8(HW_REG_ADDR(PGDATA)) | 0x04);
   
   turnInterruptsOn();
   
   return value;
}

var hexRamBrowser(){
   static Boolean firstRun = true;
   static uint32_t nibble;
   static uint32_t pointerValue;
   
   if(firstRun){
      debugSafeScreenClear(C_WHITE);
      nibble = 0x10000000;
      pointerValue = 0x77777777;/*in the middle of the address space*/
      firstRun = false;
   }
   
   if(getButtonPressed(buttonUp))
      pointerValue += nibble;
   
   if(getButtonPressed(buttonDown))
      pointerValue -= nibble;
   
   if(getButtonPressed(buttonRight))
      if(nibble > 0x00000001)
         nibble >>= 4;
   
   if(getButtonPressed(buttonLeft))
      if(nibble < 0x10000000)
         nibble <<= 4;
   
   if(getButtonPressed(buttonSelect)){
      /*open hex viewer*/
      firstRun = true;
      setSubprogramArgs(makeVar(LENGTH_ANY, TYPE_PTR, (uint64_t)pointerValue));/*length doesnt matter*/
      callSubprogram(hexViewer);
   }
   
   if(getButtonPressed(buttonBack)){
      firstRun = true;
      exitSubprogram();
   }
   
   StrPrintF(sharedDataBuffer, "Open Hex Viewer At:\n0x%08lX", pointerValue);
   UG_PutString(0, 0, sharedDataBuffer);
   
   return makeVar(LENGTH_0, TYPE_NULL, 0);
}

var getTrapAddress(){
   static Boolean firstRun = true;
   static uint16_t nibble;
   static uint16_t trapNum;
   
   if(firstRun){
      debugSafeScreenClear(C_WHITE);
      nibble = 0x100;
      trapNum = 0xA377;/*in the middle of the trap list*/
      firstRun = false;
   }
   
   if(getButtonPressed(buttonUp))
      trapNum = (trapNum + nibble & 0xFFF) | 0xA000;
   
   if(getButtonPressed(buttonDown))
      trapNum = (trapNum - nibble & 0xFFF) | 0xA000;
   
   if(getButtonPressed(buttonRight))
      if(nibble > 0x001)
         nibble >>= 4;
   
   if(getButtonPressed(buttonLeft))
      if(nibble < 0x100)
         nibble <<= 4;
   
   if(getButtonPressed(buttonSelect)){
      /*open hex viewer*/
      firstRun = true;
      setSubprogramArgs(makeVar(LENGTH_ANY, TYPE_PTR, (uint64_t)(uint32_t)SysGetTrapAddress(trapNum)));
      callSubprogram(valueViewer);
   }
   
   if(getButtonPressed(buttonBack)){
      firstRun = true;
      exitSubprogram();
   }
   
   StrPrintF(sharedDataBuffer, "Trap Num:\n0x%04X", trapNum);
   UG_PutString(0, 0, sharedDataBuffer);
   
   return makeVar(LENGTH_0, TYPE_NULL, 0);
}

var manualLssa(){
   static Boolean firstRun = true;
   static uint32_t nibble;
   static uint32_t hexValue;
   static uint32_t originalLssa;
   static Boolean customEnabled;
   
   if(firstRun){
      nibble = 0x10000000;
      hexValue = 0x77777777;
      originalLssa = readArbitraryMemory32(HW_REG_ADDR(LSSA));
      customEnabled = false;
      debugSafeScreenClear(C_WHITE);
      firstRun = false;
   }
   
   if(getButtonPressed(buttonUp))
      hexValue += nibble;
   
   if(getButtonPressed(buttonDown))
      hexValue -= nibble;
   
   if(getButtonPressed(buttonRight))
      if(nibble > 0x00000001)
         nibble >>= 4;
   
   if(getButtonPressed(buttonLeft))
      if(nibble < 0x10000000)
         nibble <<= 4;
   
   if(getButtonPressed(buttonSelect))
      if(customEnabled){
         writeArbitraryMemory32(HW_REG_ADDR(LSSA), originalLssa);
         customEnabled = false;
      }
      else{
         writeArbitraryMemory32(HW_REG_ADDR(LSSA), hexValue);
         customEnabled = true;
      }
   
   if(getButtonPressed(buttonBack)){
      firstRun = true;
      exitSubprogram();
   }
   
   StrPrintF(sharedDataBuffer, "Enter switchs between this address and the custom LSSA.\nNew LSSA:\n0x%08lX", hexValue);
   UG_PutString(0, 0, sharedDataBuffer);
   
   return makeVar(LENGTH_0, TYPE_NULL, 0);
}

var dumpBootloaderToFile(){
   makeFile((uint8_t*)0xFFFFFE00, 0x200, "BOOTLOADER.BIN");
   exitSubprogram();
   return makeVar(LENGTH_0, TYPE_NULL, 0);
}

var listRomInfo(){
   static Boolean firstRun = true;
   uint16_t y = 0;
   
   if(firstRun){
      uint16_t csa = readArbitraryMemory16(HW_REG_ADDR(CSA));
      uint16_t csgba = readArbitraryMemory16(HW_REG_ADDR(CSGBA));
      uint16_t csugba = readArbitraryMemory16(HW_REG_ADDR(CSUGBA));
      uint32_t romAddress = 0x00000000;
      uint32_t romSize = 0x00000000;
      
      firstRun = false;
      
      /*get ROM info*/
      if(csugba & 0x8000)
         romAddress |= (uint32_t)csugba << 17 & 0xE0000000;
      romAddress |= (uint32_t)csgba << 13;
      romSize = 0x20000/*128k*/ << (csa >> 1 & 0x0007);
      
      debugSafeScreenClear(C_WHITE);
      StrPrintF(sharedDataBuffer, "ROM Address:0x%08lX", romAddress);
      UG_PutString(0, y, sharedDataBuffer);
      y += FONT_HEIGHT + 1;
      StrPrintF(sharedDataBuffer, "ROM Size:0x%08lX", romSize);
      UG_PutString(0, y, sharedDataBuffer);
      y += FONT_HEIGHT + 1;
      StrPrintF(sharedDataBuffer, "Press select to dump");
      UG_PutString(0, y, sharedDataBuffer);
      y += FONT_HEIGHT + 1;
   }
   
   if(getButtonPressed(buttonBack)){
      firstRun = true;
      exitSubprogram();
   }
   
   if(getButtonPressed(buttonSelect)){
      /*ROM must be attached to CSA(chip select a) on Dragonball VZ as it controls the boot up process*/
      /*the only 3 restrictions to use this dumper are, you need a Dragonball VZ Palm, an SD card must be inserted and the SD card must be bigger than ROM / 128k rounded up to the nearest power of 2 * 128k*/
      uint16_t csa = readArbitraryMemory16(HW_REG_ADDR(CSA));
      uint16_t csgba = readArbitraryMemory16(HW_REG_ADDR(CSGBA));
      uint16_t csugba = readArbitraryMemory16(HW_REG_ADDR(CSUGBA));
      uint32_t romAddress = 0x00000000;
      uint32_t romSize = 0x00000000;
      
      /*get ROM info*/
      if(csugba & 0x8000)
         romAddress |= (uint32_t)csugba << 17 & 0xE0000000;
      romAddress |= (uint32_t)csgba << 13;
      romSize = 0x20000/*128k*/ << (csa >> 1 & 0x0007);
      
      /*CSA has no protected area, can read directly from address space without changing it*/
      
      debugSafeScreenClear(C_WHITE);
      StrPrintF(sharedDataBuffer, "Dumping...", romAddress);
      UG_PutString(0, 0, sharedDataBuffer);
      forceFrameRedraw();
      makeFile((uint8_t*)romAddress, romSize, "ROM.BIN");
      
      firstRun = true;
   }
   
   return makeVar(LENGTH_0, TYPE_NULL, 0);
}

var listChipSelects(){
   static Boolean firstRun = true;
   uint16_t y = 0;
   
   if(firstRun){
      debugSafeScreenClear(C_WHITE);
      StrPrintF(sharedDataBuffer, "CSCTRL1:0x%04X", readArbitraryMemory16(HW_REG_ADDR(CSCTRL1)));
      UG_PutString(0, y, sharedDataBuffer);
      y += FONT_HEIGHT + 1;
      StrPrintF(sharedDataBuffer, "CSUGBA:0x%04X", readArbitraryMemory16(HW_REG_ADDR(CSUGBA)));
      UG_PutString(0, y, sharedDataBuffer);
      y += FONT_HEIGHT + 1;
      StrPrintF(sharedDataBuffer, "CSA:0x%04X", readArbitraryMemory16(HW_REG_ADDR(CSA)));
      UG_PutString(0, y, sharedDataBuffer);
      y += FONT_HEIGHT + 1;
      StrPrintF(sharedDataBuffer, "CSGBA:0x%04X", readArbitraryMemory16(HW_REG_ADDR(CSGBA)));
      UG_PutString(0, y, sharedDataBuffer);
      y += FONT_HEIGHT + 1;
      StrPrintF(sharedDataBuffer, "CSB:0x%04X", readArbitraryMemory16(HW_REG_ADDR(CSB)));
      UG_PutString(0, y, sharedDataBuffer);
      y += FONT_HEIGHT + 1;
      StrPrintF(sharedDataBuffer, "CSGBB:0x%04X", readArbitraryMemory16(HW_REG_ADDR(CSGBB)));
      UG_PutString(0, y, sharedDataBuffer);
      y += FONT_HEIGHT + 1;
      StrPrintF(sharedDataBuffer, "CSC:0x%04X", readArbitraryMemory16(HW_REG_ADDR(CSC)));
      UG_PutString(0, y, sharedDataBuffer);
      y += FONT_HEIGHT + 1;
      StrPrintF(sharedDataBuffer, "CSGBC:0x%04X", readArbitraryMemory16(HW_REG_ADDR(CSGBC)));
      UG_PutString(0, y, sharedDataBuffer);
      y += FONT_HEIGHT + 1;
      StrPrintF(sharedDataBuffer, "CSD:0x%04X", readArbitraryMemory16(HW_REG_ADDR(CSD)));
      UG_PutString(0, y, sharedDataBuffer);
      y += FONT_HEIGHT + 1;
      StrPrintF(sharedDataBuffer, "CSGBD:0x%04X", readArbitraryMemory16(HW_REG_ADDR(CSGBD)));
      UG_PutString(0, y, sharedDataBuffer);
      y += FONT_HEIGHT + 1;
   }
   
   if(getButtonPressed(buttonBack)){
      firstRun = true;
      exitSubprogram();
   }
   
   return makeVar(LENGTH_0, TYPE_NULL, 0);
}
