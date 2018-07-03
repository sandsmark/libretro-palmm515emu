/*
ROM:10081164 PrvReadnBB7846PwrDn:                    ; CODE XREF: HwrADC+78↓p
ROM:10081164                                         ; HwrADC+CA↓p ...
ROM:10081164
ROM:10081164 arg_0           =  8
ROM:10081164 arg_2           =  $A
ROM:10081164
ROM:10081164                 link    a6,#0           ; Link and Allocate
ROM:10081168                 movem.l d3-d6/a2,-(sp)  ; Move Multiple Registers
ROM:1008116C                 move.w  arg_0(a6),d6    ; Move Data from Source to Destination
ROM:10081170                 movea.l arg_2(a6),a2    ; Move Address
ROM:10081174                 cmpi.w  #4,d6           ; Compare Immediate
ROM:10081178                 bls.s   loc_10081190    ; Branch if Low or Same
ROM:1008117A                 moveq   #0,d3           ; Move Quick
ROM:1008117C                 bra.s   loc_10081186    ; Branch Always
ROM:1008117E ; ---------------------------------------------------------------------------
ROM:1008117E
ROM:1008117E loc_1008117E:                           ; CODE XREF: PrvReadnBB7846PwrDn+26↓j
ROM:1008117E                 clr.w   2(a2)           ; Clear an Operand
ROM:10081182                 addq.w  #1,d3           ; Add Quick
ROM:10081184                 addq.l  #6,a2           ; Add Quick
ROM:10081186
ROM:10081186 loc_10081186:                           ; CODE XREF: PrvReadnBB7846PwrDn+18↑j
ROM:10081186                 cmpi.w  #4,d3           ; Compare Immediate
ROM:1008118A                 bcs.s   loc_1008117E    ; Branch if Carry Set
ROM:1008118C                 bra.w   loc_10081266    ; Branch Always
ROM:10081190 ; ---------------------------------------------------------------------------
ROM:10081190
ROM:10081190 loc_10081190:                           ; CODE XREF: PrvReadnBB7846PwrDn+14↑j
ROM:10081190                 move.w  #$4244,($FFFFF802).w ; Move Data from Source to Destination
ROM:10081196                 move.w  (a2),d0         ; Move Data from Source to Destination
ROM:10081198                 lsr.w   #3,d0           ; Logical Shift Right
ROM:1008119A                 move.w  d0,($FFFFF800).w ; Move Data from Source to Destination
ROM:1008119E                 move.w  #$4344,($FFFFF802).w ; Move Data from Source to Destination
ROM:100811A4                 moveq   #0,d4           ; Move Quick
ROM:100811A6
ROM:100811A6 loc_100811A6:                           ; CODE XREF: PrvReadnBB7846PwrDn+4A↓j
ROM:100811A6                 move.w  ($FFFFF802).w,d0 ; Move Data from Source to Destination
ROM:100811AA                 andi.w  #$100,d0        ; AND Immediate
ROM:100811AE                 bne.s   loc_100811A6    ; Branch if Not Equal
ROM:100811B0                 move.w  ($FFFFF800).w,d0 ; Move Data from Source to Destination
ROM:100811B4
ROM:100811B4 loc_100811B4:                           ; CODE XREF: PrvReadnBB7846PwrDn+DE↓j
ROM:100811B4                 move.w  4(a2),d3        ; Move Data from Source to Destination
ROM:100811B8                 beq.s   loc_100811C2    ; Branch if Equal
ROM:100811BA                 move.w  d3,-(sp)        ; Move Data from Source to Destination
ROM:100811BC                 jsr     PrvADCDelay     ; Jump to Subroutine
ROM:100811C0                 addq.w  #2,sp           ; Add Quick
ROM:100811C2
ROM:100811C2 loc_100811C2:                           ; CODE XREF: PrvReadnBB7846PwrDn+54↑j
ROM:100811C2                 move.w  (a2),d5         ; Move Data from Source to Destination
ROM:100811C4                 move.w  d5,d3           ; Move Data from Source to Destination
ROM:100811C6                 andi.w  #7,d3           ; AND Immediate
ROM:100811CA                 move.w  d5,d0           ; Move Data from Source to Destination
ROM:100811CC                 andi.w  #8,d0           ; AND Immediate
ROM:100811D0                 bne.s   loc_100811DE    ; Branch if Not Equal
ROM:100811D2                 moveq   #$D,d0          ; Move Quick
ROM:100811D4                 lsl.w   d0,d3           ; Logical Shift Left
ROM:100811D6                 move.w  #$424F,($FFFFF802).w ; Move Data from Source to Destination
ROM:100811DC                 bra.s   loc_100811E8    ; Branch Always
ROM:100811DE ; ---------------------------------------------------------------------------
ROM:100811DE
ROM:100811DE loc_100811DE:                           ; CODE XREF: PrvReadnBB7846PwrDn+6C↑j
ROM:100811DE                 moveq   #9,d0           ; Move Quick
ROM:100811E0                 lsl.w   d0,d3           ; Logical Shift Left
ROM:100811E2                 move.w  #$424B,($FFFFF802).w ; Move Data from Source to Destination
ROM:100811E8
ROM:100811E8 loc_100811E8:                           ; CODE XREF: PrvReadnBB7846PwrDn+78↑j
ROM:100811E8                 move.w  d4,d0           ; Move Data from Source to Destination
ROM:100811EA                 addq.w  #1,d0           ; Add Quick
ROM:100811EC                 cmp.w   d6,d0           ; Compare
ROM:100811EE                 bcc.s   loc_100811FE    ; Branch if Carry CLear
ROM:100811F0                 move.w  6(a2),d0        ; Move Data from Source to Destination
ROM:100811F4                 lsr.w   #3,d0           ; Logical Shift Right
ROM:100811F6                 or.w    d3,d0           ; Inclusive-OR Logical
ROM:100811F8                 move.w  d0,($FFFFF800).w ; Move Data from Source to Destination
ROM:100811FC                 bra.s   loc_10081208    ; Branch Always
ROM:100811FE ; ---------------------------------------------------------------------------
ROM:100811FE
ROM:100811FE loc_100811FE:                           ; CODE XREF: PrvReadnBB7846PwrDn+8A↑j
ROM:100811FE                 move.w  d3,d0           ; Move Data from Source to Destination
ROM:10081200                 ori.w   #$1B,d0         ; Inclusive-OR
ROM:10081204                 move.w  d0,($FFFFF800).w ; Move Data from Source to Destination
ROM:10081208
ROM:10081208 loc_10081208:                           ; CODE XREF: PrvReadnBB7846PwrDn+98↑j
ROM:10081208                 ori.w   #$4340,($FFFFF802).w ; Inclusive-OR
ROM:1008120E
ROM:1008120E loc_1008120E:                           ; CODE XREF: PrvReadnBB7846PwrDn+B2↓j
ROM:1008120E                 move.w  ($FFFFF802).w,d0 ; Move Data from Source to Destination
ROM:10081212                 andi.w  #$100,d0        ; AND Immediate
ROM:10081216                 bne.s   loc_1008120E    ; Branch if Not Equal
ROM:10081218                 move.w  (a2),d0         ; Move Data from Source to Destination
ROM:1008121A                 andi.w  #8,d0           ; AND Immediate
ROM:1008121E                 bne.s   loc_1008122E    ; Branch if Not Equal
ROM:10081220                 move.w  ($FFFFF800).w,d0 ; Move Data from Source to Destination
ROM:10081224                 andi.w  #$FFF,d0        ; AND Immediate
ROM:10081228                 move.w  d0,2(a2)        ; Move Data from Source to Destination
ROM:1008122C                 bra.s   loc_1008123C    ; Branch Always
ROM:1008122E ; ---------------------------------------------------------------------------
ROM:1008122E
ROM:1008122E loc_1008122E:                           ; CODE XREF: PrvReadnBB7846PwrDn+BA↑j
ROM:1008122E                 move.w  ($FFFFF800).w,d0 ; Move Data from Source to Destination
ROM:10081232                 lsl.w   #4,d0           ; Logical Shift Left
ROM:10081234                 andi.w  #$FFF,d0        ; AND Immediate
ROM:10081238                 move.w  d0,2(a2)        ; Move Data from Source to Destination
ROM:1008123C
ROM:1008123C loc_1008123C:                           ; CODE XREF: PrvReadnBB7846PwrDn+C8↑j
ROM:1008123C                 addq.l  #6,a2           ; Add Quick
ROM:1008123E                 addq.w  #1,d4           ; Add Quick
ROM:10081240                 cmp.w   d6,d4           ; Compare
ROM:10081242                 bcs.w   loc_100811B4    ; Branch if Carry Set
ROM:10081246                 move.w  #4,($FFFFF800).w ; Move Data from Source to Destination
ROM:1008124C                 move.w  #$4342,($FFFFF802).w ; Move Data from Source to Destination
ROM:10081252
ROM:10081252 loc_10081252:                           ; CODE XREF: PrvReadnBB7846PwrDn+F6↓j
ROM:10081252                 move.w  ($FFFFF802).w,d0 ; Move Data from Source to Destination
ROM:10081256                 andi.w  #$100,d0        ; AND Immediate
ROM:1008125A                 bne.s   loc_10081252    ; Branch if Not Equal
ROM:1008125C                 move.w  ($FFFFF800).w,d0 ; Move Data from Source to Destination
ROM:10081260                 move.w  #$E000,($FFFFF802).w ; Move Data from Source to Destination
ROM:10081266
ROM:10081266 loc_10081266:                           ; CODE XREF: PrvReadnBB7846PwrDn+28↑j
ROM:10081266                 movem.l (sp)+,d3-d6/a2  ; Move Multiple Registers
ROM:1008126A                 unlk    a6              ; Unlink
ROM:1008126C                 rts                     ; Return from Subroutine
ROM:1008126C ; End of function PrvReadnBB7846PwrDn
ROM:1008126C
ROM:1008126C ; ---------------------------------------------------------------------------
ROM:1008126E                 dc.w $9350
ROM:10081270 aRvreadnbb7846p_0:dc.b 'rvReadnBB7846PwrDn',0
ROM:10081283                 align 4
*/

