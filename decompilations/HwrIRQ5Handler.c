/*
ROM:10083AC0 ; =============== S U B R O U T I N E =======================================
ROM:10083AC0
ROM:10083AC0 ; Attributes: bp-based frame
ROM:10083AC0
ROM:10083AC0 sub_10083AC0:                           ; DATA XREF: sub_10084132+28↓o
ROM:10083AC0
ROM:10083AC0 var_4           = -4
ROM:10083AC0
ROM:10083AC0                 link    a6,#-4          ; Link and Allocate
ROM:10083AC4                 move.l  d3,-(sp)        ; Move Data from Source to Destination
ROM:10083AC6                 tst.b   (byte_253).w    ; Test an Operand
ROM:10083ACA                 bne.s   loc_10083AF8    ; Branch if Not Equal
ROM:10083ACC                 pea     var_4(a6)       ; Push Effective Address
ROM:10083AD0                 trap    #$F             ; Trap sysTrapPenGetRawPen
ROM:10083AD0                 dc.w    $A270
ROM:10083AD4                 move.w  d0,d3           ; Move Data from Source to Destination
ROM:10083AD6                 addq.w  #4,sp           ; Add Quick
ROM:10083AD8                 bne.s   loc_10083AE4    ; Branch if Not Equal
ROM:10083ADA                 pea     var_4(a6)       ; Push Effective Address
ROM:10083ADE                 trap    #$F             ; Trap sysTrapEvtEnqueuePenPoint
ROM:10083ADE                 dc.w    $A126
ROM:10083AE2                 addq.w  #4,sp           ; Add Quick
ROM:10083AE4
ROM:10083AE4 loc_10083AE4:                           ; CODE XREF: sub_10083AC0+18↑j
ROM:10083AE4                 tst.b   (byte_253).w    ; Test an Operand
ROM:10083AE8                 beq.s   loc_10083AF8    ; Branch if Equal
ROM:10083AEA                 pea     (1).w           ; Push Effective Address
ROM:10083AEE                 move.l  (dword_14E).w,-(sp) ; Move Data from Source to Destination
ROM:10083AF2                 trap    #$F             ; Trap sysTrapSysTimerWrite
ROM:10083AF2                 dc.w    $A09A
ROM:10083AF6                 addq.w  #8,sp           ; Add Quick
ROM:10083AF8
ROM:10083AF8 loc_10083AF8:                           ; CODE XREF: sub_10083AC0+A↑j
ROM:10083AF8                                         ; sub_10083AC0+28↑j
ROM:10083AF8                 move.l  (sp)+,d3        ; Move Data from Source to Destination
ROM:10083AFA                 unlk    a6              ; Unlink
ROM:10083AFC                 rts                     ; Return from Subroutine
ROM:10083AFC ; End of function sub_10083AC0
ROM:10083AFC
ROM:10083AFC ; ---------------------------------------------------------------------------
ROM:10083AFE                 dc.b $8E
ROM:10083AFF aHwrirq5handler:dc.b 'HwrIRQ5Handler',0
ROM:10083B0E                 align $10
*/

void HwrIRQ5Handler(){
   if(*((volatile uint16_t*)0x00000253) != 0){
      //not done
   }
}
