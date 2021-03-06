#include <stdint.h>
#include <string.h>

#include "emulator.h"
#include "portability.h"
#include "hardwareRegisters.h"
#include "flx68000.h"//for flx68000GetPc()
#include "specs/sed1376RegisterNames.h"
#include "debug/sandbox.h"


//the SED1376 has only 16 address lines(17 if you count the line that switches between registers and framebuffer) and 16 data lines, the most you can read at once is 16 bits, registers are 8 bits

//the actions described below are just my best guesses after reading the datasheet, I have not tested with actual hardware
//you read and write the register on the address lines set
//8 bit register access works normal
//16 bit register reads will result in you getting (0x00 << 8 | register)(this is unverified)
//16 bit register writes will result in you writing the lower 8 bits(this is unverified)
//32 bit register reads will result in doing 2 16 bit reads
//32 bit register writes will result in doing 2 16 bit writes

//The LCD power-on sequence is activated by programming the Power Save Mode Enable bit (REG[A0h] bit 0) to 0.
//The LCD power-off sequence is activated by programming the Power Save Mode Enable bit (REG[A0h] bit 0) to 1.


#define SED1376_REG_SIZE 0xB4
#define SED1376_LUT_SIZE 0x100
#define SED1376_FB_SIZE  0x20000//actual size is 0x14000, but that cant be masked off by address lines so size is increased to prevent buffer overflow


uint8_t sed1376Framebuffer[SED1376_FB_SIZE];

static uint8_t  sed1376Registers[SED1376_REG_SIZE];
static uint8_t  sed1376RLut[SED1376_LUT_SIZE];
static uint8_t  sed1376GLut[SED1376_LUT_SIZE];
static uint8_t  sed1376BLut[SED1376_LUT_SIZE];
static uint16_t sed1376OutputLut[SED1376_LUT_SIZE];//used to speed up pixel conversion
static uint32_t screenStartAddress;
static uint16_t lineSize;
static uint16_t (*renderPixel)(uint16_t x, uint16_t y);


#include "sed1376Accessors.c.h"

static uint32_t getBufferStartAddress(){
   uint32_t screenStartAddress = sed1376Registers[DISP_ADDR_2] << 16 | sed1376Registers[DISP_ADDR_1] << 8 | sed1376Registers[DISP_ADDR_0];
   switch((sed1376Registers[SPECIAL_EFFECT] & 0x03) * 90){
      case 0:
         //desired byte address / 4.
         screenStartAddress *= 4;
         break;

      case 90:
         //((desired byte address + (panel height * bpp / 8)) / 4) - 1.
         screenStartAddress += 1;
         screenStartAddress *= 4;
         //screenStartAddress - (panelHeight * bpp / 8);
         break;

      case 180:
         //((desired byte address + (panel width * panel height * bpp / 8)) / 4) - 1.
         screenStartAddress += 1;
         screenStartAddress *= 4;
         //screenStartAddress - (panelWidth * panelHeight * bpp / 8);
         break;

      case 270:
         //(desired byte address + ((panel width - 1) * panel height * bpp / 8)) / 4.
         screenStartAddress *= 4;
         //screenStartAddress -= ((panelWidth - 1) * panelHeight * bpp / 8);
         break;
   }

   return screenStartAddress;
}

static uint32_t getPipStartAddress(){
   uint32_t pipStartAddress = sed1376Registers[PIP_ADDR_2] << 16 | sed1376Registers[PIP_ADDR_1] << 8 | sed1376Registers[PIP_ADDR_0];
   switch((sed1376Registers[SPECIAL_EFFECT] & 0x03) * 90){
      case 0:
         //desired byte address / 4.
         pipStartAddress *= 4;
         break;

      case 90:
         //((desired byte address + (panel height * bpp / 8)) / 4) - 1.
         pipStartAddress += 1;
         pipStartAddress *= 4;
         //pipStartAddress - (panelHeight * bpp / 8);
         break;

      case 180:
         //((desired byte address + (panel width * panel height * bpp / 8)) / 4) - 1.
         pipStartAddress += 1;
         pipStartAddress *= 4;
         //pipStartAddress - (panelWidth * panelHeight * bpp / 8);
         break;

      case 270:
         //(desired byte address + ((panel width - 1) * panel height * bpp / 8)) / 4.
         pipStartAddress *= 4;
         //pipStartAddress -= ((panelWidth - 1) * panelHeight * bpp / 8);
         break;
   }

   return pipStartAddress;
}

