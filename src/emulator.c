#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "audio/blip_buf.h"
#include "audio/inductor.h"
#include "flx68000.h"
#include "emulator.h"
#include "hardwareRegisters.h"
#include "memoryAccess.h"
#include "sed1376.h"
#include "ads7846.h"
#include "pdiUsbD12.h"
#include "sdCard.h"
#include "silkscreen.h"
#include "portability.h"
#include "debug/sandbox.h"
#include "specs/emuFeatureRegistersSpec.h"


//Memory Map of Palm m515
//0x00000000-0x00FFFFFF RAM, CSC0 as RAS0, CSC1 as RAS1, CSD0 as CAS0 and CSD1 as CAS1
//0x10000000-0x103FFFFF ROM, CSA0, at address 0x00000000 during boot, palmos41-en-m515.rom, substitute "en" for your language code
//0x10400000-0x10C00000 USB, CSA1
//0x1FF80000-0x1FF800B3 SED1376(Display Controller) Registers, CSB0
//0x1FFA0000-0x1FFB3FFF SED1376(Display Controller) Framebuffer, CSB0, this is not the same as the Palm framebuffer which is always 16 bit color,
//this buffer must be processed depending on whats in the SED1376 registers, the result is the Palm framebuffer
//0xFFFFF000-0xFFFFFDFF Hardware Registers
//0xFFFFFE00-0xFFFFFFFF Bootloader, only reads from UART into RAM and jumps to it, never executed in consumer Palms
//VGhpcyBlbXVsYXRvciBpcyBkZWRpY2F0ZWQgdG8gdGhlIGJvdmluZSBtb28gY293cyB0aGF0IG1vby4=


static bool emulatorInitialized = false;

uint8_t*  palmRam;
uint8_t*  palmRom;
uint8_t*  palmReg;
input_t   palmInput;
sd_card_t palmSdCard;
misc_hw_t palmMisc;
uint16_t* palmFramebuffer;
uint16_t* palmExtendedFramebuffer;
int16_t*  palmAudio;
blip_t*   palmAudioResampler;
uint32_t  palmSpecialFeatures;
double    palmSysclksPerClk32;//how many SYSCLK cycles before toggling the 32.768 kHz crystal
double    palmCycleCounter;//can be greater then 0 if too many cycles where run
double    palmClockMultiplier;//used by the emulator to overclock the emulated Palm
uint32_t  palmFrameClk32s;//how many CLK32s have happened in the current frame
double    palmClk32Sysclks;//how many SYSCLKs have happened in the current CLK32


