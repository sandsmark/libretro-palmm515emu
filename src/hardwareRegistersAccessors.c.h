#pragma once

//declare I/O port functions in advance
static uint8_t getPortAValue();
static uint8_t getPortBValue();
static uint8_t getPortCValue();
static uint8_t getPortDValue();
static uint8_t getPortEValue();
static uint8_t getPortFValue();
static uint8_t getPortGValue();
static uint8_t getPortJValue();
static uint8_t getPortKValue();
static uint8_t getPortMValue();

//basic accessors
static uint8_t registerArrayRead8(uint32_t address){return BUFFER_READ_8(palmReg, address, 0xFFF);}
static uint16_t registerArrayRead16(uint32_t address){return BUFFER_READ_16(palmReg, address, 0xFFF);}
static uint32_t registerArrayRead32(uint32_t address){return BUFFER_READ_32(palmReg, address, 0xFFF);}
static void registerArrayWrite8(uint32_t address, uint8_t value){BUFFER_WRITE_8(palmReg, address, 0xFFF, value);}
static void registerArrayWrite16(uint32_t address, uint16_t value){BUFFER_WRITE_16(palmReg, address, 0xFFF, value);}
static void registerArrayWrite32(uint32_t address, uint32_t value){BUFFER_WRITE_32(palmReg, address, 0xFFF, value);}

//interrupt setters
static void setIprIsrBit(uint32_t interruptBit){
   //allows for setting an interrupt with masking by IMR and logging in IPR
   uint32_t newIpr = registerArrayRead32(IPR) | interruptBit;
   registerArrayWrite32(IPR, newIpr);
   registerArrayWrite32(ISR, newIpr & ~registerArrayRead32(IMR));
}

static void clearIprIsrBit(uint32_t interruptBit){
   uint32_t newIpr = registerArrayRead32(IPR) & ~interruptBit;
   registerArrayWrite32(IPR, newIpr);
   registerArrayWrite32(ISR, newIpr & ~registerArrayRead32(IMR));
}

//SPI1 FIFO accessors
static uint16_t spi1RxFifoRead(){
   uint16_t value = spi1RxFifo[spi1RxReadPosition];
   spi1RxReadPosition = (spi1RxReadPosition + 1) % 9;
   debugLog("reading from spirx1 fifo: %x\n", value);
   return value;
}

static void spi1RxFifoWrite(uint16_t value){
   debugLog("writing to spirx1 fifo: %x\n", value);
   spi1RxFifo[spi1RxWritePosition] = value;
   spi1RxWritePosition = (spi1RxWritePosition + 1) % 9;
}

static uint8_t spi1RxFifoEntrys(){
   //check for wraparound
   if(spi1RxWritePosition < spi1RxReadPosition)
      return spi1RxWritePosition + 9 - spi1RxReadPosition;
   return spi1RxWritePosition - spi1RxReadPosition;
}

static uint16_t spi1TxFifoRead(){
   uint16_t value = spi1TxFifo[spi1TxReadPosition];
   spi1TxReadPosition = (spi1TxReadPosition + 1) % 9;
   debugLog("reading from spitx1 fifo: %x\n", value);
   return value;
}

static void spi1TxFifoWrite(uint16_t value){
   debugLog("writing to spitx1 fifo: %x\n", value);
   spi1TxFifo[spi1TxWritePosition] = value;
   spi1TxWritePosition = (spi1TxWritePosition + 1) % 9;
}

static uint8_t spi1TxFifoEntrys(){
   //check for wraparound
   if(spi1TxWritePosition < spi1TxReadPosition)
      return spi1TxWritePosition + 9 - spi1TxReadPosition;
   return spi1TxWritePosition - spi1TxReadPosition;
}

//PWM1 FIFO accessors
static uint8_t pwm1FifoEntrys(){
   //check for wraparound
   if(pwm1WritePosition < pwm1ReadPosition)
      return pwm1WritePosition + 6 - pwm1ReadPosition;
   return pwm1WritePosition - pwm1ReadPosition;
}

