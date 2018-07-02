/*
ROM:10005D9C sub_10005D9C:                           ; CODE XREF: sub_10005F0C+45E↓p
ROM:10005D9C
ROM:10005D9C arg_0           =  8
ROM:10005D9C
ROM:10005D9C                 link    a6,#0           ; Link and Allocate
ROM:10005DA0                 movea.l #$FFFFF000,a0   ; Move Address
ROM:10005DA6                 move.w  #$4247,($FFFFF802).w ; Move Data from Source to Destination
ROM:10005DAC                 move.w  arg_0(a6),d0    ; Move Data from Source to Destination
ROM:10005DB0                 andi.w  #$FFFE,d0       ; AND Immediate
ROM:10005DB4                 move.w  d0,($FFFFF800).w ; Move Data from Source to Destination
ROM:10005DB8                 move.w  #$4347,($FFFFF802).w ; Move Data from Source to Destination
ROM:10005DBE
ROM:10005DBE loc_10005DBE:                           ; CODE XREF: sub_10005D9C+2A↓j
ROM:10005DBE                 move.w  $802(a0),d0     ; Move Data from Source to Destination
ROM:10005DC2                 andi.w  #$100,d0        ; AND Immediate
ROM:10005DC6                 bne.s   loc_10005DBE    ; Branch if Not Equal
ROM:10005DC8                 move.w  ($FFFFF800).w,d0 ; Move Data from Source to Destination
ROM:10005DCC                 move.w  #$E000,($FFFFF802).w ; Move Data from Source to Destination
ROM:10005DD2                 unlk    a6              ; Unlink
ROM:10005DD4                 rts                     ; Return from Subroutine
ROM:10005DD4 ; End of function sub_10005D9C
ROM:10005DD4
ROM:10005DD4 ; ---------------------------------------------------------------------------
ROM:10005DD6                 dc.w $9350
ROM:10005DD8 aRvsetbb7846cha:dc.b 'rvSetBB7846ChanRef',0
ROM:10005DEB                 align 4
*/

uint16_t PrvSetBB7846ChanRef(uint16_t channel){
   uint16_t returnValue;

   *((volatile uint16_t*)0xFFFFF802) = 0x4247;//enables SPI, required for the write to SPIDATA2 to go through
   *((volatile uint16_t*)0xFFFFF800) = channel & 0xFFFE;
   *((volatile uint16_t*)0xFFFFF802) = 0x4347;//transfer 8 bits
   while(*((volatile uint16_t*)0xFFFFF802) & 0x100);//busy wait for transfer to finish
   /*an interrupt occurs here from the transfer finishing if SPI2 is not masked in IMR*/
   returnValue = *((volatile uint16_t*)0xFFFFF800);
   *((volatile uint16_t*)0xFFFFF802) = 0xE000;//disable and set clock rate to lowest possible(don't know why since its off anyway)
   return returnValue;
}