uint32_t emulatorInit(buffer_t palmRomDump, buffer_t palmBootDump, uint32_t specialFeatures){
   if(emulatorInitialized)
      return EMU_ERROR_NONE;

   if(!palmRomDump.data)
      return EMU_ERROR_INVALID_PARAMETER;

   //allocate the buffers, add 4 to memory regions to prevent SIGSEGV from accessing off the end
   palmRam = malloc(((specialFeatures & FEATURE_RAM_HUGE) ? SUPERMASSIVE_RAM_SIZE : RAM_SIZE) + 4);
   palmRom = malloc(ROM_SIZE + 4);
   palmReg = malloc(REG_SIZE + 4);
   palmFramebuffer = malloc(160 * (160 + 60) * sizeof(uint16_t));//really 160*160, the extra pixels are the silkscreened digitizer area
   palmAudio = malloc(AUDIO_SAMPLES_PER_FRAME * 2 * sizeof(int16_t));
   palmAudioResampler = blip_new(AUDIO_SAMPLE_RATE);//have 1 second of samples
   if(specialFeatures & FEATURE_320x320)
      palmExtendedFramebuffer = malloc(320 * (320 + 120) * sizeof(uint16_t));//really 320*320, the extra pixels are the silkscreened digitizer area
   else
      palmExtendedFramebuffer = NULL;
   if(!palmRam || !palmRom || !palmReg || !palmFramebuffer || !palmAudio || !palmAudioResampler || (!palmExtendedFramebuffer && (specialFeatures & FEATURE_320x320))){
      free(palmRam);
      free(palmRom);
      free(palmReg);
      free(palmFramebuffer);
      free(palmAudio);
      blip_delete(palmAudioResampler);
      free(palmExtendedFramebuffer);
      return EMU_ERROR_OUT_OF_MEMORY;
   }

   //CPU
   flx68000Init();
   palmCycleCounter = 0.0;

   //memory
   memset(palmRam, 0x00, (specialFeatures & FEATURE_RAM_HUGE) ? SUPERMASSIVE_RAM_SIZE : RAM_SIZE);
   memcpy(palmRom, palmRomDump.data, uMin(palmRomDump.size, ROM_SIZE));
   if(palmRomDump.size < ROM_SIZE)
      memset(palmRom + palmRomDump.size, 0x00, ROM_SIZE - palmRomDump.size);
   swap16_buffer_if_little(palmRom, ROM_SIZE / sizeof(uint16_t));
   if(palmBootDump.data){
      memcpy(palmReg + REG_SIZE - 1 - BOOTLOADER_SIZE, palmBootDump.data, uMin(palmBootDump.size, BOOTLOADER_SIZE));
      if(palmBootDump.size < BOOTLOADER_SIZE)
         memset(palmReg + REG_SIZE - 1 - BOOTLOADER_SIZE + palmBootDump.size, 0x00, BOOTLOADER_SIZE - palmBootDump.size);
      swap16_buffer_if_little(palmReg + REG_SIZE - 1 - BOOTLOADER_SIZE, BOOTLOADER_SIZE / sizeof(uint16_t));
   }
   else{
      memset(palmReg + REG_SIZE - 1 - BOOTLOADER_SIZE, 0x00, BOOTLOADER_SIZE);
   }
   memset(palmFramebuffer, 0x00, 160 * 160 * sizeof(uint16_t));
   memcpy(&palmFramebuffer[160 * 160], silkscreenData, SILKSCREEN_WIDTH * SILKSCREEN_HEIGHT * (SILKSCREEN_BPP / 8));
   if(palmExtendedFramebuffer){
      memset(palmExtendedFramebuffer, 0x00, 320 * 320 * sizeof(uint16_t));
      //add 320*320 silkscreen image later, 2xBRZ should be able to make 320*320 version of the 160*160 silkscreen
   }
   memset(palmAudio, 0x00, AUDIO_SAMPLES_PER_FRAME * 2/*channels*/ * sizeof(int16_t));
   blip_set_rates(palmAudioResampler, AUDIO_CLOCK_RATE, AUDIO_SAMPLE_RATE);
   flx68000Reset();
   sed1376Reset();
   ads7846Reset();
   pdiUsbD12Reset();
   inductorReset();
   sandboxInit();

   memset(&palmInput, 0x00, sizeof(palmInput));
   memset(&palmSdCard, 0x00, sizeof(palmSdCard));
   memset(&palmMisc, 0x00, sizeof(palmMisc));
   palmMisc.batteryLevel = 100;

   //config
   palmClockMultiplier = (specialFeatures & FEATURE_FAST_CPU) ? 2.00 : 1.00;//overclock
   //palmClockMultiplier *= 0.80;//run at 80% speed, 20% is likely memory waitstates
   palmSpecialFeatures = specialFeatures;
   setRtc(0, 0, 0, 0);//RTCTIME and DAYR are not cleared by reset, clear them manually in case the frontend doesnt set the RTC

   emulatorInitialized = true;

   //hack, patch ROM image
   sandboxCommand(SANDBOX_PATCH_OS, NULL);

   return EMU_ERROR_NONE;
}

