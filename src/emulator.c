#include <stdint.h>
#include <string.h>

#include <boolean.h>

#include "m68k/m68k.h"
#include "68328Functions.h"
#include "emulator.h"
#include "hardwareRegisters.h"
#include "memoryAccess.h"
#include "sed1376.h"
#include "sdcard.h"
#include "silkscreen.h"
#include "portability.h"
#include "emuFeatureRegistersSpec.h"


//Memory Map of Palm m515
//0x00000000-0x00FFFFFF RAM, the first 256(0x100) bytes is copyed from the first 256 bytes of ROM before boot, this applys to all Palms with the 68k architecture
//0x10000000-0x103FFFFF ROM, palmos41-en-m515.rom, substitute "en" for your language code
//0x1FF80000-0x1FF800B3 SED1376(Display Controller) Registers
//0x1FFA0000-0x1FFB3FFF SED1376(Display Controller) Framebuffer, this is not the same as the Palm framebuffer which is always 16 bit color,
//this buffer must be processed depending on whats in the SED1376 registers, the result is the Palm framebuffer
//0xFFFFF000-0xFFFFFDFF Hardware Registers
//0xFFFFFF00-0xFFFFFFFF Bootloader, only reads from UART into RAM and jumps to it, never executed in consumer Palms


uint8_t   palmRam[RAM_SIZE];
uint8_t   palmRom[ROM_SIZE];
uint8_t   palmReg[REG_SIZE];
input_t   palmInput;
sdcard_t  palmSdCard;
misc_hw_t palmMisc;
uint16_t  palmFramebuffer[160 * (160 + 60)];//really 160*160, the extra pixels are the silkscreened digitizer area
uint16_t  palmExtendedFramebuffer[320 * (320 + 120)];//really 320*320, the extra pixels are the silkscreened digitizer area
uint32_t  palmSpecialFeatures;
double    palmCrystalCycles;//how many cycles before toggling the 32.768 kHz crystal
double    palmCycleCounter;//can be greater then 0 if too many cycles where run
double    palmClockMultiplier;//used by the emulator to overclock the emulated Palm


uint64_t (*emulatorGetSysTime)();
uint64_t* (*emulatorGetSdCardStateChunkList)(uint64_t sessionId, uint64_t stateId);//returns the bps chunkIds for a stateId in the order they need to be applied
void (*emulatorSetSdCardStateChunkList)(uint64_t sessionId, uint64_t stateId, uint64_t* data);//sets the bps chunkIds for a stateId in the order they need to be applied
uint8_t* (*emulatorGetSdCardChunk)(uint64_t sessionId, uint64_t chunkId);
void (*emulatorSetSdCardChunk)(uint64_t sessionId, uint64_t chunkId, uint8_t* data, uint64_t size);

static inline bool allSdCardCallbacksPresent(){
   if(emulatorGetSysTime && emulatorGetSdCardStateChunkList && emulatorSetSdCardStateChunkList && emulatorGetSdCardChunk && emulatorSetSdCardChunk)
      return true;
   return false;
}


//debug
#if defined(EMU_DEBUG) && defined(EMU_OPCODE_LEVEL_DEBUG)
#define LOGGED_OPCODES 100
static bool invalidBehaviorAbort;
static char disassemblyBuffer[LOGGED_OPCODES][100];//store the opcode and program counter for the last 10 opcodes

#if defined(EMU_LOG_APIS)
const char* lookupTrap(uint16_t trap);
#endif

static char* takeStackDump(uint32_t bytes){
   char* textBytes = malloc(bytes * 2);
   uint32_t textBytesOffset = 0;
   uint32_t stackAddress = m68k_get_reg(NULL, M68K_REG_SP);

   textBytes[0] = '\0';

   for(uint32_t count = 0; count < bytes; count++){
      sprintf(textBytes + textBytesOffset, "%02X", m68k_read_memory_8(stackAddress + count));
      textBytesOffset = strlen(textBytes);
   }

   return textBytes;
}

#if defined(EMU_LOG_APIS)
static bool spammingTrap(uint16_t trap){
   switch(trap){

      case 0xA249://sysTrapHwrDelay
         return true;

      default:
         return false;
   }
   return false;
}
#endif