uint16_t PrvReadnBB7846PwrDn(uint16_t unk_int, uint16_t* unk_ptr){
   uint16_t count;
   
   if(unk_int <= 4){
      uint16_t spi2Data;
      
      //enable SPI2
      *((volatile uint16_t*)0xFFFFF802) = 0x4244;
      
      //send 5 bits
      *((volatile uint16_t*)0xFFFFF800) = unk_ptr[0] << 3;
      *((volatile uint16_t*)0xFFFFF802) = 0x4344;
      
      //busy wait for transfer to complete
      while(*((volatile uint16_t*)0xFFFFF802) & 0x0100);
      //set d0 to *((volatile uint16_t*)0xFFFFF800)
      
      //delay, reason unknown
      if(unk_ptr[2] != 0)
         PrvADCDelay(unk_ptr[2]);
      
      //may be swapped
      *((volatile uint16_t*)0xFFFFF802) = (unk_ptr[0] & 0x0008) ? 0x424B : 0x424F;
      
      //more here
      //d0 = d4;
      //d0++
      d0 = 1;
      //d6 is unk_int
      
      //more here
      
      //shift out 3 bits, guaranteed to be 0s
      *((volatile uint16_t*)0xFFFFF800) = 0x0004;
      *((volatile uint16_t*)0xFFFFF802) = 0x4342;
      
      //busy wait for transfer to complete
      while(*((volatile uint16_t*)0xFFFFF802) & 0x0100);
      
      //get value and quit
      spi2Data = *((volatile uint16_t*)0xFFFFF800);
      *((volatile uint16_t*)0xFFFFF802) = 0xE000;
      
      return spi2Data;
   }
   
   count = 0;
   while(count < 4){
      unk_ptr[1] = 0x0000;//clr.w 2(a2)
      count++;//addq #1, d3
      unk_ptr += 3;//addq #6, a2
   }
   
   //doesnt return properly
}