void emulatorExit(){
   if(emulatorInitialized){
      free(palmRam);
      free(palmRom);
      free(palmReg);
      free(palmFramebuffer);
      free(palmAudio);
      blip_delete(palmAudioResampler);
      free(palmExtendedFramebuffer);
      free(palmSdCard.flashChip.data);
      emulatorInitialized = false;
   }
}

void emulatorReset(){
   //reset doesnt clear RAM or SD card, all programs are stored in RAM or on SD card
   debugLog("Reset triggered, PC:0x%08X\n", flx68000GetPc());
   flx68000Reset();
   sed1376Reset();
   ads7846Reset();
   pdiUsbD12Reset();
   inductorReset();
}

void emulatorSetRtc(uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds){
   setRtc(days, hours, minutes, seconds);
}

uint64_t emulatorGetStateSize(){
   uint64_t size = 0;

   size += sizeof(uint32_t);//save state version
   size += sizeof(uint32_t);//palmSpecialFeatures
   size += sizeof(uint64_t);//palmSdCard.flashChip.size, needs to be done first to verify the malloc worked
   size += flx68000StateSize();
   size += sed1376StateSize();
   size += ads7846StateSize();
   size += pdiUsbD12StateSize();
   if(palmSpecialFeatures & FEATURE_RAM_HUGE)
      size += SUPERMASSIVE_RAM_SIZE;//system RAM buffer
   else
      size += RAM_SIZE;//system RAM buffer
   size += REG_SIZE;//hardware registers
   size += TOTAL_MEMORY_BANKS;//bank handlers
   size += sizeof(uint32_t) * 4 * CHIP_END;//chip select states
   size += sizeof(uint8_t) * 5 * CHIP_END;//chip select states
   size += sizeof(uint64_t) * 4;//32.32 fixed point double, timerXCycleCounter and CPU cycle timers
   size += sizeof(int8_t);//pllSleepWait
   size += sizeof(int8_t);//pllWakeWait
   size += sizeof(uint32_t);//clk32Counter
   size += sizeof(uint64_t);//pctlrCpuClockDivider
   size += sizeof(uint16_t) * 2;//timerStatusReadAcknowledge
   size += sizeof(uint32_t);//interruptEdgeTriggered
   size += sizeof(uint16_t) * 9;//RX 8 * 16 SPI1 FIFO, 1 index is for FIFO full
   size += sizeof(uint16_t) * 9;//TX 8 * 16 SPI1 FIFO, 1 index is for FIFO full
   size += sizeof(uint8_t) * 4;//spi1(R/T)x(Read/Write)Position
   size += sizeof(int32_t);//pwm1ClocksToNextSample
   size += sizeof(uint8_t) * 6;//pwm1Fifo[6]
   size += sizeof(uint8_t) * 2;//pwm1(Read/Write)
   size += sizeof(uint64_t) * 2;//inductorCurrentCharge / inductorChargeAtLastSample
   size += sizeof(uint8_t) * 7;//palmMisc
   size += sizeof(uint32_t);//palmSdCard.command
   size += sizeof(uint8_t) * 2;//palmSdCard.response / palmSdCard.commandBitsRemaining
   size += sizeof(palmSdCard.dataPacket);//palmSdCard.dataPacket
   size += palmSdCard.flashChip.size;//palmSdCard.flashChip.data

   return size;
}