static void invalidBehaviorCheck(){
   char opcodeName[100];
   uint32_t lastProgramCounter = m68k_get_reg(NULL, M68K_REG_PPC);
   uint32_t programCounter = m68k_get_reg(NULL, M68K_REG_PC);
   uint16_t instruction = m68k_get_reg(NULL, M68K_REG_IR);
   bool invalidInstruction = !m68k_is_valid_instruction(instruction, M68K_CPU_TYPE_68000);
   bool invalidBank = (bankType[START_BANK(programCounter)] == CHIP_NONE);

   //get current opcode
   if(!invalidBank){
      //must dissasemble as 68020 to prevent address masking, is also more descriptive for invalid opcodes
      m68k_disassemble(opcodeName, lastProgramCounter, M68K_CPU_TYPE_68020);
   }
   else{
      strcpy(opcodeName, "Invalid bank, cant read");
   }
   sprintf(opcodeName + strlen(opcodeName), " at PC:0x%08X", lastProgramCounter);

   //shift opcode buffer
   for(uint32_t i = 0; i < LOGGED_OPCODES - 1; i++)
      strcpy(disassemblyBuffer[i], disassemblyBuffer[i + 1]);

   //add to opcode buffer
   strcpy(disassemblyBuffer[LOGGED_OPCODES - 1], opcodeName);

   if(invalidInstruction || invalidBank/* || (instruction == 0x0000 && lastProgramCounter != 0x00000000)*/){
      //0x0000 is "ori.b #$IMM, D0", effectivly NOP if the post op byte is 0x00 but still a valid opcode
      //usualy never encountered unless executing empty address space, so it still triggers debug abort
      m68k_end_timeslice();
      invalidBehaviorAbort = true;

      for(uint32_t i = 0; i < LOGGED_OPCODES; i++)
         debugLog("%s\n", disassemblyBuffer[i]);
      //currently CPU32 opcodes will be listed as "unknown", I cant change that properly unless I directly edit musashi source, something I want to avoid doing
      debugLog("Instruction:\"%s\", instruction value:0x%04X, bank type:%d\n", invalidInstruction ? "unknown" : opcodeName, instruction, bankType[START_BANK(lastProgramCounter)]);
   }

   //custom debug operations
   switch(programCounter){
      /*
      //case 0x10000566:
      case 0x100003F8:
         {
            //failing on executing first trap "HwrPreDebugInit"
            char* data = takeStackDump(32);
            debugLog("Stack dump:%s\n", data);
            free(data);
         }
         break;
      */

      default:
         break;
   }

#if defined(EMU_LOG_APIS)
   /*
   static uint32_t trapDumpWait = 300;
   static bool trapsNotDumped = true;

   if(trapsNotDumped && trapDumpWait == 0){
      debugLog("Trap dispatch controller is at:0x%08X, table addr:0x%08X\n", programCounter, m68k_read_memory_32(0x00000122));
      for(uint32_t count = 0x000; count < 0x475; count++){
         debugLog("API:0x%04X, location:0x%08X\n", count | 0xA000, m68k_read_memory_32(0x000008CC + count * 4));
      }
      trapsNotDumped = false;
   }
   */

   if(instruction == 0x4E4F){
      //Trap F/api call
      uint16_t trap = m68k_read_memory_16(lastProgramCounter + 2);
      if(!spammingTrap(trap)){
         debugLog("Trap F API:%s, API number:0x%04X, PC:0x%08X\n", lookupTrap(trap), trap, lastProgramCounter);
      }
      //trapDumpWait--;
   }
#endif
}
#endif