int32_t pwm1FifoRunSample(int32_t now, int32_t clockOffset){
   uint8_t sample = pwm1Fifo[pwm1ReadPosition];
   uint16_t period = registerArrayRead8(PWMP1) + 2;
   uint16_t pwmc1 = registerArrayRead16(PWMC1);
   bool usingClk32 = pwmc1 & 0x8000;
   uint8_t prescaler = (pwmc1 >> 8 & 0x7F) + 1;
   uint8_t clockDivider = 2 << (pwmc1 & 0x03);
   uint8_t repeat = 1 << (pwmc1 >> 2 & 0x03);
   int32_t audioNow = now + clockOffset;
   int32_t audioSampleDuration = usingClk32 ? audioGetFramePercentIncrementFromClk32s(period * prescaler * clockDivider) : audioGetFramePercentIncrementFromSysclks(period * prescaler * clockDivider);
   double dutyCycle = dMin((double)sample / period, 1.00);

   for(uint8_t index = 0; index < repeat; index++){
      inductorPwmDutyCycle(audioNow, audioSampleDuration, dutyCycle);
      audioNow += audioSampleDuration;
   }

   //remove used entry
   if(pwm1FifoEntrys() > 0)
      pwm1ReadPosition = (pwm1ReadPosition + 1) % 6;

   //check for interrupt
   if(pwm1FifoEntrys() < 2){
      //trigger interrupt if enabled
      if(pwmc1 & 0x0040)
         setIprIsrBit(INT_PWM1);
      //checkInterrupts() is run when the clock that called this function is finished

      registerArrayWrite16(PWMC1, pwmc1 | 0x0080);//set IRQ bit
   }

   return audioSampleDuration * repeat;
}

static void pwm1FifoWrite(uint8_t value){
   if(pwm1FifoEntrys() < 5){
      pwm1WritePosition = (pwm1WritePosition + 1) % 6;
      pwm1Fifo[pwm1WritePosition] = value;
   }
}

static void pwm1FifoFlush(){
   pwm1ReadPosition = pwm1WritePosition;
}

//register setters
static void setCsa(uint16_t value){
   chips[CHIP_A0_ROM].enable = value & 0x0001;
   chips[CHIP_A0_ROM].readOnly = value & 0x8000;
   chips[CHIP_A0_ROM].lineSize = 0x20000/*128kb*/ << (value >> 1 & 0x0007);

   //CSA is now just a normal chip select
   if(chips[CHIP_A0_ROM].enable && chips[CHIP_A0_ROM].inBootMode)
      chips[CHIP_A0_ROM].inBootMode = false;

   chips[CHIP_A1_USB].enable = chips[CHIP_A0_ROM].enable;
   chips[CHIP_A1_USB].readOnly = chips[CHIP_A0_ROM].readOnly;
   chips[CHIP_A1_USB].start = chips[CHIP_A0_ROM].start + chips[CHIP_A0_ROM].lineSize;
   chips[CHIP_A1_USB].lineSize = chips[CHIP_A0_ROM].lineSize;

   registerArrayWrite16(CSA, value & 0x81FF);
}

static void setCsb(uint16_t value){
   uint16_t csControl1 = registerArrayRead16(CSCTRL1);

   chips[CHIP_B0_SED].enable = value & 0x0001;
   chips[CHIP_B0_SED].readOnly = value & 0x8000;
   chips[CHIP_B0_SED].lineSize = 0x20000/*128kb*/ << (value >> 1 & 0x0007);

   //attributes
   chips[CHIP_B0_SED].supervisorOnlyProtectedMemory = value & 0x4000;
   chips[CHIP_B0_SED].readOnlyForProtectedMemory = value & 0x2000;
   if(csControl1 & 0x4000 && csControl1 & 0x0001)
      chips[CHIP_B0_SED].unprotectedSize = chips[CHIP_B0_SED].lineSize / (1 << 7 - ((value >> 11 & 0x0003) | 0x0004));
   else
      chips[CHIP_B0_SED].unprotectedSize = chips[CHIP_B0_SED].lineSize / (1 << 7 - (value >> 11 & 0x0003));

   registerArrayWrite16(CSB, value & 0xF9FF);
}