bool emulatorSaveState(buffer_t buffer){
   uint64_t offset = 0;

   if(buffer.size < emulatorGetStateSize())
      return false;//state cant fit

   //state validation, wont load states that are not from the same state version
   writeStateValue32(buffer.data + offset, SAVE_STATE_VERSION);
   offset += sizeof(uint32_t);

   //features, hotpluging emulated hardware is not supported
   writeStateValue32(buffer.data + offset, palmSpecialFeatures);
   offset += sizeof(uint32_t);

   //SD card size
   writeStateValue64(buffer.data + offset, palmSdCard.flashChip.size);
   offset += sizeof(uint64_t);

   //chips
   flx68000SaveState(buffer.data + offset);
   offset += flx68000StateSize();
   sed1376SaveState(buffer.data + offset);
   offset += sed1376StateSize();
   ads7846SaveState(buffer.data + offset);
   offset += ads7846StateSize();
   pdiUsbD12SaveState(buffer.data + offset);
   offset += pdiUsbD12StateSize();

   //memory
   if(palmSpecialFeatures & FEATURE_RAM_HUGE){
      memcpy(buffer.data + offset, palmRam, SUPERMASSIVE_RAM_SIZE);
      swap16_buffer_if_little(buffer.data + offset, SUPERMASSIVE_RAM_SIZE / sizeof(uint16_t));
      offset += SUPERMASSIVE_RAM_SIZE;
   }
   else{
      memcpy(buffer.data + offset, palmRam, RAM_SIZE);
      swap16_buffer_if_little(buffer.data + offset, RAM_SIZE / sizeof(uint16_t));
      offset += RAM_SIZE;
   }
   memcpy(buffer.data + offset, palmReg, REG_SIZE);
   swap16_buffer_if_little(buffer.data + offset, REG_SIZE / sizeof(uint16_t));
   offset += REG_SIZE;
   memcpy(buffer.data + offset, bankType, TOTAL_MEMORY_BANKS);
   offset += TOTAL_MEMORY_BANKS;
   for(uint8_t chip = CHIP_BEGIN; chip < CHIP_END; chip++){
      writeStateValueBool(buffer.data + offset, chips[chip].enable);
      offset += sizeof(uint8_t);
      writeStateValue32(buffer.data + offset, chips[chip].start);
      offset += sizeof(uint32_t);
      writeStateValue32(buffer.data + offset, chips[chip].lineSize);
      offset += sizeof(uint32_t);
      writeStateValue32(buffer.data + offset, chips[chip].mask);
      offset += sizeof(uint32_t);
      writeStateValueBool(buffer.data + offset, chips[chip].inBootMode);
      offset += sizeof(uint8_t);
      writeStateValueBool(buffer.data + offset, chips[chip].readOnly);
      offset += sizeof(uint8_t);
      writeStateValueBool(buffer.data + offset, chips[chip].readOnlyForProtectedMemory);
      offset += sizeof(uint8_t);
      writeStateValueBool(buffer.data + offset, chips[chip].supervisorOnlyProtectedMemory);
      offset += sizeof(uint8_t);
      writeStateValue32(buffer.data + offset, chips[chip].unprotectedSize);
      offset += sizeof(uint32_t);
   }

   //timing
   writeStateValueDouble(buffer.data + offset, palmSysclksPerClk32);
   offset += sizeof(uint64_t);
   writeStateValueDouble(buffer.data + offset, palmCycleCounter);
   offset += sizeof(uint64_t);
   writeStateValue8(buffer.data + offset, pllSleepWait);
   offset += sizeof(int8_t);
   writeStateValue8(buffer.data + offset, pllWakeWait);
   offset += sizeof(int8_t);
   writeStateValue32(buffer.data + offset, clk32Counter);
   offset += sizeof(uint32_t);
   writeStateValueDouble(buffer.data + offset, pctlrCpuClockDivider);
   offset += sizeof(uint64_t);
   writeStateValueDouble(buffer.data + offset, timerCycleCounter[0]);
   offset += sizeof(uint64_t);
   writeStateValueDouble(buffer.data + offset, timerCycleCounter[1]);
   offset += sizeof(uint64_t);
   writeStateValue16(buffer.data + offset, timerStatusReadAcknowledge[0]);
   offset += sizeof(uint16_t);
   writeStateValue16(buffer.data + offset, timerStatusReadAcknowledge[1]);
   offset += sizeof(uint16_t);
   writeStateValue32(buffer.data + offset, interruptEdgeTriggered);
   offset += sizeof(uint32_t);

   //SPI1
   for(uint8_t fifoPosition = 0; fifoPosition < 9; fifoPosition++){
      writeStateValue16(buffer.data + offset, spi1RxFifo[fifoPosition]);
      offset += sizeof(uint16_t);
   }
   for(uint8_t fifoPosition = 0; fifoPosition < 9; fifoPosition++){
      writeStateValue16(buffer.data + offset, spi1TxFifo[fifoPosition]);
      offset += sizeof(uint16_t);
   }
   writeStateValue8(buffer.data + offset, spi1RxReadPosition);
   offset += sizeof(uint8_t);
   writeStateValue8(buffer.data + offset, spi1RxWritePosition);
   offset += sizeof(uint8_t);
   writeStateValue8(buffer.data + offset, spi1TxReadPosition);
   offset += sizeof(uint8_t);
   writeStateValue8(buffer.data + offset, spi1TxWritePosition);
   offset += sizeof(uint8_t);

   //PWM1, audio
   writeStateValue32(buffer.data + offset, pwm1ClocksToNextSample);
   offset += sizeof(int32_t);
   for(uint8_t fifoPosition = 0; fifoPosition < 6; fifoPosition++){
      writeStateValue8(buffer.data + offset, pwm1Fifo[fifoPosition]);
      offset += sizeof(uint8_t);
   }
   writeStateValue8(buffer.data + offset, pwm1ReadPosition);
   offset += sizeof(uint8_t);
   writeStateValue8(buffer.data + offset, pwm1WritePosition);
   offset += sizeof(uint8_t);
   writeStateValueDouble(buffer.data + offset, inductorCurrentCharge);
   offset += sizeof(uint64_t);
   writeStateValueDouble(buffer.data + offset, inductorChargeAtLastSample);
   offset += sizeof(uint64_t);

   //misc
   writeStateValueBool(buffer.data + offset, palmMisc.powerButtonLed);
   offset += sizeof(uint8_t);
   writeStateValueBool(buffer.data + offset, palmMisc.lcdOn);
   offset += sizeof(uint8_t);
   writeStateValue8(buffer.data + offset, palmMisc.backlightLevel);
   offset += sizeof(uint8_t);
   writeStateValueBool(buffer.data + offset, palmMisc.vibratorOn);
   offset += sizeof(uint8_t);
   writeStateValueBool(buffer.data + offset, palmMisc.batteryCharging);
   offset += sizeof(uint8_t);
   writeStateValue8(buffer.data + offset, palmMisc.batteryLevel);
   offset += sizeof(uint8_t);
   writeStateValue8(buffer.data + offset, palmMisc.dataPort);
   offset += sizeof(uint8_t);

   //SD card
   writeStateValue32(buffer.data + offset, palmSdCard.command);
   offset += sizeof(uint32_t);
   writeStateValue8(buffer.data + offset, palmSdCard.commandBitsRemaining);
   offset += sizeof(uint8_t);
   writeStateValue8(buffer.data + offset, palmSdCard.response);
   offset += sizeof(uint8_t);
   memcpy(buffer.data + offset, palmSdCard.dataPacket, sizeof(palmSdCard.dataPacket));
   offset += sizeof(palmSdCard.dataPacket);
   memcpy(buffer.data + offset, palmSdCard.flashChip.data, palmSdCard.flashChip.size);
   offset += palmSdCard.flashChip.size;

   return true;
}

