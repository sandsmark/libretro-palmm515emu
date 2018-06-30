/*
 ROM:100827CE ; =============== S U B R O U T I N E =======================================
 ROM:100827CE
 ROM:100827CE ; Attributes: bp-based frame
 ROM:100827CE
 ROM:100827CE sub_100827CE:                           ; CODE XREF: sub_10082802+B2↓p
 ROM:100827CE
 ROM:100827CE var_18          = -$18
 ROM:100827CE var_C           = -$C
 ROM:100827CE var_A           = -$A
 ROM:100827CE arg_0           =  8
 ROM:100827CE
 ROM:100827CE                 link    a6,#-$18        ; Link and Allocate
 ROM:100827D2                 move.l  a2,-(sp)        ; Move Data from Source to Destination
 ROM:100827D4                 movea.l arg_0(a6),a2    ; Move Address
 ROM:100827D8                 pea     var_18(a6)      ; Push Effective Address
 ROM:100827DC                 clr.w   -(sp)           ; Clear an Operand
 ROM:100827DE                 jsr     sub_10081284    ; Jump to Subroutine
 ROM:100827E2                 move.w  var_C(a6),(a2)  ; Move Data from Source to Destination
 ROM:100827E6                 move.w  var_A(a6),2(a2) ; Move Data from Source to Destination
 ROM:100827EC                 addq.w  #6,sp           ; Add Quick
 ROM:100827EE                 movea.l (sp)+,a2        ; Move Address
 ROM:100827F0                 unlk    a6              ; Unlink
 ROM:100827F2                 rts                     ; Return from Subroutine
 ROM:100827F2 ; End of function sub_100827CE
 ROM:100827F2
 ROM:100827F2 ; ---------------------------------------------------------------------------
 ROM:100827F4                 dc.b $8A
 ROM:100827F5 aPrvbbgetxy:    dc.b 'PrvBBGetXY'
*/
//stack = old a2(0x4), a6 + 0x18(0x4), 0x0000(0x2), uint16_t[7] ADC values(0xE)

//writes all ADC values(7 * sizeof(uint16_t)) to to the uint16_t buffer
void (*HwrADC)(uint16_t* dest, uint16_t/*could also be Boolean or uint8_t*/ params) = (void (*)(uint16_t*, uint16_t))0x10081284;

void PrvBBGetXY(PointType* dest){
   uint16_t values[7];
   HwrADC(values, 0x0000);
   //this going on the assumption that structs store their data with the first variable in the highest address growing down, could be wrong
   //if true this storage method is the oppisite of how the stack grows
   *dest.x = values[6];
   *dest.y = values[5];
}