static void setCsd(uint16_t value){
   uint16_t csControl1 = registerArrayRead16(CSCTRL1);

   chips[CHIP_DX_RAM].enable = value & 0x0001;
   chips[CHIP_DX_RAM].readOnly = value & 0x8000;
   if(csControl1 & 0x0040 && value & 0x0200)
      chips[CHIP_DX_RAM].lineSize = 0x800000/*8mb*/ << (value >> 1 & 0x0001);
   else
      chips[CHIP_DX_RAM].lineSize = 0x8000/*32kb*/ << (value >> 1 & 0x0007);

   //attributes
   chips[CHIP_DX_RAM].supervisorOnlyProtectedMemory = value & 0x4000;
   chips[CHIP_DX_RAM].readOnlyForProtectedMemory = value & 0x2000;
   if(csControl1 & 0x4000 && csControl1 & 0x0010)
      chips[CHIP_DX_RAM].unprotectedSize = chips[CHIP_DX_RAM].lineSize / (1 << 7 - ((value >> 11 & 0x0003) | 0x0004));
   else
      chips[CHIP_DX_RAM].unprotectedSize = chips[CHIP_DX_RAM].lineSize / (1 << 7 - (value >> 11 & 0x0003));

   registerArrayWrite16(CSD, value);
}

static void setCsgba(uint16_t value){
   uint16_t csugba = registerArrayRead16(CSUGBA);

   //add extra address bits if enabled
   if(csugba & 0x8000)
      chips[CHIP_A0_ROM].start = (csugba >> 12 & 0x0007) << 29 | value >> 1 << 14;
   else
      chips[CHIP_A0_ROM].start = value >> 1 << 14;

   chips[CHIP_A1_USB].start = chips[CHIP_A0_ROM].start + chips[CHIP_A0_ROM].lineSize;

   registerArrayWrite16(CSGBA, value & 0xFFFE);
}

static void setCsgbb(uint16_t value){
   uint16_t csugba = registerArrayRead16(CSUGBA);

   //add extra address bits if enabled
   if(csugba & 0x8000)
      chips[CHIP_B0_SED].start = (csugba >> 8 & 0x0007) << 29 | value >> 1 << 14;
   else
      chips[CHIP_B0_SED].start = value >> 1 << 14;

   registerArrayWrite16(CSGBB, value & 0xFFFE);
}

static void setCsgbd(uint16_t value){
   uint16_t csugba = registerArrayRead16(CSUGBA);

   //add extra address bits if enabled
   if(csugba & 0x8000)
      chips[CHIP_DX_RAM].start = (csugba & 0x0007) << 29 | value >> 1 << 14;
   else
      chips[CHIP_DX_RAM].start = value >> 1 << 14;

   registerArrayWrite16(CSGBD, value & 0xFFFE);
}

static void updateCsdAddressLines(){
   uint16_t dramc = registerArrayRead16(DRAMC);
   uint16_t sdctrl = registerArrayRead16(SDCTRL);

   if(registerArrayRead16(CSD) & 0x0200 && sdctrl & 0x8000 && dramc & 0x8000 && !(dramc & 0x0400)){
      //this register can remap address lines, that behavior is way too CPU intensive and complicated so only the "memory testing" and "correct" behavior is being emulated
      chips[CHIP_DX_RAM].mask = 0x003FFFFF;

      //address line 23 is enabled
      if((sdctrl & 0x000C) == 0x0008)
         chips[CHIP_DX_RAM].mask |= 0x00800000;

      //address line 22 is enabled
      if((sdctrl & 0x0030) == 0x0010)
         chips[CHIP_DX_RAM].mask |= 0x00400000;
   }
   else{
      //RAM is not enabled properly
      chips[CHIP_DX_RAM].mask = 0x00000000;
   }
}