void emulatorInit(uint8_t* palmRomDump, uint8_t* palmBootDump, uint32_t specialFeatures){
   //CPU
   m68k_init();
   m68k_set_cpu_type(M68K_CPU_TYPE_68000);
   patchTo68328();
   m68k_set_reset_instr_callback(emulatorReset);
   m68k_set_int_ack_callback(interruptAcknowledge);
#if defined(EMU_DEBUG) && defined(EMU_OPCODE_LEVEL_DEBUG)
   for(uint32_t i = 0; i < LOGGED_OPCODES; i++)
      strcpy(disassemblyBuffer[i], "Not an opcode.\n");
   m68k_set_instr_hook_callback(invalidBehaviorCheck);
#endif
   resetHwRegisters();
   lowPowerStopActive = false;
   palmCycleCounter = 0.0;
   
   //memory
   memset(palmRam, 0x00, RAM_SIZE);
   memcpy(palmRom, palmRomDump, ROM_SIZE);
   if(palmBootDump)
       memcpy(palmReg + REG_SIZE - 1 - BOOTLOADER_SIZE, palmBootDump, BOOTLOADER_SIZE);
   else
       memset(palmReg + REG_SIZE - 1 - BOOTLOADER_SIZE, 0x00, BOOTLOADER_SIZE);
   memcpy(&palmFramebuffer[160 * 160], silkscreenData, SILKSCREEN_WIDTH * SILKSCREEN_HEIGHT * (SILKSCREEN_BPP / 8));
   resetAddressSpace();
   sed1376Reset();
   sdCardInit();
   
   //input
   palmInput.buttonUp = false;
   palmInput.buttonDown = false;
   palmInput.buttonLeft = false;//only used in hybrid mode
   palmInput.buttonRight = false;//only used in hybrid mode
   palmInput.buttonSelect = false;//only used in hybrid mode
   
   palmInput.buttonCalender = false;
   palmInput.buttonAddress = false;
   palmInput.buttonTodo = false;
   palmInput.buttonNotes = false;
   
   palmInput.buttonPower = false;
   palmInput.buttonContrast = false;
   
   palmInput.touchscreenX = 0;
   palmInput.touchscreenY = 0;
   palmInput.touchscreenTouched = false;
   
   //sdcard
   palmSdCard.sessionId = 0x0000000000000000;
   palmSdCard.stateId = 0x0000000000000000;
   palmSdCard.size = 0;
   palmSdCard.type = CARD_NONE;
   palmSdCard.inserted = false;
   
   //misc settable attributes
   palmMisc.batteryCharging = false;
   palmMisc.batteryLevel = 100;
   palmMisc.inDock = false;
   
   //config
   palmClockMultiplier = (specialFeatures & FEATURE_FAST_CPU) ? 2.0 : 1.0;//overclock
   palmSpecialFeatures = specialFeatures;
   
   //start running
   m68k_pulse_reset();
}

void emulatorExit(){
   sdCardExit();
}

void emulatorReset(){
   //reset doesnt clear RAM or sdcard, all programs are stored in RAM or on sdcard
   debugLog("Reset triggered, PC:0x%08X\n", m68k_get_reg(NULL, M68K_REG_PC));
   resetHwRegisters();
   resetAddressSpace();//address space must be reset after hardware registers because it is dependant on them
   sed1376Reset();
   m68k_pulse_reset();
}

void emulatorSetRtc(uint32_t days, uint32_t hours, uint32_t minutes, uint32_t seconds){
   setRtc(days, hours, minutes, seconds);
}

uint32_t emulatorSetSdCard(uint64_t size, uint8_t type){
   if(!allSdCardCallbacksPresent())
      return EMU_ERROR_CALLBACKS_NOT_SET;
   
   palmSdCard.sessionId = emulatorGetSysTime();
   palmSdCard.stateId = 0x0000000000000000;//set when saving state
   palmSdCard.size = size;
   palmSdCard.type = type;
   palmSdCard.inserted = true;
   
   return EMU_ERROR_NONE;
}

uint32_t emulatorGetStateSize(){
   uint32_t size = 0;
   
   size += sizeof(uint32_t);//save state version
   size += sizeof(uint32_t) * (M68K_REG_CAAR + 1);//CPU registers
   size += sizeof(uint8_t);//lowPowerStopActive
   size += RAM_SIZE;//system RAM buffer
   size += REG_SIZE;//hardware registers
   size += SED1376_FB_SIZE;//SED1376
   size += SED1376_LUT_SIZE * 3;//SED1376 r, g and b luts
   size += SED1376_REG_SIZE;//SED1376
   size += TOTAL_MEMORY_BANKS;//bank handlers
   size += sizeof(uint32_t) * 4 * CHIP_END;//chip select states
   size += sizeof(uint8_t) * 5 * CHIP_END;//chip select states
   size += sizeof(uint64_t) * 3;//palmSdCard
   size += sizeof(uint8_t) * 2;//palmSdCard
   size += sizeof(uint64_t) * 4;//32.32 fixed point double, timerXCycleCounter and CPU cycle timers
   size += sizeof(int32_t);//pllWakeWait
   size += sizeof(uint32_t);//clk32Counter
   size += sizeof(uint8_t) * 7;//palmMisc
   size += sizeof(uint32_t);//palmSpecialFeatures
   
   return size;
}