void sed1376Reset(){
   memset(sed1376Registers, 0x00, SED1376_REG_SIZE);
   memset(sed1376OutputLut, 0x00, SED1376_LUT_SIZE * sizeof(uint16_t));
   memset(sed1376RLut, 0x00, SED1376_LUT_SIZE);
   memset(sed1376GLut, 0x00, SED1376_LUT_SIZE);
   memset(sed1376BLut, 0x00, SED1376_LUT_SIZE);
   memset(sed1376Framebuffer, 0x00, SED1376_FB_SIZE);

   palmMisc.backlightLevel = 0;
   palmMisc.lcdOn = false;

   renderPixel = NULL;

   sed1376Registers[REV_CODE] = 0x28;
   sed1376Registers[DISP_BUFF_SIZE] = 0x14;

   //timing hack
   sed1376Registers[PWR_SAVE_CFG] = 0x80;
}

uint64_t sed1376StateSize(){
   uint64_t size = 0;

   size += SED1376_REG_SIZE;
   size += SED1376_LUT_SIZE * 3;
   size += SED1376_FB_SIZE;

   return size;
}

void sed1376SaveState(uint8_t* data){
   uint64_t offset = 0;

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
}

void sed1376LoadState(uint8_t* data){
   uint64_t offset = 0;

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

   //refresh LUT
   MULTITHREAD_LOOP for(uint16_t count = 0; count < SED1376_LUT_SIZE; count++)
      sed1376OutputLut[count] = makeRgb16FromSed666(sed1376RLut[count], sed1376GLut[count], sed1376BLut[count]);
}

bool sed1376PowerSaveEnabled(){
   return sed1376Registers[PWR_SAVE_CFG] & 0x01;
}

uint8_t sed1376GetRegister(uint8_t address){
   //returning 0x00 on power save mode is done in the sed1376ReadXX functions

   if(sandboxRunning())
      debugLog("SED1376 register read from 0x%02X, PC 0x%08X.\n", address, flx68000GetPc());

   switch(address){
      case LUT_READ_LOC:
      case LUT_WRITE_LOC:
      case LUT_B_WRITE:
      case LUT_G_WRITE:
      case LUT_R_WRITE:
         //write only
         return 0x00;

      case PWR_SAVE_CFG:
      case SPECIAL_EFFECT:
      case DISP_MODE:
      case LINE_SIZE_0:
      case LINE_SIZE_1:
      case PIP_ADDR_0:
      case PIP_ADDR_1:
      case PIP_ADDR_2:
      case SCRATCH_0:
      case SCRATCH_1:
      case GPIO_CONF_0:
      case GPIO_CONT_0:
      case GPIO_CONF_1:
      case GPIO_CONT_1:
      case MEM_CLK:
      case PIXEL_CLK:
         //simple read, no actions needed
         return sed1376Registers[address];

      default:
         return 0x00;
   }
   
   return 0x00;//silence warnings
}