static void setPllfsr(uint16_t value){
   uint16_t oldPllfsr = registerArrayRead16(PLLFSR);
   if(!(oldPllfsr & 0x4000)){
      //frequency protect bit not set
      registerArrayWrite16(PLLFSR, (value & 0x4FFF) | (oldPllfsr & 0x8000));//preserve CLK32 bit
      palmSysclksPerClk32 = sysclksPerClk32();
   }
}

static void setScr(uint8_t value){
   uint8_t oldScr = registerArrayRead8(SCR);
   uint8_t newScr = value & 0x1F;

   //preserve privilege violation, write protect violation and bus error timeout
   newScr |= oldScr & 0xE0;

   //clear violations on writing 1 to them
   newScr &= ~(value & 0xE0);

   chips[CHIP_REGISTERS].supervisorOnlyProtectedMemory = value & 0x08;

   registerArrayWrite8(SCR, newScr);//must be written before calling setRegisterFFFFAccessMode
   if((newScr & 0x04) != (oldScr & 0x04)){
      if(newScr & 0x04)
         setRegisterXXFFAccessMode();
      else
         setRegisterFFFFAccessMode();
   }
}

static void setIlcr(uint16_t value){
   uint16_t oldIlcr = registerArrayRead16(ILCR);
   uint16_t newIlcr = 0;

   //SPI1, interrupt level 0 an 7 are invalid values that cause the register not to update
   if((value & 0x7000) != 0x0000 && (value & 0x7000) != 0x7000)
      newIlcr |= value & 0x7000;
   else
      newIlcr |= oldIlcr & 0x7000;

   //UART2, interrupt level 0 an 7 are invalid values that cause the register not to update
   if((value & 0x0700) != 0x0000 && (value & 0x0700) != 0x0700)
      newIlcr |= value & 0x0700;
   else
      newIlcr |= oldIlcr & 0x0700;

   //PWM2, interrupt level 0 an 7 are invalid values that cause the register not to update
   if((value & 0x0070) != 0x0000 && (value & 0x0070) != 0x0070)
      newIlcr |= value & 0x0070;
   else
      newIlcr |= oldIlcr & 0x0070;

   //TMR2, interrupt level 0 an 7 are invalid values that cause the register not to update
   if((value & 0x0007) != 0x0000 && (value & 0x0007) != 0x0007)
      newIlcr |= value & 0x0007;
   else
      newIlcr |= oldIlcr & 0x0007;

   registerArrayWrite16(ILCR, newIlcr);
}

static void setSpiCont1(uint16_t value){
   //only master mode is implemented!!!
   uint16_t oldSpiCont1 = registerArrayRead16(SPICONT1);

   debugLog("SPI1 write, old value:0x%04X, value:0x%04X\n", oldSpiCont1, value);

   if(!(value & 0x0400) && value & 0x0200){
      //slave mode and enabled, mode is irrelevent fo SPI is off, dont know what to do
      debugLog("SPI1 set to slave mode, PC:0x%08X\n", flx68000GetPc());
   }

   //do a transfer
   if(value & oldSpiCont1 & 0x0200 && value & 0x0100){
      //enabled and exchange set
      uint8_t bitCount = (value & 0x000F) + 1;
      uint16_t startBit = 1 << (bitCount - 1);

      debugLog("SPI1 transfer, PC:0x%08X\n", flx68000GetPc());

      while(spi1TxFifoEntrys() > 0){
         uint16_t currentTxFifoEntry = spi1TxFifoRead();
         uint16_t newRxFifoEntry = 0;

         debugLog("reading from sd card %x\n", bitCount);
         for(uint8_t bits = 0; bits < bitCount; bits++){
            newRxFifoEntry |= sdCardExchangeBit(currentTxFifoEntry & startBit);
            newRxFifoEntry <<= 1;
            currentTxFifoEntry <<= 1;
         }

         spi1RxFifoWrite(newRxFifoEntry);

         //overflow occured, remove 1 FIFO entry
         //I do not currently know if the FIFO entry is removed from the back or front of the FIFO, going with the back for now
         if(spi1RxFifoEntrys() == 0)
            spi1RxFifoRead();
      }
      value = ~0x0100;
   }

   registerArrayWrite16(SPICONT1, value);
}