void emulatorSaveState(uint8_t* data){
   uint32_t offset = 0;
   
   //state validation, wont load states that are not from the same state version
   writeStateValueUint32(data + offset, SAVE_STATE_VERSION);
   offset += sizeof(uint32_t);
   
   //CPU
   for(uint32_t cpuReg = 0; cpuReg <=  M68K_REG_CAAR; cpuReg++){
      writeStateValueUint32(data + offset, m68k_get_reg(NULL, cpuReg));
      offset += sizeof(uint32_t);
   }
   writeStateValueBool(data + offset, lowPowerStopActive);
   offset += sizeof(uint8_t);
   
   //memory
   memcpy(data + offset, palmRam, RAM_SIZE);
   offset += RAM_SIZE;
   memcpy(data + offset, palmReg, REG_SIZE);
   offset += REG_SIZE;
   memcpy(data + offset, bankType, TOTAL_MEMORY_BANKS);
   offset += TOTAL_MEMORY_BANKS;
   for(uint32_t chip = CHIP_BEGIN; chip < CHIP_END; chip++){
      writeStateValueBool(data + offset, chips[chip].enable);
      offset += sizeof(uint8_t);
      writeStateValueUint32(data + offset, chips[chip].start);
      offset += sizeof(uint32_t);
      writeStateValueUint32(data + offset, chips[chip].size);
      offset += sizeof(uint32_t);
      writeStateValueUint32(data + offset, chips[chip].mask);
      offset += sizeof(uint32_t);
      writeStateValueBool(data + offset, chips[chip].inBootMode);
      offset += sizeof(uint8_t);
      writeStateValueBool(data + offset, chips[chip].readOnly);
      offset += sizeof(uint8_t);
      writeStateValueBool(data + offset, chips[chip].readOnlyForProtectedMemory);
      offset += sizeof(uint8_t);
      writeStateValueBool(data + offset, chips[chip].supervisorOnlyProtectedMemory);
      offset += sizeof(uint8_t);
      writeStateValueUint32(data + offset, chips[chip].unprotectedSize);
      offset += sizeof(uint32_t);
   }

   //SED1376
   memcpy(data + offset, sed1376Registers, SED1376_REG_SIZE);
   offset += SED1376_REG_SIZE;
   memcpy(data + offset, sed1376RLut, SED1376_LUT_SIZE);
   offset += SED1376_LUT_SIZE;
   memcpy(data + offset, sed1376GLut, SED1376_LUT_SIZE);
   offset += SED1376_LUT_SIZE;
   memcpy(data + offset, sed1376BLut, SED1376_LUT_SIZE);
   offset += SED1376_LUT_SIZE;
   memcpy(data + offset, sed1376Framebuffer, SED1376_FB_SIZE);
   offset += SED1376_FB_SIZE;

   //sdcard
   //update sdcard struct and save sdcard data
   if(allSdCardCallbacksPresent() && palmSdCard.type != CARD_NONE){
      uint64_t stateId = emulatorGetSysTime();
      sdCardSaveState(palmSdCard.sessionId, stateId);
      palmSdCard.stateId = stateId;
   }
   writeStateValueUint64(data + offset, palmSdCard.sessionId);
   offset += sizeof(uint64_t);
   writeStateValueUint64(data + offset, palmSdCard.stateId);
   offset += sizeof(uint64_t);
   writeStateValueUint64(data + offset, palmSdCard.size);
   offset += sizeof(uint64_t);
   writeStateValueUint8(data + offset, palmSdCard.type);
   offset += sizeof(uint8_t);
   writeStateValueBool(data + offset, palmSdCard.inserted);
   offset += sizeof(uint8_t);

   //timing
   writeStateValueDouble(data + offset, palmCrystalCycles);
   offset += sizeof(uint64_t);
   writeStateValueDouble(data + offset, palmCycleCounter);
   offset += sizeof(uint64_t);
   writeStateValueInt32(data + offset, pllWakeWait);
   offset += sizeof(int32_t);
   writeStateValueUint32(data + offset, clk32Counter);
   offset += sizeof(uint32_t);
   writeStateValueDouble(data + offset, timer1CycleCounter);
   offset += sizeof(uint64_t);
   writeStateValueDouble(data + offset, timer2CycleCounter);
   offset += sizeof(uint64_t);
   
   //misc
   writeStateValueBool(data + offset, palmMisc.alarmLed);
   offset += sizeof(uint8_t);
   writeStateValueBool(data + offset, palmMisc.lcdOn);
   offset += sizeof(uint8_t);
   writeStateValueBool(data + offset, palmMisc.backlightOn);
   offset += sizeof(uint8_t);
   writeStateValueBool(data + offset, palmMisc.vibratorOn);
   offset += sizeof(uint8_t);
   writeStateValueBool(data + offset, palmMisc.batteryCharging);
   offset += sizeof(uint8_t);
   writeStateValueUint8(data + offset, palmMisc.batteryLevel);
   offset += sizeof(uint8_t);
   writeStateValueUint8(data + offset, palmMisc.inDock);
   offset += sizeof(uint8_t);

   //features
   writeStateValueUint32(data + offset, palmSpecialFeatures);
   offset += sizeof(uint32_t);
}

