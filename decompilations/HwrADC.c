/*
 ROM:10081284 ; =============== S U B R O U T I N E =======================================
 ROM:10081284
 ROM:10081284 ; Attributes: bp-based frame
 ROM:10081284
 ROM:10081284 sub_10081284:                           ; CODE XREF: sub_10081F56+14↓p
 ROM:10081284                                         ; sub_100820AE+C↓p ...
 ROM:10081284
 ROM:10081284 var_1E          = -$1E
 ROM:10081284 var_1C          = -$1C
 ROM:10081284 var_18          = -$18
 ROM:10081284 var_16          = -$16
 ROM:10081284 var_14          = -$14
 ROM:10081284 var_12          = -$12
 ROM:10081284 var_10          = -$10
 ROM:10081284 var_E           = -$E
 ROM:10081284 var_C           = -$C
 ROM:10081284 var_8           = -8
 ROM:10081284 var_6           = -6
 ROM:10081284 var_2           = -2
 ROM:10081284 arg_0           =  8
 ROM:10081284 arg_2           =  $A
 ROM:10081284
 ROM:10081284                 link    a6,#-$1E        ; Link and Allocate
 ROM:10081288                 movem.l d3-d7/a2-a4,-(sp) ; Move Multiple Registers
 ROM:1008128C                 moveq   #0,d6           ; Move Quick
 ROM:1008128E                 move    sr,d0           ; Move Data from Source to Destination
 ROM:10081290                 ori     #$700,sr        ; Inclusive-OR
 ROM:10081294                 move.w  d0,var_1E(a6)   ; Move Data from Source to Destination
 ROM:10081298                 movea.l arg_2(a6),a2    ; Move Address
 ROM:1008129C                 andi.b  #$FB,($FFFFF431).w ; AND Immediate
 ROM:100812A2                 ori.b   #1,($FFFFF420).w ; Inclusive-OR
 ROM:100812A8                 andi.b  #$FE,($FFFFF421).w ; AND Immediate
 ROM:100812AE                 andi.b  #$FE,($FFFFF422).w ; AND Immediate
 ROM:100812B4                 move.w  arg_0(a6),d0    ; Move Data from Source to Destination
 ROM:100812B8                 cmpi.w  #8,d0           ; switch 9 cases
 ROM:100812BC                 bhi.w   def_100812C6    ; jumptable 100812C6 default case
 ROM:100812C0                 add.w   d0,d0           ; Add
 ROM:100812C2                 move.w  jpt_100812C6(pc,d0.w),d0 ; Move Data from Source to Destination
 ROM:100812C6                 jmp     jpt_100812C6(pc,d0.w) ; switch jump
 ROM:100812C6 ; ---------------------------------------------------------------------------
 ROM:100812CA jpt_100812C6:   dc.w loc_100812DC-$100812CA
 ROM:100812CA                                         ; DATA XREF: sub_10081284+3E↑r
 ROM:100812CA                                         ; sub_10081284+42↑r
 ROM:100812CA                 dc.w loc_1008131A-$100812CA ; jump table for switch statement
 ROM:100812CA                 dc.w loc_10081372-$100812CA
 ROM:100812CA                 dc.w loc_100813E4-$100812CA
 ROM:100812CA                 dc.w loc_1008149E-$100812CA
 ROM:100812CA                 dc.w loc_100814C6-$100812CA
 ROM:100812CA                 dc.w loc_100814EE-$100812CA
 ROM:100812CA                 dc.w loc_100815AC-$100812CA
 ROM:100812CA                 dc.w loc_100816DE-$100812CA
 ROM:100812DC ; ---------------------------------------------------------------------------
 ROM:100812DC
 ROM:100812DC loc_100812DC:                           ; CODE XREF: sub_10081284+42↑j
 ROM:100812DC                                         ; DATA XREF: sub_10081284:jpt_100812C6↑o
 ROM:100812DC                 move.w  #$D9,var_18(a6) ; jumptable 100812C6 case 0
 ROM:100812E2                 move.w  #$99,var_12(a6) ; Move Data from Source to Destination
 ROM:100812E8                 move.w  #$14,var_14(a6) ; Move Data from Source to Destination
 ROM:100812EE                 move.w  #$14,var_E(a6)  ; Move Data from Source to Destination
 ROM:100812F4                 pea     var_18(a6)      ; Push Effective Address
 ROM:100812F8                 move.w  #2,-(sp)        ; Move Data from Source to Destination
 ROM:100812FC                 jsr     sub_10081164    ; Jump to Subroutine
 ROM:10081300                 move.w  var_16(a6),d0   ; Move Data from Source to Destination
 ROM:10081304                 lsr.w   #4,d0           ; Logical Shift Right
 ROM:10081306                 move.w  d0,$C(a2)       ; Move Data from Source to Destination
 ROM:1008130A                 move.w  var_10(a6),d0   ; Move Data from Source to Destination
 ROM:1008130E                 lsr.w   #4,d0           ; Logical Shift Right
 ROM:10081310                 move.w  d0,$E(a2)       ; Move Data from Source to Destination
 ROM:10081314                 addq.w  #6,sp           ; Add Quick
 ROM:10081316                 bra.w   loc_100816EE    ; Branch Always
 ROM:1008131A ; ---------------------------------------------------------------------------
 ROM:1008131A
 ROM:1008131A loc_1008131A:                           ; CODE XREF: sub_10081284+42↑j
 ROM:1008131A                                         ; DATA XREF: sub_10081284:jpt_100812C6↑o
 ROM:1008131A                 move.w  #$424B,($FFFFF802).w ; jumptable 100812C6 case 1
 ROM:10081320                 move.w  #$AE0,($FFFFF800).w ; Move Data from Source to Destination
 ROM:10081326                 move.w  #$434B,($FFFFF802).w ; Move Data from Source to Destination
 ROM:1008132C                 move.w  #$A7,var_18(a6) ; Move Data from Source to Destination
 ROM:10081332                 move.w  #$3C,var_14(a6) ; '<' ; Move Data from Source to Destination
 ROM:10081338
 ROM:10081338 loc_10081338:                           ; CODE XREF: sub_10081284+BC↓j
 ROM:10081338                 move.w  ($FFFFF802).w,d0 ; Move Data from Source to Destination
 ROM:1008133C                 andi.w  #$100,d0        ; AND Immediate
 ROM:10081340                 bne.s   loc_10081338    ; Branch if Not Equal
 ROM:10081342                 move.w  ($FFFFF800).w,d0 ; Move Data from Source to Destination
 ROM:10081346                 pea     var_18(a6)      ; Push Effective Address
 ROM:1008134A                 move.w  #1,-(sp)        ; Move Data from Source to Destination
 ROM:1008134E                 jsr     sub_10081164    ; Jump to Subroutine
 ROM:10081352                 move.w  var_16(a6),d4   ; Move Data from Source to Destination
 ROM:10081356                 move.w  d4,4(a2)        ; Move Data from Source to Destination
 ROM:1008135A                 move.w  d4,d3           ; Move Data from Source to Destination
 ROM:1008135C                 lsr.w   #3,d3           ; Logical Shift Right
 ROM:1008135E                 cmpi.w  #$FF,d3         ; Compare Immediate
 ROM:10081362                 addq.w  #6,sp           ; Add Quick
 ROM:10081364                 bls.s   loc_1008136A    ; Branch if Low or Same
 ROM:10081366                 move.w  #$FF,d3         ; Move Data from Source to Destination
 ROM:1008136A
 ROM:1008136A loc_1008136A:                           ; CODE XREF: sub_10081284+E0↑j
 ROM:1008136A                 move.w  d3,2(a2)        ; Move Data from Source to Destination
 ROM:1008136E                 bra.w   loc_100816EE    ; Branch Always
 ROM:10081372 ; ---------------------------------------------------------------------------
 ROM:10081372
 ROM:10081372 loc_10081372:                           ; CODE XREF: sub_10081284+42↑j
 ROM:10081372                                         ; DATA XREF: sub_10081284:jpt_100812C6↑o
 ROM:10081372                 moveq   #5,d4           ; jumptable 100812C6 case 2
 ROM:10081374                 move.w  #$EF,var_18(a6) ; Move Data from Source to Destination
 ROM:1008137A                 move.w  #$E7,var_12(a6) ; Move Data from Source to Destination
 ROM:10081380                 move.w  #$E7,var_C(a6)  ; Move Data from Source to Destination
 ROM:10081386                 clr.w   var_14(a6)      ; Clear an Operand
 ROM:1008138A                 lea     var_18(a6),a0   ; Load Effective Address
 ROM:1008138E                 addq.l  #8,a0           ; Add Quick
 ROM:10081390                 addq.l  #2,a0           ; Add Quick
 ROM:10081392                 movea.l a0,a3           ; Move Address
 ROM:10081394                 move.w  #$3C,(a0) ; '<' ; Move Data from Source to Destination
 ROM:10081398                 clr.w   var_8(a6)       ; Clear an Operand
 ROM:1008139C
 ROM:1008139C loc_1008139C:                           ; CODE XREF: sub_10081284+152↓j
 ROM:1008139C                 pea     var_18(a6)      ; Push Effective Address
 ROM:100813A0                 move.w  #3,-(sp)        ; Move Data from Source to Destination
 ROM:100813A4                 jsr     sub_10081164    ; Jump to Subroutine
 ROM:100813A8                 lea     var_18(a6),a0   ; Load Effective Address
 ROM:100813AC                 addq.l  #8,a0           ; Add Quick
 ROM:100813AE                 addq.l  #6,a0           ; Add Quick
 ROM:100813B0                 movea.l a0,a4           ; Move Address
 ROM:100813B2                 move.w  (a0),d3         ; Move Data from Source to Destination
 ROM:100813B4                 sub.w   var_10(a6),d3   ; Subtract
 ROM:100813B8                 tst.w   d3              ; Test an Operand
 ROM:100813BA                 addq.w  #6,sp           ; Add Quick
 ROM:100813BC                 bge.s   loc_100813C0    ; Branch if Greater or Equal
 ROM:100813BE                 neg.w   d3              ; Negate
 ROM:100813C0
 ROM:100813C0 loc_100813C0:                           ; CODE XREF: sub_10081284+138↑j
 ROM:100813C0                 cmpi.w  #$28,d3 ; '('   ; Compare Immediate
 ROM:100813C4                 blt.s   loc_100813D8    ; Branch if Less Than
 ROM:100813C6                 cmpi.w  #5,(a3)         ; Compare Immediate
 ROM:100813CA                 bls.s   loc_100813D0    ; Branch if Low or Same
 ROM:100813CC                 move.w  (a3),d0         ; Move Data from Source to Destination
 ROM:100813CE                 bra.s   loc_100813D2    ; Branch Always
 ROM:100813D0 ; ---------------------------------------------------------------------------
 ROM:100813D0
 ROM:100813D0 loc_100813D0:                           ; CODE XREF: sub_10081284+146↑j
 ROM:100813D0                 moveq   #5,d0           ; Move Quick
 ROM:100813D2
 ROM:100813D2 loc_100813D2:                           ; CODE XREF: sub_10081284+14A↑j
 ROM:100813D2                 add.w   d0,(a3)         ; Add
 ROM:100813D4                 subq.w  #1,d4           ; Subtract Quick
 ROM:100813D6                 bne.s   loc_1008139C    ; Branch if Not Equal
 ROM:100813D8
 ROM:100813D8 loc_100813D8:                           ; CODE XREF: sub_10081284+140↑j
 ROM:100813D8                 move.w  (a4),d0         ; Move Data from Source to Destination
 ROM:100813DA                 lsr.w   #4,d0           ; Logical Shift Right
 ROM:100813DC                 move.w  d0,4(a2)        ; Move Data from Source to Destination
 ROM:100813E0                 bra.w   loc_100816EE    ; Branch Always
 ROM:100813E4 ; ---------------------------------------------------------------------------
 ROM:100813E4
 ROM:100813E4 loc_100813E4:                           ; CODE XREF: sub_10081284+42↑j
 ROM:100813E4                                         ; DATA XREF: sub_10081284:jpt_100812C6↑o
 ROM:100813E4                 move.w  #$424B,($FFFFF802).w ; jumptable 100812C6 case 3
 ROM:100813EA                 move.w  #$AE0,($FFFFF800).w ; Move Data from Source to Destination
 ROM:100813F0                 move.w  #$434B,($FFFFF802).w ; Move Data from Source to Destination
 ROM:100813F6                 move.w  #$87,var_18(a6) ; Move Data from Source to Destination
 ROM:100813FC                 move.w  #$F7,var_12(a6) ; Move Data from Source to Destination
 ROM:10081402                 move.w  #$3C,var_14(a6) ; '<' ; Move Data from Source to Destination
 ROM:10081408                 clr.w   var_E(a6)       ; Clear an Operand
 ROM:1008140C
 ROM:1008140C loc_1008140C:                           ; CODE XREF: sub_10081284+190↓j
 ROM:1008140C                 move.w  ($FFFFF802).w,d0 ; Move Data from Source to Destination
 ROM:10081410                 andi.w  #$100,d0        ; AND Immediate
 ROM:10081414                 bne.s   loc_1008140C    ; Branch if Not Equal
 ROM:10081416                 move.w  ($FFFFF800).w,d0 ; Move Data from Source to Destination
 ROM:1008141A                 pea     var_18(a6)      ; Push Effective Address
 ROM:1008141E                 move.w  #2,-(sp)        ; Move Data from Source to Destination
 ROM:10081422                 jsr     sub_10081164    ; Jump to Subroutine
 ROM:10081426                 lea     var_18(a6),a0   ; Load Effective Address
 ROM:1008142A                 addq.l  #2,a0           ; Add Quick
 ROM:1008142C                 movea.l a0,a3           ; Move Address
 ROM:1008142E                 move.w  (a0),6(a2)      ; Move Data from Source to Destination
 ROM:10081432                 lea     var_18(a6),a0   ; Load Effective Address
 ROM:10081436                 addq.l  #8,a0           ; Add Quick
 ROM:10081438                 movea.l a0,a4           ; Move Address
 ROM:1008143A                 move.w  (a0),8(a2)      ; Move Data from Source to Destination
 ROM:1008143E                 moveq   #0,d3           ; Move Quick
 ROM:10081440                 move.w  (a3),d3         ; Move Data from Source to Destination
 ROM:10081442                 moveq   #0,d4           ; Move Quick
 ROM:10081444                 move.w  (a4),d4         ; Move Data from Source to Destination
 ROM:10081446                 move.l  d4,d5           ; Move Data from Source to Destination
 ROM:10081448                 sub.l   d3,d5           ; Subtract
 ROM:1008144A                 move.l  d5,d0           ; Move Data from Source to Destination
 ROM:1008144C                 move.l  #$9B8,d1        ; Move Data from Source to Destination
 ROM:10081452                 jsr     sub_1008052E    ; Jump to Subroutine
 ROM:10081456                 addi.l  #$800,d0        ; Add Immediate
 ROM:1008145C                 moveq   #$C,d1          ; Move Quick
 ROM:1008145E                 asr.l   d1,d0           ; Arithmetic Shift Right
 ROM:10081460                 move.l  d0,d4           ; Move Data from Source to Destination
 ROM:10081462                 move.l  d4,d0           ; Move Data from Source to Destination
 ROM:10081464                 move.l  #$A49,d1        ; Move Data from Source to Destination
 ROM:1008146A                 jsr     sub_1008052E    ; Jump to Subroutine
 ROM:1008146E                 move.l  #$3E8,d1        ; Move Data from Source to Destination
 ROM:10081474                 jsr     sub_100805BE    ; Jump to Subroutine
 ROM:10081478                 addi.l  #-$111,d0       ; Add Immediate
 ROM:1008147E                 move.l  d0,d3           ; Move Data from Source to Destination
 ROM:10081480                 cmpi.l  #$FFFF,d3       ; Compare Immediate
 ROM:10081486                 addq.w  #6,sp           ; Add Quick
 ROM:10081488                 ble.s   loc_10081490    ; Branch if Less or Equal
 ROM:1008148A                 move.l  #$FFFF,d3       ; Move Data from Source to Destination
 ROM:10081490
 ROM:10081490 loc_10081490:                           ; CODE XREF: sub_10081284+204↑j
 ROM:10081490                 tst.l   d3              ; Test an Operand
 ROM:10081492                 bge.s   loc_10081496    ; Branch if Greater or Equal
 ROM:10081494                 moveq   #0,d3           ; Move Quick
 ROM:10081496
 ROM:10081496 loc_10081496:                           ; CODE XREF: sub_10081284+20E↑j
 ROM:10081496                 move.w  d3,$A(a2)       ; Move Data from Source to Destination
 ROM:1008149A                 bra.w   loc_100816EE    ; Branch Always
 ROM:1008149E ; ---------------------------------------------------------------------------
 ROM:1008149E
 ROM:1008149E loc_1008149E:                           ; CODE XREF: sub_10081284+42↑j
 ROM:1008149E                                         ; DATA XREF: sub_10081284:jpt_100812C6↑o
 ROM:1008149E                 move.w  #$D9,var_18(a6) ; jumptable 100812C6 case 4
 ROM:100814A4                 move.w  #$14,var_14(a6) ; Move Data from Source to Destination
 ROM:100814AA                 pea     var_18(a6)      ; Push Effective Address
 ROM:100814AE                 move.w  #1,-(sp)        ; Move Data from Source to Destination
 ROM:100814B2                 jsr     sub_10081164    ; Jump to Subroutine
 ROM:100814B6                 move.w  var_16(a6),d0   ; Move Data from Source to Destination
 ROM:100814BA                 lsr.w   #4,d0           ; Logical Shift Right
 ROM:100814BC                 move.w  d0,$C(a2)       ; Move Data from Source to Destination
 ROM:100814C0                 addq.w  #6,sp           ; Add Quick
 ROM:100814C2                 bra.w   loc_100816EE    ; Branch Always
 ROM:100814C6 ; ---------------------------------------------------------------------------
 ROM:100814C6
 ROM:100814C6 loc_100814C6:                           ; CODE XREF: sub_10081284+42↑j
 ROM:100814C6                                         ; DATA XREF: sub_10081284:jpt_100812C6↑o
 ROM:100814C6                 move.w  #$99,var_18(a6) ; jumptable 100812C6 case 5
 ROM:100814CC                 move.w  #$14,var_14(a6) ; Move Data from Source to Destination
 ROM:100814D2                 pea     var_18(a6)      ; Push Effective Address
 ROM:100814D6                 move.w  #1,-(sp)        ; Move Data from Source to Destination
 ROM:100814DA                 jsr     sub_10081164    ; Jump to Subroutine
 ROM:100814DE                 move.w  var_16(a6),d0   ; Move Data from Source to Destination
 ROM:100814E2                 lsr.w   #4,d0           ; Logical Shift Right
 ROM:100814E4                 move.w  d0,$E(a2)       ; Move Data from Source to Destination
 ROM:100814E8                 addq.w  #6,sp           ; Add Quick
 ROM:100814EA                 bra.w   loc_100816EE    ; Branch Always
 ROM:100814EE ; ---------------------------------------------------------------------------
 ROM:100814EE
 ROM:100814EE loc_100814EE:                           ; CODE XREF: sub_10081284+42↑j
 ROM:100814EE                                         ; DATA XREF: sub_10081284:jpt_100812C6↑o
 ROM:100814EE                 move.w  #$D1,var_18(a6) ; jumptable 100812C6 case 6
 ROM:100814F4                 move.w  #$B1,var_12(a6) ; Move Data from Source to Destination
 ROM:100814FA                 move.w  #$C1,var_C(a6)  ; Move Data from Source to Destination
 ROM:10081500                 move.w  #$14,var_14(a6) ; Move Data from Source to Destination
 ROM:10081506                 move.w  #$14,var_E(a6)  ; Move Data from Source to Destination
 ROM:1008150C                 move.w  #$14,var_8(a6)  ; Move Data from Source to Destination
 ROM:10081512                 pea     var_18(a6)      ; Push Effective Address
 ROM:10081516                 move.w  #3,-(sp)        ; Move Data from Source to Destination
 ROM:1008151A                 jsr     sub_10081164    ; Jump to Subroutine
 ROM:1008151E                 lea     var_18(a6),a0   ; Load Effective Address
 ROM:10081522                 addq.l  #2,a0           ; Add Quick
 ROM:10081524                 movea.l a0,a3           ; Move Address
 ROM:10081526                 move.w  (a0),d0         ; Move Data from Source to Destination
 ROM:10081528                 lsr.w   #4,d0           ; Logical Shift Right
 ROM:1008152A                 move.w  d0,$C(a2)       ; Move Data from Source to Destination
 ROM:1008152E                 lea     var_18(a6),a0   ; Load Effective Address
 ROM:10081532                 addq.l  #8,a0           ; Add Quick
 ROM:10081534                 movea.l a0,a4           ; Move Address
 ROM:10081536                 move.w  (a0),d0         ; Move Data from Source to Destination
 ROM:10081538                 lsr.w   #4,d0           ; Logical Shift Right
 ROM:1008153A                 move.w  d0,$10(a2)      ; Move Data from Source to Destination
 ROM:1008153E                 lea     var_18(a6),a0   ; Load Effective Address
 ROM:10081542                 addq.l  #8,a0           ; Add Quick
 ROM:10081544                 addq.l  #6,a0           ; Add Quick
 ROM:10081546                 move.l  a0,d3           ; Move Data from Source to Destination
 ROM:10081548                 move.w  (a0),d0         ; Move Data from Source to Destination
 ROM:1008154A                 lsr.w   #4,d0           ; Logical Shift Right
 ROM:1008154C                 move.w  d0,$12(a2)      ; Move Data from Source to Destination
 ROM:10081550                 moveq   #0,d7           ; Move Quick
 ROM:10081552                 move.w  (a3),d7         ; Move Data from Source to Destination
 ROM:10081554                 moveq   #0,d4           ; Move Quick
 ROM:10081556                 move.w  (a4),d4         ; Move Data from Source to Destination
 ROM:10081558                 movea.l d4,a3           ; Move Address
 ROM:1008155A                 movea.l d3,a0           ; Move Address
 ROM:1008155C                 moveq   #0,d5           ; Move Quick
 ROM:1008155E                 move.w  (a0),d5         ; Move Data from Source to Destination
 ROM:10081560                 move.l  a3,d0           ; Move Data from Source to Destination
 ROM:10081562                 addq.w  #6,sp           ; Add Quick
 ROM:10081564                 beq.s   loc_10081592    ; Branch if Equal
 ROM:10081566                 cmp.l   d4,d5           ; Compare
 ROM:10081568                 bhi.s   loc_1008156E    ; Branch if High
 ROM:1008156A                 moveq   #0,d3           ; Move Quick
 ROM:1008156C                 bra.s   loc_10081598    ; Branch Always
 ROM:1008156E ; ---------------------------------------------------------------------------
 ROM:1008156E
 ROM:1008156E loc_1008156E:                           ; CODE XREF: sub_10081284+2E4↑j
 ROM:1008156E                 move.l  d5,d0           ; Move Data from Source to Destination
 ROM:10081570                 sub.l   d4,d0           ; Subtract
 ROM:10081572                 move.l  d7,d1           ; Move Data from Source to Destination
 ROM:10081574                 jsr     sub_1008052E    ; Jump to Subroutine
 ROM:10081578                 move.l  d4,d1           ; Move Data from Source to Destination
 ROM:1008157A                 jsr     sub_10080560    ; Jump to Subroutine
 ROM:1008157E                 lsr.l   #2,d0           ; Logical Shift Right
 ROM:10081580                 movea.l d0,a3           ; Move Address
 ROM:10081582                 move.l  d0,d3           ; Move Data from Source to Destination
 ROM:10081584                 cmpa.w  #$FFF,a3        ; Compare Address
 ROM:10081588                 bls.s   loc_10081598    ; Branch if Low or Same
 ROM:1008158A                 move.l  #$FFF,d3        ; Move Data from Source to Destination
 ROM:10081590                 bra.s   loc_10081598    ; Branch Always
 ROM:10081592 ; ---------------------------------------------------------------------------
 ROM:10081592
 ROM:10081592 loc_10081592:                           ; CODE XREF: sub_10081284+2E0↑j
 ROM:10081592                 move.l  #$FFF,d3        ; Move Data from Source to Destination
 ROM:10081598
 ROM:10081598 loc_10081598:                           ; CODE XREF: sub_10081284+2E8↑j
 ROM:10081598                                         ; sub_10081284+304↑j ...
 ROM:10081598                 move.l  #$FFF,d4        ; Move Data from Source to Destination
 ROM:1008159E                 sub.l   d3,d4           ; Subtract
 ROM:100815A0                 move.l  d4,d0           ; Move Data from Source to Destination
 ROM:100815A2                 lsr.l   #4,d0           ; Logical Shift Right
 ROM:100815A4                 move.w  d0,$14(a2)      ; Move Data from Source to Destination
 ROM:100815A8                 bra.w   loc_100816EE    ; Branch Always
 ROM:100815AC ; ---------------------------------------------------------------------------
 ROM:100815AC
 ROM:100815AC loc_100815AC:                           ; CODE XREF: sub_10081284+42↑j
 ROM:100815AC                                         ; DATA XREF: sub_10081284:jpt_100812C6↑o
 ROM:100815AC                 move.w  #$D1,var_18(a6) ; jumptable 100812C6 case 7
 ROM:100815B2                 move.w  #$91,var_12(a6) ; Move Data from Source to Destination
 ROM:100815B8                 move.w  #$B1,var_C(a6)  ; Move Data from Source to Destination
 ROM:100815BE                 move.w  #$C1,var_6(a6)  ; Move Data from Source to Destination
 ROM:100815C4                 move.w  #$14,var_14(a6) ; Move Data from Source to Destination
 ROM:100815CA                 move.w  #$14,var_E(a6)  ; Move Data from Source to Destination
 ROM:100815D0                 move.w  #$14,var_8(a6)  ; Move Data from Source to Destination
 ROM:100815D6                 move.w  #$14,var_2(a6)  ; Move Data from Source to Destination
 ROM:100815DC                 pea     var_18(a6)      ; Push Effective Address
 ROM:100815E0                 move.w  #4,-(sp)        ; Move Data from Source to Destination
 ROM:100815E4                 jsr     sub_10081164    ; Jump to Subroutine
 ROM:100815E8                 lea     var_18(a6),a0   ; Load Effective Address
 ROM:100815EC                 addq.l  #2,a0           ; Add Quick
 ROM:100815EE                 movea.l a0,a4           ; Move Address
 ROM:100815F0                 move.w  (a0),d0         ; Move Data from Source to Destination
 ROM:100815F2                 lsr.w   #4,d0           ; Logical Shift Right
 ROM:100815F4                 move.w  d0,$C(a2)       ; Move Data from Source to Destination
 ROM:100815F8                 lea     var_18(a6),a0   ; Load Effective Address
 ROM:100815FC                 addq.l  #8,a0           ; Add Quick
 ROM:100815FE                 move.l  a0,d3           ; Move Data from Source to Destination
 ROM:10081600                 move.w  (a0),d0         ; Move Data from Source to Destination
 ROM:10081602                 lsr.w   #4,d0           ; Logical Shift Right
 ROM:10081604                 move.w  d0,$E(a2)       ; Move Data from Source to Destination
 ROM:10081608                 lea     var_18(a6),a0   ; Load Effective Address
 ROM:1008160C                 addq.l  #8,a0           ; Add Quick
 ROM:1008160E                 addq.l  #6,a0           ; Add Quick
 ROM:10081610                 move.l  a0,d4           ; Move Data from Source to Destination
 ROM:10081612                 move.w  (a0),d0         ; Move Data from Source to Destination
 ROM:10081614                 lsr.w   #4,d0           ; Logical Shift Right
 ROM:10081616                 move.w  d0,$10(a2)      ; Move Data from Source to Destination
 ROM:1008161A                 lea     var_18(a6),a0   ; Load Effective Address
 ROM:1008161E                 lea     $14(a0),a0      ; Load Effective Address
 ROM:10081622                 move.l  a0,var_1C(a6)   ; Move Data from Source to Destination
 ROM:10081626                 move.w  (a0),d0         ; Move Data from Source to Destination
 ROM:10081628                 lsr.w   #4,d0           ; Logical Shift Right
 ROM:1008162A                 move.w  d0,$12(a2)      ; Move Data from Source to Destination
 ROM:1008162E                 moveq   #0,d0           ; Move Quick
 ROM:10081630                 move.w  (a4),d0         ; Move Data from Source to Destination
 ROM:10081632                 movea.l d0,a3           ; Move Address
 ROM:10081634                 movea.l d3,a0           ; Move Address
 ROM:10081636                 moveq   #0,d5           ; Move Quick
 ROM:10081638                 move.w  (a0),d5         ; Move Data from Source to Destination
 ROM:1008163A                 movea.l d4,a0           ; Move Address
 ROM:1008163C                 moveq   #0,d3           ; Move Quick
 ROM:1008163E                 move.w  (a0),d3         ; Move Data from Source to Destination
 ROM:10081640                 movea.l d3,a4           ; Move Address
 ROM:10081642                 movea.l var_1C(a6),a0   ; Move Address
 ROM:10081646                 moveq   #0,d7           ; Move Quick
 ROM:10081648                 move.w  (a0),d7         ; Move Data from Source to Destination
 ROM:1008164A                 move.l  a4,d0           ; Move Data from Source to Destination
 ROM:1008164C                 addq.w  #6,sp           ; Add Quick
 ROM:1008164E                 beq.s   loc_1008167C    ; Branch if Equal
 ROM:10081650                 cmp.l   d3,d7           ; Compare
 ROM:10081652                 bhi.s   loc_10081658    ; Branch if High
 ROM:10081654                 moveq   #0,d4           ; Move Quick
 ROM:10081656                 bra.s   loc_10081682    ; Branch Always
 ROM:10081658 ; ---------------------------------------------------------------------------
 ROM:10081658
 ROM:10081658 loc_10081658:                           ; CODE XREF: sub_10081284+3CE↑j
 ROM:10081658                 move.l  d7,d0           ; Move Data from Source to Destination
 ROM:1008165A                 sub.l   d3,d0           ; Subtract
 ROM:1008165C                 move.l  a3,d1           ; Move Data from Source to Destination
 ROM:1008165E                 jsr     sub_1008052E    ; Jump to Subroutine
 ROM:10081662                 move.l  d3,d1           ; Move Data from Source to Destination
 ROM:10081664                 jsr     sub_10080560    ; Jump to Subroutine
 ROM:10081668                 lsr.l   #2,d0           ; Logical Shift Right
 ROM:1008166A                 movea.l d0,a4           ; Move Address
 ROM:1008166C                 move.l  d0,d4           ; Move Data from Source to Destination
 ROM:1008166E                 cmpa.w  #$FFF,a4        ; Compare Address
 ROM:10081672                 bls.s   loc_10081682    ; Branch if Low or Same
 ROM:10081674                 move.l  #$FFF,d4        ; Move Data from Source to Destination
 ROM:1008167A                 bra.s   loc_10081682    ; Branch Always
 ROM:1008167C ; ---------------------------------------------------------------------------
 ROM:1008167C
 ROM:1008167C loc_1008167C:                           ; CODE XREF: sub_10081284+3CA↑j
 ROM:1008167C                 move.l  #$FFF,d4        ; Move Data from Source to Destination
 ROM:10081682
 ROM:10081682 loc_10081682:                           ; CODE XREF: sub_10081284+3D2↑j
 ROM:10081682                                         ; sub_10081284+3EE↑j ...
 ROM:10081682                 move.l  #$FFF,d7        ; Move Data from Source to Destination
 ROM:10081688                 sub.l   d4,d7           ; Subtract
 ROM:1008168A                 move.l  d7,d0           ; Move Data from Source to Destination
 ROM:1008168C                 lsr.l   #4,d0           ; Logical Shift Right
 ROM:1008168E                 move.w  d0,$14(a2)      ; Move Data from Source to Destination
 ROM:10081692                 tst.l   d3              ; Test an Operand
 ROM:10081694                 beq.s   loc_100816C6    ; Branch if Equal
 ROM:10081696                 move.l  #$1000,d0       ; Move Data from Source to Destination
 ROM:1008169C                 sub.l   d3,d0           ; Subtract
 ROM:1008169E                 move.l  a3,d1           ; Move Data from Source to Destination
 ROM:100816A0                 jsr     sub_1008052E    ; Jump to Subroutine
 ROM:100816A4                 move.l  d3,d1           ; Move Data from Source to Destination
 ROM:100816A6                 jsr     sub_10080560    ; Jump to Subroutine
 ROM:100816AA                 lsr.l   #3,d0           ; Logical Shift Right
 ROM:100816AC                 move.l  d0,d7           ; Move Data from Source to Destination
 ROM:100816AE                 lsr.l   #2,d5           ; Logical Shift Right
 ROM:100816B0                 move.l  d7,d3           ; Move Data from Source to Destination
 ROM:100816B2                 add.l   d5,d3           ; Add
 ROM:100816B4                 move.l  d3,d4           ; Move Data from Source to Destination
 ROM:100816B6                 cmpi.l  #$FFF,d4        ; Compare Immediate
 ROM:100816BC                 bls.s   loc_100816CC    ; Branch if Low or Same
 ROM:100816BE                 move.l  #$FFF,d3        ; Move Data from Source to Destination
 ROM:100816C4                 bra.s   loc_100816CC    ; Branch Always
 ROM:100816C6 ; ---------------------------------------------------------------------------
 ROM:100816C6
 ROM:100816C6 loc_100816C6:                           ; CODE XREF: sub_10081284+410↑j
 ROM:100816C6                 move.l  #$FFF,d3        ; Move Data from Source to Destination
 ROM:100816CC
 ROM:100816CC loc_100816CC:                           ; CODE XREF: sub_10081284+438↑j
 ROM:100816CC                                         ; sub_10081284+440↑j
 ROM:100816CC                 move.l  #$FFF,d4        ; Move Data from Source to Destination
 ROM:100816D2                 sub.l   d3,d4           ; Subtract
 ROM:100816D4                 move.l  d4,d0           ; Move Data from Source to Destination
 ROM:100816D6                 lsr.l   #4,d0           ; Logical Shift Right
 ROM:100816D8                 move.w  d0,$16(a2)      ; Move Data from Source to Destination
 ROM:100816DC                 bra.s   loc_100816EE    ; Branch Always
 ROM:100816DE ; ---------------------------------------------------------------------------
 ROM:100816DE
 ROM:100816DE loc_100816DE:                           ; CODE XREF: sub_10081284+42↑j
 ROM:100816DE                                         ; DATA XREF: sub_10081284:jpt_100812C6↑o
 ROM:100816DE                 move.w  #$DC,-(sp)      ; jumptable 100812C6 case 8
 ROM:100816E2                 jsr     sub_10081114    ; Jump to Subroutine
 ROM:100816E6                 addq.w  #2,sp           ; Add Quick
 ROM:100816E8                 bra.s   loc_100816EE    ; Branch Always
 ROM:100816EA ; ---------------------------------------------------------------------------
 ROM:100816EA
 ROM:100816EA def_100812C6:                           ; CODE XREF: sub_10081284+38↑j
 ROM:100816EA                 move.w  #$502,d6        ; jumptable 100812C6 default case
 ROM:100816EE
 ROM:100816EE loc_100816EE:                           ; CODE XREF: sub_10081284+92↑j
 ROM:100816EE                                         ; sub_10081284+EA↑j ...
 ROM:100816EE                 ori.b   #4,($FFFFF431).w ; Inclusive-OR
 ROM:100816F4                 move.w  var_1E(a6),d0   ; Move Data from Source to Destination
 ROM:100816F8                 move    d0,sr           ; Move Data from Source to Destination
 ROM:100816FA                 move.w  d6,d0           ; Move Data from Source to Destination
 ROM:100816FC                 movem.l (sp)+,d3-d7/a2-a4 ; Move Multiple Registers
 ROM:10081700                 unlk    a6              ; Unlink
 ROM:10081702                 rts                     ; Return from Subroutine
 ROM:10081702 ; End of function sub_10081284
 ROM:10081702
 ROM:10081702 ; ---------------------------------------------------------------------------
 ROM:10081704                 dc.b $86
 ROM:10081705 aHwradc:        dc.b 'HwrADC'
*/

uint16_t HwrADC(uint16_t mode, uint16_t* dataOut){
   *((volatile uint8_t*)0xFFFFF431) &= 0xFB;
   *((volatile uint8_t*)0xFFFFF420) |= 0x01;
   *((volatile uint8_t*)0xFFFFF421) &= 0xFE;
   *((volatile uint8_t*)0xFFFFF422) &= 0xFE;
   
   switch(mode){
      case 0:
         //returns touchscreen x(channel 6) and y(channel 7)
         break;
         
      case 1:
         //all 0x0000 execpt channel 1 which is the dock status and channel 2 which in unknown
         break;
         
      case 2:
         //all 0x0000 execpt channel 2 which is 0x001C when docked and 0x00FF when disconnected
         break;
         
      case 3:
         break;
         
      case 4:
         //only returns touchscreen x position(channel 6), all other values are 0x0000
         break;
         
      case 5:
         break;
         
      case 6:
         //caused "Fatal Execption"
         break;
         
      case 7:
         break;
         
      case 8:
         break;
         
      default:
         return 0x502;//sysErrParamErr
   }
      
   
   //alot more here
   
   *((volatile uint8_t*)0xFFFFF431) |= 0x04;
   return 0x0000;//errNone
}