static void setSpiCont2(uint16_t value){
   //the ENABLE bit must be set before the transfer and in the transfer command
   //important bits are ENABLE, XCH, IRQ, IRQEN and BITCOUNT
   uint16_t oldSpiCont2 = registerArrayRead16(SPICONT2);
//   debugLog("SPIcont2 write, old value:0x%04X, value:0x%04X\n", oldSpiCont2, value);

   //force or clear an interrupt
   if((value & 0x00C0) == 0x00C0)
      setIprIsrBit(INT_SPI2);
   else
      clearIprIsrBit(INT_SPI2);

   //do a transfer
   if(value & oldSpiCont2 & 0x0200 && value & 0x0100){
      //enabled and exchange set
      uint8_t bitCount = (value & 0x000F) + 1;
      uint16_t startBit = 1 << (bitCount - 1);
      uint16_t spi2Data = registerArrayRead16(SPIDATA2);
      bool spiClk2Enabled = !(registerArrayRead8(PESEL) & 0x04);
      //uint16_t oldSpi2Data = spi2Data;

      //the input data is shifted into the unused bits if the transfer is less than 16 bits
      if(spiClk2Enabled){
         //shift in valid data
         for(uint8_t bits = 0; bits < bitCount; bits++){
            bool newBit = ads7846ExchangeBit(spi2Data & startBit);
            spi2Data <<= 1;
            spi2Data |= newBit;
         }
      }
      else{
         //shift in 0s
         spi2Data <<= bitCount;
      }
      registerArrayWrite16(SPIDATA2, spi2Data);

//      debugLog("SPI2 transfer, ENABLE:%s, XCH:%s, IRQ:%s, IRQEN:%s, BITCOUNT:%d\n", boolString(value & 0x0200), boolString(value & 0x0100), boolString(value & 0x0080), boolString(value & 0x0400), (value & 0x000F) + 1);
//      debugLog("SPI2 transfer, before:0x%04X, after:0x%04X, PC:0x%08X\n", oldSpi2Data, spi2Data, m68k_get_reg(NULL, M68K_REG_PPC));

      //unset XCH, transfers are instant since timing is not emulated
      value &= 0xFEFF;

      //acknowledge transfer finished, transfers are instant since timing is not emulated
      value |= 0x0080;

      //IRQEN set, send an interrupt after transfer
      if(value & 0x0040)
         setIprIsrBit(INT_SPI2);
   }

   //check for any interrupts from the transfer
   checkInterrupts();

   registerArrayWrite16(SPICONT2, value & 0xE3FF);
}

static void setTstat1(uint16_t value){
   uint16_t oldTstat1 = registerArrayRead16(TSTAT1);
   uint16_t newTstat1 = (value & timerStatusReadAcknowledge[0]) | (oldTstat1 & ~timerStatusReadAcknowledge[0]);

   //debugLog("TSTAT1 write, old value:0x%04X, new value:0x%04X, write value:0x%04X\n", oldTstat1, newTstat1, value);

   if(!(newTstat1 & 0x0001) && (oldTstat1 & 0x0001)){
      //debugLog("Timer 1 interrupt cleared.\n");
      clearIprIsrBit(INT_TMR1);
      checkInterrupts();
   }
   timerStatusReadAcknowledge[0] &= newTstat1;//clear acknowledged reads cleared bits
   registerArrayWrite16(TSTAT1, newTstat1);
}