void emulatorLoadState(uint8_t* data){
   uint32_t offset = 0;
   
   //state validation, wont load states that are not from the same state version
   if(readStateValueUint32(data + offset) != SAVE_STATE_VERSION)
      return;
   offset += sizeof(uint32_t);

   //CPU
   for(uint32_t cpuReg = 0; cpuReg <=  M68K_REG_CAAR; cpuReg++){
      m68k_set_reg(cpuReg, readStateValueUint32(data + offset));
      offset += sizeof(uint32_t);
   }
   lowPowerStopActive = readStateValueBool(data + offset);
   offset += 1;
   
   //memory
   memcpy(palmRam, data + offset, RAM_SIZE);
   offset += RAM_SIZE;
   memcpy(palmReg, data + offset, REG_SIZE);
   offset += REG_SIZE;
   memcpy(bankType, data + offset, TOTAL_MEMORY_BANKS);
   offset += TOTAL_MEMORY_BANKS;
   for(uint32_t chip = CHIP_BEGIN; chip < CHIP_END; chip++){
      chips[chip].enable = readStateValueBool(data + offset);
      offset += sizeof(uint8_t);
      chips[chip].start = readStateValueUint32(data + offset);
      offset += sizeof(uint32_t);
      chips[chip].size = readStateValueUint32(data + offset);
      offset += sizeof(uint32_t);
      chips[chip].mask = readStateValueUint32(data + offset);
      offset += sizeof(uint32_t);
      chips[chip].inBootMode = readStateValueBool(data + offset);
      offset += sizeof(uint8_t);
      chips[chip].readOnly = readStateValueBool(data + offset);
      offset += sizeof(uint8_t);
      chips[chip].readOnlyForProtectedMemory = readStateValueBool(data + offset);
      offset += sizeof(uint8_t);
      chips[chip].supervisorOnlyProtectedMemory = readStateValueBool(data + offset);
      offset += sizeof(uint8_t);
      chips[chip].unprotectedSize = readStateValueUint32(data + offset);
      offset += sizeof(uint32_t);
   }

   //SED1376
   memcpy(sed1376Registers, data + offset, SED1376_REG_SIZE);
   offset += SED1376_REG_SIZE;
   memcpy(sed1376RLut, data + offset, SED1376_LUT_SIZE);
   offset += SED1376_LUT_SIZE;
   memcpy(sed1376GLut, data + offset, SED1376_LUT_SIZE);
   offset += SED1376_LUT_SIZE;
   memcpy(sed1376BLut, data + offset, SED1376_LUT_SIZE);
   offset += SED1376_LUT_SIZE;
   memcpy(sed1376Framebuffer, data + offset, SED1376_FB_SIZE);
   offset += SED1376_FB_SIZE;
   sed1376RefreshLut();

   //sdcard
   palmSdCard.sessionId = readStateValueUint64(data + offset);
   offset += sizeof(uint64_t);
   palmSdCard.stateId = readStateValueUint64(data + offset);
   offset += sizeof(uint64_t);
   palmSdCard.size = readStateValueUint64(data + offset);
   offset += sizeof(uint64_t);
   palmSdCard.type = readStateValueUint8(data + offset);
   offset += sizeof(uint8_t);
   palmSdCard.inserted = readStateValueBool(data + offset);
   offset += sizeof(uint8_t);
   //update sdcard data from sdcard struct
   if(allSdCardCallbacksPresent() && palmSdCard.type != CARD_NONE){
      sdCardLoadState(palmSdCard.sessionId, palmSdCard.stateId);
   }

   //timing
   palmCrystalCycles = readStateValueDouble(data + offset);
   offset += sizeof(uint64_t);
   palmCycleCounter = readStateValueDouble(data + offset);
   offset += sizeof(uint64_t);
   pllWakeWait = readStateValueInt32(data + offset);
   offset += sizeof(int32_t);
   clk32Counter = readStateValueUint32(data + offset);
   offset += sizeof(uint32_t);
   timer1CycleCounter = readStateValueDouble(data + offset);
   offset += sizeof(uint64_t);
   timer2CycleCounter = readStateValueDouble(data + offset);
   offset += sizeof(uint64_t);
   
   //misc
   palmMisc.alarmLed = readStateValueBool(data + offset);
   offset += sizeof(uint8_t);
   palmMisc.lcdOn = readStateValueBool(data + offset);
   offset += sizeof(uint8_t);
   palmMisc.backlightOn = readStateValueBool(data + offset);
   offset += sizeof(uint8_t);
   palmMisc.vibratorOn = readStateValueBool(data + offset);
   offset += sizeof(uint8_t);
   palmMisc.batteryCharging = readStateValueBool(data + offset);
   offset += sizeof(uint8_t);
   palmMisc.batteryLevel = readStateValueUint8(data + offset);
   offset += sizeof(uint8_t);
   palmMisc.inDock = readStateValueUint8(data + offset);
   offset += sizeof(uint8_t);

   //features
   palmSpecialFeatures = readStateValueUint32(data + offset);
   offset += sizeof(uint32_t);
}