bool emulatorLoadState(buffer_t buffer){
   uint64_t offset = 0;

   //state validation, wont load states that are not from the same state version
   if(readStateValue32(buffer.data + offset) != SAVE_STATE_VERSION)
      return false;
   offset += sizeof(uint32_t);

   //features, hotpluging emulated hardware is not supported
   if(readStateValue32(buffer.data + offset) != palmSpecialFeatures)
      return false;
   offset += sizeof(uint32_t);

   //SD card size, the malloc when loading can make it fail, make sure if it fails the emulator state doesnt change
   uint64_t stateSdCardSize = readStateValue64(buffer.data + offset);
   uint8_t* stateSdCardBuffer = stateSdCardSize > 0 ? malloc(stateSdCardSize) : NULL;
   if(stateSdCardSize > 0 && !stateSdCardBuffer)
      return false;
   offset += sizeof(uint64_t);

   //chips
   flx68000LoadState(buffer.data + offset);
   offset += flx68000StateSize();
   sed1376LoadState(buffer.data + offset);
   offset += sed1376StateSize();
   ads7846LoadState(buffer.data + offset);
   offset += ads7846StateSize();
   pdiUsbD12LoadState(buffer.data + offset);
   offset += pdiUsbD12StateSize();

   //memory
   if(palmSpecialFeatures & FEATURE_RAM_HUGE){
      memcpy(palmRam, buffer.data + offset, SUPERMASSIVE_RAM_SIZE);
      swap16_buffer_if_little(palmRam, SUPERMASSIVE_RAM_SIZE / sizeof(uint16_t));
      offset += SUPERMASSIVE_RAM_SIZE;
   }
   else{
      memcpy(palmRam, buffer.data + offset, RAM_SIZE);
      swap16_buffer_if_little(palmRam, RAM_SIZE / sizeof(uint16_t));
      offset += RAM_SIZE;
   }
   memcpy(palmReg, buffer.data + offset, REG_SIZE);
   swap16_buffer_if_little(palmReg, REG_SIZE / sizeof(uint16_t));
   offset += REG_SIZE;
   memcpy(bankType, buffer.data + offset, TOTAL_MEMORY_BANKS);
   offset += TOTAL_MEMORY_BANKS;
   for(uint8_t chip = CHIP_BEGIN; chip < CHIP_END; chip++){
      chips[chip].enable = readStateValueBool(buffer.data + offset);
      offset += sizeof(uint8_t);
      chips[chip].start = readStateValue32(buffer.data + offset);
      offset += sizeof(uint32_t);
      chips[chip].lineSize = readStateValue32(buffer.data + offset);
      offset += sizeof(uint32_t);
      chips[chip].mask = readStateValue32(buffer.data + offset);
      offset += sizeof(uint32_t);
      chips[chip].inBootMode = readStateValueBool(buffer.data + offset);
      offset += sizeof(uint8_t);
      chips[chip].readOnly = readStateValueBool(buffer.data + offset);
      offset += sizeof(uint8_t);
      chips[chip].readOnlyForProtectedMemory = readStateValueBool(buffer.data + offset);
      offset += sizeof(uint8_t);
      chips[chip].supervisorOnlyProtectedMemory = readStateValueBool(buffer.data + offset);
      offset += sizeof(uint8_t);
      chips[chip].unprotectedSize = readStateValue32(buffer.data + offset);
      offset += sizeof(uint32_t);
   }

   //timing
   palmSysclksPerClk32 = readStateValueDouble(buffer.data + offset);
   offset += sizeof(uint64_t);
   palmCycleCounter = readStateValueDouble(buffer.data + offset);
   offset += sizeof(uint64_t);
   pllSleepWait = readStateValue8(buffer.data + offset);
   offset += sizeof(int8_t);
   pllWakeWait = readStateValue8(buffer.data + offset);
   offset += sizeof(int8_t);
   clk32Counter = readStateValue32(buffer.data + offset);
   offset += sizeof(uint32_t);
   pctlrCpuClockDivider = readStateValueDouble(buffer.data + offset);
   offset += sizeof(uint64_t);
   timerCycleCounter[0] = readStateValueDouble(buffer.data + offset);
   offset += sizeof(uint64_t);
   timerCycleCounter[1] = readStateValueDouble(buffer.data + offset);
   offset += sizeof(uint64_t);
   timerStatusReadAcknowledge[0] = readStateValue16(buffer.data + offset);
   offset += sizeof(uint16_t);
   timerStatusReadAcknowledge[1] = readStateValue16(buffer.data + offset);
   offset += sizeof(uint16_t);
   interruptEdgeTriggered = readStateValue32(buffer.data + offset);
   offset += sizeof(uint32_t);

   //SPI1
   for(uint8_t fifoPosition = 0; fifoPosition < 9; fifoPosition++){
      spi1RxFifo[fifoPosition] = readStateValue16(buffer.data + offset);
      offset += sizeof(uint16_t);
   }
   for(uint8_t fifoPosition = 0; fifoPosition < 9; fifoPosition++){
      spi1TxFifo[fifoPosition] = readStateValue16(buffer.data + offset);
      offset += sizeof(uint16_t);
   }
   spi1RxReadPosition = readStateValue8(buffer.data + offset);
   offset += sizeof(uint8_t);
   spi1RxWritePosition = readStateValue8(buffer.data + offset);
   offset += sizeof(uint8_t);
   spi1TxReadPosition = readStateValue8(buffer.data + offset);
   offset += sizeof(uint8_t);
   spi1TxWritePosition = readStateValue8(buffer.data + offset);
   offset += sizeof(uint8_t);

   //PWM1, audio
   pwm1ClocksToNextSample = readStateValue32(buffer.data + offset);
   offset += sizeof(int32_t);
   for(uint8_t fifoPosition = 0; fifoPosition < 6; fifoPosition++){
      pwm1Fifo[fifoPosition] = readStateValue8(buffer.data + offset);
      offset += sizeof(uint8_t);
   }
   pwm1ReadPosition = readStateValue8(buffer.data + offset);
   offset += sizeof(uint8_t);
   pwm1WritePosition = readStateValue8(buffer.data + offset);
   offset += sizeof(uint8_t);
   inductorCurrentCharge = readStateValueDouble(buffer.data + offset);
   offset += sizeof(uint64_t);
   inductorChargeAtLastSample = readStateValueDouble(buffer.data + offset);
   offset += sizeof(uint64_t);

   //misc
   palmMisc.powerButtonLed = readStateValueBool(buffer.data + offset);
   offset += sizeof(uint8_t);
   palmMisc.lcdOn = readStateValueBool(buffer.data + offset);
   offset += sizeof(uint8_t);
   palmMisc.backlightLevel = readStateValue8(buffer.data + offset);
   offset += sizeof(uint8_t);
   palmMisc.vibratorOn = readStateValueBool(buffer.data + offset);
   offset += sizeof(uint8_t);
   palmMisc.batteryCharging = readStateValueBool(buffer.data + offset);
   offset += sizeof(uint8_t);
   palmMisc.batteryLevel = readStateValue8(buffer.data + offset);
   offset += sizeof(uint8_t);
   palmMisc.dataPort = readStateValue8(buffer.data + offset);
   offset += sizeof(uint8_t);

   //SD card
   palmSdCard.command = readStateValue32(buffer.data + offset);
   offset += sizeof(uint32_t);
   palmSdCard.commandBitsRemaining = readStateValue8(buffer.data + offset);
   offset += sizeof(uint8_t);
   palmSdCard.response = readStateValue8(buffer.data + offset);
   offset += sizeof(uint8_t);
   memcpy(palmSdCard.dataPacket, buffer.data + offset, sizeof(palmSdCard.dataPacket));
   offset += sizeof(palmSdCard.dataPacket);
   if(palmSdCard.flashChip.data)
      free(palmSdCard.flashChip.data);
   palmSdCard.flashChip.data = stateSdCardBuffer;
   palmSdCard.flashChip.size = stateSdCardSize;
   memcpy(palmSdCard.flashChip.data, buffer.data + offset, stateSdCardSize);
   offset += stateSdCardSize;

   return true;
}