void sed1376SetRegister(uint8_t address, uint8_t value){

   if(sandboxRunning())
      debugLog("SED1376 register write 0x%02X to 0x%02X, PC 0x%08X.\n", value, address, flx68000GetPc());

   switch(address){
      case PWR_SAVE_CFG:
         //bit 7 must always be set, timing hack
         sed1376Registers[address] = (value & 0x01) | 0x80;
         break;

      case DISP_MODE:
         sed1376Registers[address] = value & 0xF7;
         break;

      case PANEL_TYPE:
         sed1376Registers[address] = value & 0xFB;
         break;

      case SPECIAL_EFFECT:
         sed1376Registers[address] = value & 0xD3;
         break;

      case DISP_ADDR_2:
      case PIP_ADDR_2:
         sed1376Registers[address] = value & 0x01;
         break;

      case LINE_SIZE_1:
      case PIP_LINE_SZ_1:
      case PIP_X_START_1:
      case PIP_X_END_1:
      case PIP_Y_START_1:
      case PIP_Y_END_1:
         sed1376Registers[address] = value & 0x03;
         break;

      case LUT_WRITE_LOC:
         sed1376BLut[value] = sed1376Registers[LUT_B_WRITE];
         sed1376GLut[value] = sed1376Registers[LUT_G_WRITE];
         sed1376RLut[value] = sed1376Registers[LUT_R_WRITE];
         //wheter or not these are changed on a write, or if that depends on the LUT_READ_LOC register is yet to be tested, turn this off for now
         /*
         sed1376Registers[LUT_B_READ] = sed1376BLut[value];
         sed1376Registers[LUT_G_READ] = sed1376GLut[value];
         sed1376Registers[LUT_R_READ] = sed1376RLut[value];
         */
         //debugLog("Writing R:0x%02X, G:0x%02X, B:0x%02X to LUT:0x%02X\n", sed1376RLut[value], sed1376GLut[value], sed1376BLut[value], value);
         sed1376OutputLut[value] = makeRgb16FromSed666(sed1376RLut[value], sed1376GLut[value], sed1376BLut[value]);
         break;

      case LUT_READ_LOC:
         sed1376Registers[LUT_B_READ] = sed1376BLut[value];
         sed1376Registers[LUT_G_READ] = sed1376GLut[value];
         sed1376Registers[LUT_R_READ] = sed1376RLut[value];
         break;

      case GPIO_CONF_0:
      case GPIO_CONT_0:
         sed1376Registers[address] = value & 0x7F;
         updateLcdStatus();
         break;

      case GPIO_CONF_1:
      case GPIO_CONT_1:
         sed1376Registers[address] = value & 0x80;
         break;

      case MEM_CLK:
         sed1376Registers[address] = value & 0x30;
         break;

      case PIXEL_CLK:
         sed1376Registers[address] = value & 0x73;
         break;

      case LUT_B_WRITE:
      case LUT_G_WRITE:
      case LUT_R_WRITE:
         sed1376Registers[address] = value & 0xFC;
         break;

      case SCRATCH_0:
      case SCRATCH_1:
      case DISP_ADDR_0:
      case DISP_ADDR_1:
      case PIP_ADDR_0:
      case PIP_ADDR_1:
      case LINE_SIZE_0:
      case PIP_LINE_SZ_0:
      case PIP_X_START_0:
      case PIP_X_END_0:
      case PIP_Y_START_0:
      case PIP_Y_END_0:
         //simple write, no actions needed
         sed1376Registers[address] = value;
         break;

      default:
         break;
   }
}