uint32_t emulatorInstallPrcPdb(uint8_t* data, uint32_t size){
   //pretend to pass for now
   //return EMU_ERROR_NOT_IMPLEMENTED;
   return EMU_ERROR_NONE;
}

void emulateFrame(){
   refreshButtonState();
   while(palmCycleCounter < CPU_FREQUENCY / EMU_FPS){
      if(pllIsOn() && !lowPowerStopActive)
         palmCycleCounter += m68k_execute(palmCrystalCycles * palmClockMultiplier) / palmClockMultiplier;//normaly 33mhz / 60fps
      else
         palmCycleCounter += palmCrystalCycles;
      clk32();
   }
   palmCycleCounter -= CPU_FREQUENCY / EMU_FPS;

   sed1376Render();
}

bool emulateUntilDebugEventOrFrameEnd(){
#if defined(EMU_DEBUG) && defined(EMU_OPCODE_LEVEL_DEBUG)
   invalidBehaviorAbort = false;

   refreshButtonState();
   while(palmCycleCounter < CPU_FREQUENCY / EMU_FPS){
      if(pllIsOn() && !lowPowerStopActive)
         palmCycleCounter += m68k_execute(palmCrystalCycles * palmClockMultiplier) / palmClockMultiplier;//normaly 33mhz / 60fps
      else
         palmCycleCounter += palmCrystalCycles;
      clk32();
      if(invalidBehaviorAbort)
         break;
   }
   palmCycleCounter -= CPU_FREQUENCY / EMU_FPS;

   sed1376Render();

   return invalidBehaviorAbort;
#else
   emulateFrame();
   return false;
#endif
}
