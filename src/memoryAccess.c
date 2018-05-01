#include <stdint.h>

#include "emulator.h"
#include "hardwareRegisters.h"
#include "memoryAccess.h"
#include "m68k/m68k.h"
#include "sed1376.h"


uint8_t bankType[TOTAL_MEMORY_BANKS];


//RAM accesses
static inline unsigned int ramRead8(unsigned int address){return BUFFER_READ_8(palmRam, address, chips[CHIP_D_RAM].start, chips[CHIP_D_RAM].mask);}
static inline unsigned int ramRead16(unsigned int address){return BUFFER_READ_16(palmRam, address, chips[CHIP_D_RAM].start, chips[CHIP_D_RAM].mask);}
static inline unsigned int ramRead32(unsigned int address){return BUFFER_READ_32(palmRam, address, chips[CHIP_D_RAM].start, chips[CHIP_D_RAM].mask);}
static inline void ramWrite8(unsigned int address, unsigned int value){BUFFER_WRITE_8(palmRam, address, chips[CHIP_D_RAM].start, chips[CHIP_D_RAM].mask, value);}
static inline void ramWrite16(unsigned int address, unsigned int value){BUFFER_WRITE_16(palmRam, address, chips[CHIP_D_RAM].start, chips[CHIP_D_RAM].mask, value);}
static inline void ramWrite32(unsigned int address, unsigned int value){BUFFER_WRITE_32(palmRam, address, chips[CHIP_D_RAM].start, chips[CHIP_D_RAM].mask, value);}

//ROM accesses
static inline unsigned int romRead8(unsigned int address){return BUFFER_READ_8(palmRom, address, chips[CHIP_A_ROM].start, chips[CHIP_A_ROM].mask);}
static inline unsigned int romRead16(unsigned int address){return BUFFER_READ_16(palmRom, address, chips[CHIP_A_ROM].start, chips[CHIP_A_ROM].mask);}
static inline unsigned int romRead32(unsigned int address){return BUFFER_READ_32(palmRom, address, chips[CHIP_A_ROM].start, chips[CHIP_A_ROM].mask);}

//SED1376 accesses
static inline unsigned int sed1376Read8(unsigned int address){
   if(sed1376PowerSaveEnabled())
      return 0x00;
   address -= chips[CHIP_B_SED].start;
   address &= chips[CHIP_B_SED].mask;
   if(address < SED1376_FB_OFFSET)
      return sed1376GetRegister(address & 0xFF);
   else
      return BUFFER_READ_8(sed1376Framebuffer, address, SED1376_FB_OFFSET, 0xFFFFFFFF);
}
static inline unsigned int sed1376Read16(unsigned int address){
   if(sed1376PowerSaveEnabled())
      return 0x0000;
   address -= chips[CHIP_B_SED].start;
   address &= chips[CHIP_B_SED].mask;
   if(address < SED1376_FB_OFFSET)
      return sed1376GetRegister(address & 0xFF);
   else
      return BUFFER_READ_16(sed1376Framebuffer, address, SED1376_FB_OFFSET, 0xFFFFFFFF);
}
static inline unsigned int sed1376Read32(unsigned int address){
   if(sed1376PowerSaveEnabled())
      return 0x00000000;
   address -= chips[CHIP_B_SED].start;
   address &= chips[CHIP_B_SED].mask;
   if(address < SED1376_FB_OFFSET)
      return sed1376GetRegister(address & 0xFF);
   else
      return BUFFER_READ_32(sed1376Framebuffer, address, SED1376_FB_OFFSET, 0xFFFFFFFF);
}
static inline void sed1376Write8(unsigned int address, unsigned int value){
   address -= chips[CHIP_B_SED].start;
   address &= chips[CHIP_B_SED].mask;
   if(address < SED1376_FB_OFFSET)
      sed1376SetRegister(address & 0xFF, value);
   else
      BUFFER_WRITE_8(sed1376Framebuffer, address, SED1376_FB_OFFSET, 0xFFFFFFFF, value);
}
static inline void sed1376Write16(unsigned int address, unsigned int value){
   address -= chips[CHIP_B_SED].start;
   address &= chips[CHIP_B_SED].mask;
   if(address < SED1376_FB_OFFSET)
      sed1376SetRegister(address & 0xFF, value);
   else
      BUFFER_WRITE_16(sed1376Framebuffer, address, SED1376_FB_OFFSET, 0xFFFFFFFF, value);
}
static inline void sed1376Write32(unsigned int address, unsigned int value){
   address -= chips[CHIP_B_SED].start;
   address &= chips[CHIP_B_SED].mask;
   if(address < SED1376_FB_OFFSET)
      sed1376SetRegister(address & 0xFF, value);
   else
      BUFFER_WRITE_32(sed1376Framebuffer, address, SED1376_FB_OFFSET, 0xFFFFFFFF, value);
}