void sed1376Render(){
   if(palmMisc.lcdOn && pllIsOn() && !sed1376PowerSaveEnabled() && !(sed1376Registers[DISP_MODE] & 0x80)){
      //only render if LCD on, PLL on, power save off, and force blank off, SED1376 clock is provided by the CPU, if its off so is the SED
      bool color = sed1376Registers[PANEL_TYPE] & 0x40;
      bool pictureInPictureEnabled = sed1376Registers[SPECIAL_EFFECT] & 0x10;
      uint8_t bitDepth = 1 << (sed1376Registers[DISP_MODE] & 0x07);
      uint16_t rotation = 90 * (sed1376Registers[SPECIAL_EFFECT] & 0x03);

      screenStartAddress = getBufferStartAddress();
      lineSize = (sed1376Registers[LINE_SIZE_1] << 8 | sed1376Registers[LINE_SIZE_0]) * 4;
      selectRenderer(color, bitDepth);

      if(renderPixel){
         MULTITHREAD_DOUBLE_LOOP for(uint16_t pixelY = 0; pixelY < 160; pixelY++)
            for(uint16_t pixelX = 0; pixelX < 160; pixelX++)
               palmFramebuffer[pixelY * 160 + pixelX] = renderPixel(pixelX, pixelY);

         //debugLog("Screen start address:0x%08X, buffer width:%d, swivel view:%d degrees\n", screenStartAddress, lineSize, rotation);
         //debugLog("Screen format, color:%s, BPP:%d\n", boolString(color), bitDepth);

         if(pictureInPictureEnabled){
            uint16_t pipStartX = sed1376Registers[PIP_X_START_1] << 8 | sed1376Registers[PIP_X_START_0];
            uint16_t pipStartY = sed1376Registers[PIP_Y_START_1] << 8 | sed1376Registers[PIP_Y_START_0];
            uint16_t pipEndX = (sed1376Registers[PIP_X_END_1] << 8 | sed1376Registers[PIP_X_END_0]) + 1;
            uint16_t pipEndY = (sed1376Registers[PIP_Y_END_1] << 8 | sed1376Registers[PIP_Y_END_0]) + 1;
            if(rotation == 0 || rotation == 180){
               pipStartX *= 32 / bitDepth;
               pipEndX *= 32 / bitDepth;
            }
            else{
               pipStartY *= 32 / bitDepth;
               pipEndY *= 32 / bitDepth;
            }
            //debugLog("PIP state, start x:%d, end x:%d, start y:%d, end y:%d\n", pipStartX, pipEndX, pipStartY, pipEndY);
            //render PIP only if PIP window is onscreen
            if(pipStartX < 160 && pipStartY < 160){
               pipEndX = uMin(pipEndX, 160);
               pipEndY = uMin(pipEndX, 160);
               screenStartAddress = getPipStartAddress();
               lineSize = (sed1376Registers[PIP_LINE_SZ_1] << 8 | sed1376Registers[PIP_LINE_SZ_0]) * 4;
               MULTITHREAD_DOUBLE_LOOP for(uint16_t pixelY = pipStartY; pixelY < pipEndY; pixelY++)
                  for(uint16_t pixelX = pipStartX; pixelX < pipEndX; pixelX++)
                     palmFramebuffer[pixelY * 160 + pixelX] = renderPixel(pixelX, pixelY);
            }
         }

         //rotation
         //later, unemulated

         //display inversion
         if((sed1376Registers[DISP_MODE] & 0x30) == 0x10)
            MULTITHREAD_LOOP for(uint32_t count = 0; count < 160 * 160; count++)
               palmFramebuffer[count] = ~palmFramebuffer[count];

         //backlight level, 0 = 1/4 color intensity, 1 = 1/2 color intensity, 2 = full color intensity
         switch(palmMisc.backlightLevel){
            case 0:
               MULTITHREAD_LOOP for(uint32_t count = 0; count < 160 * 160; count++){
                  palmFramebuffer[count] >>= 2;
                  palmFramebuffer[count] &= 0x39E7;
               }
               break;
            case 1:
               MULTITHREAD_LOOP for(uint32_t count = 0; count < 160 * 160; count++){
                  palmFramebuffer[count] >>= 1;
                  palmFramebuffer[count] &= 0x7BEF;
               }
               break;
            case 2:
               //nothing
               break;
         }
      }
      else{
         debugLog("Invalid screen format, color:%s, BPP:%d, rotation:%d\n", boolString(color), bitDepth, rotation);
      }
   }
   else{
      //black screen
      memset(palmFramebuffer, 0x00, 160 * 160 * sizeof(uint16_t));
      //debugLog("Cant draw screen, LCD on:%s, PLL on:%s, power save on:%s, forced blank on:%s\n", boolString(palmMisc.lcdOn), boolString(pllIsOn()), boolString(sed1376PowerSaveEnabled()), boolString(sed1376Registers[DISP_MODE] & 0x80));
   }
}