static void setTstat2(uint16_t value){
   uint16_t oldTstat2 = registerArrayRead16(TSTAT2);
   uint16_t newTstat2 = (value & timerStatusReadAcknowledge[1]) | (oldTstat2 & ~timerStatusReadAcknowledge[1]);

   //debugLog("TSTAT2 write, old value:0x%04X, new value:0x%04X, write value:0x%04X\n", oldTstat2, newTstat2, value);

   if(!(newTstat2 & 0x0001) && (oldTstat2 & 0x0001)){
      //debugLog("Timer 2 interrupt cleared.\n");
      clearIprIsrBit(INT_TMR2);
      checkInterrupts();
   }
   timerStatusReadAcknowledge[1] &= newTstat2;//clear acknowledged reads for cleared bits
   registerArrayWrite16(TSTAT2, newTstat2);
}

static void setPwmc1(uint16_t value){
   uint16_t oldPwmc1 = registerArrayRead16(PWMC1);

   //dont allow manually setting FIFOAV
   value &= 0xFFDF;

   //PWM1 enabled, set IRQ
   if(!(oldPwmc1 & 0x0010) && value & 0x0010){
      value |= 0x0080;//enable IRQ
      pwm1ClocksToNextSample = 0;//when first sample is written output it immediatly
   }

   //clear interrupt by write(reading can also clear the interrupt)
   if(oldPwmc1 & 0x0080 && !(value & 0x0080)){
      clearIprIsrBit(INT_PWM1);
      checkInterrupts();
   }

   //interrupt enabled and interrupt set
   if((value & 0x00C0) == 0x00C0){
      //this register also allows forcing an interrupt by writing a 1 to its IRQ bit when IRQEN is enabled
      setIprIsrBit(INT_PWM1);
      checkInterrupts();
   }

   //PWM1 disabled
   if(oldPwmc1 & 0x0010 && !(value & 0x0010)){
      pwm1FifoFlush();
      value &= 0x80FF;//clear PWM prescaler value
   }

   registerArrayWrite16(PWMC1, value);
}

static void setIsr(uint32_t value, bool useTopWord, bool useBottomWord){
   //Palm OS uses this 32 bit register as 2 16 bit registers

   //prevent any internal hardware interrupts from being cleared
   value &= 0x000F0F00;//IRQ5 is always level triggered

   if(useTopWord){
      uint16_t interruptControlRegister = registerArrayRead16(ICR);

      if(!(interruptControlRegister & 0x0800)){
         //IRQ1 is not edge triggered
         value &= ~INT_IRQ1;
      }

      if(!(interruptControlRegister & 0x0400)){
         //IRQ2 is not edge triggered
         value &= ~INT_IRQ2;
      }

      if(!(interruptControlRegister & 0x0200)){
         //IRQ3 is not edge triggered
         value &= ~INT_IRQ3;
      }

      if(!(interruptControlRegister & 0x0100)){
         //IRQ6 is not edge triggered
         value &= ~INT_IRQ6;
      }

      registerArrayWrite16(IPR, registerArrayRead16(IPR) & ~(value >> 16));
      registerArrayWrite16(ISR, registerArrayRead16(ISR) & ~(value >> 16));
   }
   if(useBottomWord){
      uint8_t portDEdgeSelect = registerArrayRead8(PDIRQEG);

      registerArrayWrite16(IPR + 2, registerArrayRead16(IPR + 2) & ~(value & 0xFFFF & portDEdgeSelect << 8));
      registerArrayWrite16(ISR + 2, registerArrayRead16(ISR + 2) & ~(value & 0xFFFF & portDEdgeSelect << 8));
   }

   checkInterrupts();
}