static inline bool probeRead(uint8_t bank, unsigned int address){
   if(chips[bank].supervisorOnlyProtectedMemory && address >= chips[bank].unprotectedSize && !(m68k_get_reg(NULL, M68K_REG_SR) & 0x2000)){
      setPrivilegeViolation();
      return false;
   }
   return true;
}

static inline bool probeWrite(uint8_t bank, unsigned int address){
   if(chips[bank].readOnly){
      setWriteProtectViolation();
      return false;
   }
   else if(address >= chips[bank].unprotectedSize){
      if(chips[bank].supervisorOnlyProtectedMemory && !(m68k_get_reg(NULL, M68K_REG_SR) & 0x2000)){
         setPrivilegeViolation();
         return false;
      }
      if(chips[bank].readOnlyForProtectedMemory){
         setWriteProtectViolation();
         return false;
      }
   }
   return true;
}

/* Read from anywhere */
unsigned int m68k_read_memory_8(unsigned int address){
   uint8_t addressType = bankType[START_BANK(address)];

   if(!probeRead(addressType, address))
      return 0x00;

   switch(addressType){

      case CHIP_A_ROM:
         return romRead8(address);

      case CHIP_B_SED:
         return sed1376Read8(address);

      case CHIP_C_USB:
         return 0x00;

      case CHIP_D_RAM:
         return ramRead8(address);

      case CHIP_REGISTERS:
         return getHwRegister8(address);

      case CHIP_NONE:
         setBusErrorTimeOut();
         return 0x00;

      default:
         debugLog("Unknown bank type:%d\n", bankType[START_BANK(address)]);
         break;
   }

   return 0x00;
}

unsigned int m68k_read_memory_16(unsigned int address){
   uint8_t addressType = bankType[START_BANK(address)];

   if(!probeRead(addressType, address))
      return 0x0000;

   switch(addressType){

      case CHIP_A_ROM:
         return romRead16(address);

      case CHIP_B_SED:
         return sed1376Read16(address);

      case CHIP_C_USB:
         return 0x0000;

      case CHIP_D_RAM:
         return ramRead16(address);

      case CHIP_REGISTERS:
         return getHwRegister16(address);

      case CHIP_NONE:
         setBusErrorTimeOut();
         return 0x0000;

      default:
         debugLog("Unknown bank type:%d\n", bankType[START_BANK(address)]);
         break;
   }

   return 0x0000;
}

unsigned int m68k_read_memory_32(unsigned int address){
   uint8_t addressType = bankType[START_BANK(address)];

   if(!probeRead(addressType, address))
      return 0x00000000;

   switch(addressType){

      case CHIP_A_ROM:
         return romRead32(address);

      case CHIP_B_SED:
         return sed1376Read32(address);

      case CHIP_C_USB:
         return 0x00000000;

      case CHIP_D_RAM:
         return ramRead32(address);

      case CHIP_REGISTERS:
         return getHwRegister32(address);

      case CHIP_NONE:
         setBusErrorTimeOut();
         return 0x00000000;

      default:
         debugLog("Unknown bank type:%d\n", bankType[START_BANK(address)]);
         break;
   }

   return 0x00000000;
}

/* Write to anywhere */
void m68k_write_memory_8(unsigned int address, unsigned int value){
   uint8_t addressType = bankType[START_BANK(address)];

   if(!probeWrite(addressType, address))
      return;

   switch(addressType){

      case CHIP_A_ROM:
         break;

      case CHIP_B_SED:
         sed1376Write8(address, value);
         break;

      case CHIP_C_USB:
         break;

      case CHIP_D_RAM:
         ramWrite8(address, value);
         break;

      case CHIP_REGISTERS:
         setHwRegister8(address, value);
         break;

      case CHIP_NONE:
         setBusErrorTimeOut();
         break;

      default:
         debugLog("Unknown bank type:%d\n", bankType[START_BANK(address)]);
         break;
   }

   return;
}

void m68k_write_memory_16(unsigned int address, unsigned int value){
   uint8_t addressType = bankType[START_BANK(address)];

   if(!probeWrite(addressType, address))
      return;

   switch(addressType){

      case CHIP_A_ROM:
         break;

      case CHIP_B_SED:
         sed1376Write16(address, value);
         break;

      case CHIP_C_USB:
         break;

      case CHIP_D_RAM:
         ramWrite16(address, value);
         break;

      case CHIP_REGISTERS:
         setHwRegister16(address, value);
         break;

      case CHIP_NONE:
         setBusErrorTimeOut();
         break;

      default:
         debugLog("Unknown bank type:%d\n", bankType[START_BANK(address)]);
         break;
   }

   return;
}