uint64_t emulatorGetRamSize(){
   return palmSpecialFeatures & FEATURE_RAM_HUGE ? SUPERMASSIVE_RAM_SIZE : RAM_SIZE;
}

bool emulatorSaveRam(buffer_t buffer){
   uint64_t size = palmSpecialFeatures & FEATURE_RAM_HUGE ? SUPERMASSIVE_RAM_SIZE : RAM_SIZE;

   if(buffer.size < size)
      return false;

   memcpy(buffer.data, palmRam, size);
   swap16_buffer_if_little(buffer.data, size / sizeof(uint16_t));

   return true;
}

bool emulatorLoadRam(buffer_t buffer){
   uint64_t size = palmSpecialFeatures & FEATURE_RAM_HUGE ? SUPERMASSIVE_RAM_SIZE : RAM_SIZE;

   if(buffer.size < size)
      return false;

   memcpy(palmRam, buffer.data, size);
   swap16_buffer_if_little(palmRam, size / sizeof(uint16_t));

   return true;
}

buffer_t emulatorGetSdCardBuffer(){
   debugLog("sd card buffer %x\n", palmSdCard.flashChip);
   return palmSdCard.flashChip;
}

uint32_t emulatorInsertSdCard(buffer_t image){
   //SD card is currently inserted
   if(palmSdCard.flashChip.data) {
      debugLog("already have sd card %x\n", palmSdCard.flashChip.data);
      return EMU_ERROR_RESOURCE_LOCKED;
   }

   palmSdCard.flashChip.data = malloc(image.size);
   if(!palmSdCard.flashChip.data)
      return EMU_ERROR_OUT_OF_MEMORY;

   if(image.data) {
      debugLog("got data, size: %x at %x\n", image.size, image.data);
      memcpy(palmSdCard.flashChip.data, image.data, image.size);
   } else {
      debugLog("got empty sd card\n");
      memset(palmSdCard.flashChip.data, 0x00, image.size);
   }

   palmSdCard.flashChip.size = image.size;

   return EMU_ERROR_NONE;
}