//register getters
static uint8_t getPortDInputPinValues(){
   uint8_t requestedRow = ~getPortKValue();
   uint8_t portDInputValues = 0x00;

   // if this is set, it means the battery is dead, so don't
//   portDInputValues |= 0x80;//battery dead bit, dont know the proper level to set this

   // don't set this immediately, then it will immediately try to query it
   if(palmSdCard.flashChip.data) {
//      debugLog("have sd card\n");
      portDInputValues |= 0x20;
   } else {
//      debugLog("don't have sd card\n");
   }

   //kbd row 0
   if(requestedRow & 0x20)
      portDInputValues |= palmInput.buttonCalendar | palmInput.buttonAddress << 1 | palmInput.buttonTodo << 2 | palmInput.buttonNotes << 3;

   //kbd row 1
   if(requestedRow & 0x40)
      portDInputValues |= palmInput.buttonUp | palmInput.buttonDown << 1;

   //kbd row 2
   if(requestedRow & 0x80)
      portDInputValues |= palmInput.buttonPower | palmInput.buttonAddress << 3;

   //the port d pins state is high for false, invert them on return, Palm OS uses PDPOL to swap back to pressed == 1
   return ~portDInputValues;
}

static uint8_t getPortAValue(){
   //not attached, used as data lines
   return 0x00;
}

static uint8_t getPortBValue(){
   return ((registerArrayRead8(PBDATA) & registerArrayRead8(PBDIR)) | ~registerArrayRead8(PBDIR)) & registerArrayRead8(PBSEL);
}

static uint8_t getPortCValue(){
   //port c uses pull downs not pull ups
   return registerArrayRead8(PCDATA) & registerArrayRead8(PCDIR) & registerArrayRead8(PCSEL);
}

static uint8_t getPortDValue(){
   uint8_t portDValue = getPortDInputPinValues();
   uint8_t portDData = registerArrayRead8(PDDATA);
   uint8_t portDDir = registerArrayRead8(PDDIR);
   uint8_t portDPolarity = registerArrayRead8(PDPOL);
//      portDValue |= 0x20;

   portDValue ^= portDPolarity;//only input polarity is affected by PDPOL
   portDValue &= ~portDDir;//only use above pin values for inputs, port d allows using special function pins as inputs while active unlike other ports
   portDValue |= portDData & portDDir;//if a pin is an output and has its data bit set return that too

   debugLog("d: %x k value: %x\n", portDValue, ~getPortKValue());

   return portDValue;
}

static uint8_t getPortEValue(){
   return ((registerArrayRead8(PEDATA) & registerArrayRead8(PEDIR)) | ~registerArrayRead8(PEDIR)) & registerArrayRead8(PESEL);
}

static uint8_t getPortFValue(){
   uint8_t portFValue = 0x00;
   uint8_t portFData = registerArrayRead8(PKDATA);
   uint8_t portFDir = registerArrayRead8(PKDIR);
   uint8_t portFSel = registerArrayRead8(PKSEL);
   bool penIrqPin = !(ads7846PenIrqEnabled && palmInput.touchscreenTouched);//penIrqPin pulled low on touch

   portFValue |= penIrqPin << 1;
   portFValue |= 0x85;//bit 7 & 2-0 have pull ups, bits 6-3 have pull downs, bit 2 is occupied by PENIRQ
   portFValue &= ~portFDir & (portFSel | 0x02);//IRQ5 is readable even when PFSEL bit 2 is false
   portFValue |= portFData & portFDir & portFSel;

   return portFValue;
}

static uint8_t getPortGValue(){
   //port g only has 6 pins not 8
   uint8_t portGValue = 0x00;
   uint8_t portGData = registerArrayRead8(PGDATA);
   uint8_t portGDir = registerArrayRead8(PGDIR);
   uint8_t portGSel = registerArrayRead8(PGSEL);

   portGValue |= (palmMisc.backlightLevel > 0) << 1;
   portGValue |= 0x3D;//floating pins are high
   portGValue &= ~portGDir & portGSel;
   portGValue |= portGData & portGDir & portGSel;

   return portGValue;
}

static uint8_t getPortJValue(){
   return ((registerArrayRead8(PJDATA) & registerArrayRead8(PJDIR)) | ~registerArrayRead8(PJDIR)) & registerArrayRead8(PJSEL);
}