void m68k_write_memory_32(unsigned int address, unsigned int value){
   uint8_t addressType = bankType[START_BANK(address)];

   if(!probeWrite(addressType, address))
      return;

   switch(addressType){

      case CHIP_A_ROM:
         break;

      case CHIP_B_SED:
         sed1376Write32(address, value);
         break;

      case CHIP_C_USB:
         break;

      case CHIP_D_RAM:
         ramWrite32(address, value);
         break;

      case CHIP_REGISTERS:
         setHwRegister32(address, value);
         break;

      case CHIP_NONE:
         setBusErrorTimeOut();
         break;

      default:
         debugLog("Unknown bank type:%d\n", bankType[START_BANK(address)]);
         break;
   }

   return;
}

void m68k_write_memory_32_pd(unsigned int address, unsigned int value){
   //musashi says to write 2 16 bit words, but for now I am just writing as 32bit long
   //normal 68k has 16 bit bus but Dragonball VZ has 32 bit bus, so just write all at once(unverified, may not be accurate)
   m68k_write_memory_32(address, value >> 16 | value << 16);
}

/* Memory access for the disassembler */
unsigned int m68k_read_disassembler_8(unsigned int address){return m68k_read_memory_8(address);}
unsigned int m68k_read_disassembler_16(unsigned int address){return m68k_read_memory_16(address);}
unsigned int m68k_read_disassembler_32(unsigned int address){return m68k_read_memory_32(address);}


static uint8_t getProperBankType(uint32_t bank){
   //special conditions
   if((bank & 0x00FF) == 0x00FF && registersAreXXFFMapped()){
      //XXFF register mode
      return CHIP_REGISTERS;
   }
   
   //normal banks
   if(BANK_IN_RANGE(bank, REG_START_ADDRESS, REG_SIZE)){
      //registers have first priority, they cover 0xFFFFF000 even if a chipselect overlaps this area or CHIP_A_ROM is in boot mode
      return CHIP_REGISTERS;
   }
   else if(chips[CHIP_A_ROM].inBootMode || (chips[CHIP_A_ROM].enable && BANK_IN_RANGE(bank, chips[CHIP_A_ROM].start, chips[CHIP_A_ROM].size))){
      return CHIP_A_ROM;
   }
   else if(chips[CHIP_B_SED].enable && BANK_IN_RANGE(bank, chips[CHIP_B_SED].start, chips[CHIP_B_SED].size) && sed1376ClockConnected()){
      return CHIP_B_SED;
   }
   else if(chips[CHIP_C_USB].enable && BANK_IN_RANGE(bank, chips[CHIP_C_USB].start, chips[CHIP_C_USB].size)){
      return CHIP_C_USB;
   }
   else if(chips[CHIP_D_RAM].enable && BANK_IN_RANGE(bank, chips[CHIP_D_RAM].start, chips[CHIP_D_RAM].size)){
      return CHIP_D_RAM;
   }

   return CHIP_NONE;
}

void setRegisterXXFFAccessMode(){
   for(uint32_t topByte = 0; topByte < 0x100; topByte++)
      bankType[START_BANK(topByte << 24 | 0x00FFF000)] = CHIP_REGISTERS;
}

void setRegisterFFFFAccessMode(){
   for(uint32_t topByte = 0; topByte < 0x100; topByte++){
      uint32_t bank = START_BANK(topByte << 24 | 0x00FFF000);
      bankType[bank] = getProperBankType(bank);
   }
}

void setSed1376Attached(bool attached){
   if(chips[CHIP_B_SED].enable){
      if(attached){
         for(uint32_t bank = START_BANK(chips[CHIP_B_SED].start); bank <= END_BANK(chips[CHIP_B_SED].start, chips[CHIP_B_SED].size); bank++)
             bankType[bank] = CHIP_B_SED;
      }
      else{
         for(uint32_t bank = START_BANK(chips[CHIP_B_SED].start); bank <= END_BANK(chips[CHIP_B_SED].start, chips[CHIP_B_SED].size); bank++)
            bankType[bank] = CHIP_NONE;
      }
   }
}

void resetAddressSpace(){
   for(uint32_t bank = 0; bank < TOTAL_MEMORY_BANKS; bank++)
      bankType[bank] = getProperBankType(bank);
}