void emulatorEjectSdCard(){
   debugLog("ejecting sd card");
   //clear SD flash chip and controller
   if(palmSdCard.flashChip.data) {
      debugLog("freeing sd card data");
      free(palmSdCard.flashChip.data);
   }
   memset(&palmSdCard, 0x00, sizeof(palmSdCard));
}

uint32_t emulatorInstallPrcPdb(buffer_t file){
   return sandboxCommand(SANDBOX_INSTALL_APP, &file);
   //return EMU_ERROR_NONE;
}

void emulatorRunFrame(){
   //I/O
   refreshInputState();

   //CPU
   palmFrameClk32s = 0;
   for(; palmCycleCounter < (double)CRYSTAL_FREQUENCY / EMU_FPS; palmCycleCounter += 1.0){
      flx68000Execute();
      palmFrameClk32s++;
   }
   palmCycleCounter -= (double)CRYSTAL_FREQUENCY / EMU_FPS;

   //audio
   blip_end_frame(palmAudioResampler, blip_clocks_needed(palmAudioResampler, AUDIO_SAMPLES_PER_FRAME));
   /*
   if(blip_samples_avail(palmAudioResampler) != AUDIO_SAMPLES_PER_FRAME)
      printf("There are %d audio samples available, %d should be available\n", blip_samples_avail(palmAudioResampler), AUDIO_SAMPLES_PER_FRAME);
   */
   blip_read_samples(palmAudioResampler, palmAudio, AUDIO_SAMPLES_PER_FRAME, true);
   MULTITHREAD_LOOP for(uint32_t samples = 0; samples < AUDIO_SAMPLES_PER_FRAME * 2; samples += 2)
      palmAudio[samples + 1] = palmAudio[samples];

   //video
   sed1376Render();
}