static uint8_t getPortKValue(){
   uint8_t portKValue = 0x00;
   uint8_t portKData = registerArrayRead8(PKDATA);
   uint8_t portKDir = registerArrayRead8(PKDIR);
   uint8_t portKSel = registerArrayRead8(PKSEL);

   portKValue |= !(palmMisc.dataPort == PORT_USB_CRADLE || palmMisc.dataPort == PORT_SERIAL_CRADLE) << 2;//true if charging
   portKValue |= 0xFB;//floating pins are high
   portKValue &= ~portKDir & portKSel;
   portKValue |= portKData & portKDir & portKSel;

   return portKValue;
}

static uint8_t getPortMValue(){
   //bit 5 has a pull up not pull down, bits 4-0 have a pull down, bit 7-6 are not active at all
   return ((registerArrayRead8(PMDATA) & registerArrayRead8(PMDIR)) | (~registerArrayRead8(PMDIR) & 0x20)) & registerArrayRead8(PMSEL);
}

static void samplePwm1(bool forClk32, double sysclks){
   //clear PWM1 FIFO values if enough time has passed
   uint16_t pwmc1 = registerArrayRead16(PWMC1);
   int32_t audioNow;
   int32_t audioClocks;

   //check if enabled and validate clock mode, CLK32 is used if PLL is disabled as the inductor still needs to settle
   if(forClk32 != (bool)(pwmc1 & 0x8000) && !(forClk32 && palmSysclksPerClk32 < 1.0))
      return;

   //these calculations are fairly heavy, only do them after we know the clock mode is valid
   audioNow = audioGetFramePercentage();
   audioClocks = forClk32 ? audioGetFramePercentIncrementFromClk32s(1) : audioGetFramePercentIncrementFromSysclks(sysclks);

   if(pwmc1 & 0x0010){
      //add cycles
      pwm1ClocksToNextSample -= audioClocks;

      //use samples
      while(pwm1ClocksToNextSample <= 0){
         //switch out samples until waiting(pwm1ClocksToNextSample is positive)
         //continue processing existing sample stream
         int32_t audioUsed = pwm1FifoRunSample(audioNow, pwm1ClocksToNextSample);
         pwm1ClocksToNextSample += audioUsed;
         audioNow += audioUsed;
      }
   }
   else{
      //PWM1 not enabled
      inductorPwmOff(audioNow, audioClocks);
   }
}

static uint16_t getPwmc1(){
   uint16_t returnValue = registerArrayRead16(PWMC1);

   //have FIFOAV set if a slot is available
   if(pwm1FifoEntrys() < 5)
      returnValue |= 0x0020;

   //clear INT_PWM1 if active
   if(returnValue & 0x0080){
      clearIprIsrBit(INT_PWM1);
      checkInterrupts();
      registerArrayWrite16(PWMC1, returnValue & 0xFF5F);
   }

   //the REPEAT field is write only
   returnValue &= 0xFFF3;

   return returnValue;
}

//updaters
static void updatePowerButtonLedStatus(){
   palmMisc.powerButtonLed = (bool)(getPortBValue() & 0x40) != palmMisc.batteryCharging;
}

static void updateVibratorStatus(){
   palmMisc.vibratorOn = getPortKValue() & 0x10;
}

static void updateAds7846ChipSelectStatus(){
   ads7846SetChipSelect(getPortGValue() & 0x04);
}

static void updateBacklightAmplifierStatus(){
   palmMisc.backlightLevel = (palmMisc.backlightLevel > 0) ? (1 + backlightAmplifierState()) : 0;
}

static void updateTouchState(){
   if(!(registerArrayRead8(PFSEL) & 0x02)){
      uint16_t icr = registerArrayRead16(ICR);
      bool penIrqPin = !(ads7846PenIrqEnabled && palmInput.touchscreenTouched);//penIrqPin pulled low on touch

      //switch polarity
      if(icr & 0x0080)
         penIrqPin = !penIrqPin;

      if(!penIrqPin)
         setIprIsrBit(INT_IRQ5);
      else
         clearIprIsrBit(INT_IRQ5);
   }
}
