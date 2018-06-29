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
void (*getActualValues)(PointType* dest) = (void (*)(PointType*))0x10081284;

void PrvBBGetXY(PointType* dest){
   getActualValues(dest);
}
