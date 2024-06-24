;/* $Header: /devel/sst2/Win95/dx/dd16/thunk16.asm 6     8/27/99 9:38a Andrew $ */
;/*
;** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
;** All Rights Reserved.
;**
;** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
;** the contents of this file may not be disclosed to third parties, copied or
;** duplicated in any form, in whole or in part, without the prior written
;** permission of 3Dfx Interactive, Inc.
;**
;** RESTRICTED RIGHTS LEGEND:
;** Use, duplication or disclosure by the Government is subject to restrictions
;** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
;** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
;** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
;** rights reserved under the Copyright Laws of the United States.
;**
;** File name:   thunk16.asm
;**
;** Description: Functions and macros to handle the "thunking" from
;**              the 16-bpp side (the GDI entrypoints) to the
;**              32-bpp driver side.
;**
;** $Revision: 6 $
;** $Date: 8/27/99 9:38a $
;**
;**
;*/
        ;//.MODEL SMALL, C, FARSTACK
        .MODEL LARGE
        .386p
VXDLDR_DEVICE_ID        EQU     27h
VXDLDR_LoadDevice       EQU     1
VXDLDR_UnloadDevice     EQU     2
CSIM_DEVICE_ID          EQU     0BEEFh  ; 0Ah

; STB Begin change
; STB-SR 12/16/98 Added for STB Performance optimizations
ifdef INCSTBPERF
INCLUDE ..\..\BUILD\STBPERF.INC
endif
; STB END change

DIB_BitBlt          PROTO   FAR PASCAL
DIB_ExtTextOut      PROTO   FAR PASCAL
DIB_Output          PROTO   FAR PASCAL
DIB_StretchDIBits   PROTO   FAR PASCAL
DIB_DibBltExt       PROTO   FAR PASCAL
DIB_DibToDevice     PROTO   FAR PASCAL
DIB_StretchBlt      PROTO   FAR PASCAL
DIB_BeginAccess     PROTO   FAR PASCAL
DIB_EndAccess     PROTO   FAR PASCAL
DIB_BitmapBits      PROTO   FAR PASCAL
;;DIBMoveHelp     PROTO FAR C

_THK16TEXT   SEGMENT WORD USE16 PUBLIC 'CODE'
_THK16TEXT   ENDS

_DATA   SEGMENT WORD USE16 PUBLIC 'DATA'
_DATA   ENDS

        EXTRN _lpDriverData:FAR PTR
        EXTRN _lph3IORegs:DWORD
        EXTRN _h3READ:FAR


;*----------------------------------------------------------------------
;Function name: RAMPAGE_SYNC
;
;Description:   Spin until HW is idle.
;
;Information:   This is a MACRO.
;
;Return:        VOID
;----------------------------------------------------------------------*

   RAMPAGE_SYNC MACRO
           local   DoRead, SkipRead
           push    ds
           lds     si, _lpDriverData ; Global Data
           lds     si, DWORD PTR [si]         ; Dib Engine of Main Device
           test    BYTE PTR [si+1Ch], BUSY  ; Is Device Busy
           pop     ds
           jnz     short SkipRead
           push    ebx
           push    esi
           mov     esi, _lph3IORegs
           mov     bx, wFlatDataSel
           mov     ds, bx
   DoRead:
           mov     ebx, [esi]
           test    bl, 1        ; bit 0 = SST2_BUSY
           jnz     short DoRead
           pop     esi
           pop     ebx
   SkipRead:
   ENDM

CURSOR_POINTS   equ     0001h
CURSOR_RECT     equ     0002h
CURSOR_SCAN     equ     0003h


;*----------------------------------------------------------------------
;Function name: DIBBEGINCURSOR
;
;Description:   Setup before the DibCursor call.
;
;Information:   This is a MACRO.
;               Surrounded by "#ifdef NOT_RIGHT_NOW".
;
;Return:        VOID
;----------------------------------------------------------------------*
DIBBEGINCURSOR MACRO lpDst:=<0>, lpSrc:=<0>, rect:=<0>, dX:=<0>, dY:=<0>, sX:=<0>, sY:=<0>, dwidth:=<0>, dheight:=<0>, swidth:=<0>, sheight:=<0>
        LOCAL   NoCall, PushSrc, PushDst
        mov     ax, @data
        mov     ds, ax
        lds     si, _lpDriverData
        test    BYTE PTR [si + 8], 08h          ;; HWC Exclusive ??
        jnz      short NoCall
        test    BYTE PTR [si + 9], 02h          ;; DibCursor On ?
        jz      short NoCall
        mov     bx, sp
        push    sheight
        push    swidth
        push    dheight
        push    dwidth
        push    sY                
        push    sX                
        push    dY                
        push    dX
        push    rect

        ;
        ; We only need to Exclude the Src/Dst if it's the lpPDevice that
        ; describes the whole screen
        ; If it ain't it don't matter
        ;
        mov     cx, lpSrc
        cmp     cx, 0                                   ; If zero
        jz      PushSrc                                 ; then just push
        mov     edx, ss:[bx+lpSrc]
        cmp     DWORD PTR [si], edx                     ; If it's lpPDevice
        jz      PushSrc                                 ; then just push
        xor     cx, cx                                  ; else it don't matter
PushSrc:
        push    cx

        mov     cx, lpDst
        cmp     cx, 0                                   ; If zero
        jz      PushDst                                 ; then just push
        mov     edx, ss:[bx+lpDst]
        cmp     DWORD PTR [si], edx                     ; If it's lpPDevice
        jz      PushDst                                 ; then just push
        xor     cx, cx                                  ; else it don't matter
PushDst:
        push    cx
        call    DibCursor
        add     sp, 22                
NoCall:
ENDM

CURSOREXCLUDE   equ     0000000000001000b


;*----------------------------------------------------------------------
;Function name: DIBENDCURSOR
;
;Description:   Preprocess before the dib EndAccess call.
;
;Information:   This is a MACRO.
;               Surrounded by "#ifdef NOT_RIGHT_NOW".
;
;Return:        VOID
;----------------------------------------------------------------------*
DIBENDCURSOR MACRO DstOff:=<0>, SrcOff:=<0>
        LOCAL  NoDst, NoSrc
        push    eax
        mov     bx, @data
        mov     ds, bx
        lds     si, _lpDriverData
        btr     WORD PTR [si + 8], 10
        jnc     short NoDst
        RAMPAGE_SYNC
        push    DWORD PTR [bp+DstOff]
        push    CURSOREXCLUDE
        call    DIB_EndAccess
;
;       This makes Mouse Trails + Winbench wimpy!!!!!
;
;;        mov     bx, @data
;;        mov     ds, bx
;;        call    DIBMoveHelp
NoDst:
        mov     bx, @data
        mov     ds, bx
        lds     si, _lpDriverData
        btr     WORD PTR [si + 8], 11
        jnc     short NoSrc
        RAMPAGE_SYNC
        push    DWORD PTR [bp+SrcOff]
        push    CURSOREXCLUDE
        call    DIB_EndAccess
NoSrc:
        pop     eax
ENDM

DIBENGINE           struc       ;                                      */ typedef struct {                        /*
  deType            dw  ?       ; contains TYPE_DIBENG or 0           ;*/ WORD         deType;                    /*
  deWidth           dw	?       ; Width of dib in pixels              ;*/ WORD         deWidth;                   /*
  deHeight          dw	?       ; Height of dib in pixels             ;*/ WORD         deHeight;                  /*
  deWidthBytes	    dw	?       ; #bytes per scan line                ;*/ WORD         deWidthBytes;              /*
  dePlanes          db	?       ; # of planes in bitmap               ;*/ BYTE         dePlanes;                  /*
  deBitsPixel	    db	?       ; # of bits per pixel                 ;*/ BYTE         deBitsPixel;               /*
  deReserved1	    dd	?       ; cannot be used.                     ;*/ DWORD        deReserved1;               /*
  deDeltaScan       dd  ?       ; + or -. Displacement to next scan.  ;*/ DWORD        deDeltaScan;               /*
  delpPDevice	    dd	?       ; Pointer to associated PDevice       ;*/ LPBYTE       delpPDevice;               /*
  deBits            df	?       ; fword offset to bits of dib         ;*/ DWORD        deBitsOffset;              /*
                                ;                                     ;*/ WORD         deBitsSelector;            /*
  deFlags           dw	?       ; additional flags                    ;*/ WORD         deFlags;                   /*
  deVersion         dw  ?       ; lsb=minor, msb=major (0400h = 4.0)  ;*/ WORD         deVersion;                 /*
  deBitmapInfo	    dd	?       ; pointer to the bitmapinfo header    ;*/ LPBITMAPINFO deBitmapInfo;              /*
  deBeginAccess     dd	?       ; Begin surface access call back      ;*/ void         (FAR *deBeginAccess)();    /*
  deEndAccess       dd	?       ; End surface access call back        ;*/ void         (FAR *deEndAccess)();      /*
  deDriverReserved  dd  ?       ; Reserved for Minidriver use.        ;*/ DWORD        deDriverReserved;          /*
DIBENGINE	    ends            ;                                      */ } DIBENGINE, FAR *LPDIBENGINE;          /*

OFFSCREEN       equ     0000000000001000b       ;offscreen surface (use with VRAM)
VRAM            equ     1000000000000000b       ;physical surface (video memory)
BUSY            equ     0000000000010000b       ;busy bit
PALETTE_XLAT    equ     0001000000000000b       ;set only in 8bpp modes
TYPE_DIBENG     equ     'RP'                    ;Dibengine


;*----------------------------------------------------------------------
;Function name: DEVBITCHECK
;
;Description:   Preprocess and do a _DoHostStage2 if a
;               valid device bitmap.
;
;Information:   This is a MACRO.
;
;Return:        VOID
;----------------------------------------------------------------------*
DEVBITCHECK Macro lpDev
        LOCAL   NoWork
        
        ; the reason we use [bp+lpDev+6+6] on the following line is
        ; because the BP is offset by the pushes in FN16ENTRY (3 words)
        ; as well as the push of BP and the far call return address
        ; this totals up to a 12 byte offset
        lds     si, [bp+lpDev+6+6]              ; get lpDev into DS:SI
        mov     ax, ds                          ; and immediately check
        or      ax, si                          ; for a NULL pointer
        jz      short NoWork                    ; do nothing on NULL
        
        assume  si:NEAR PTR DIBENGINE
        cmp     [si].deType, TYPE_DIBENG        ; is this a DIBENG device type
        jnz     short NoWork                    ; if not, do nothing
        mov     ax, [si].deFlags                ; if so, check the flags
        and     ax, VRAM OR OFFSCREEN           ; for device bitmap settings
        cmp     ax, VRAM OR OFFSCREEN           ; is it a device bitmap?
        jnz     short NoWork                    ; if not, do nothing
        
        ; The old DeviceBitCheck would now call _DoHostStage2
        ; so it could make sure the device bitmap was located properly
        ; I've encoded the trivial rejection checks from _DoHostStage2
        ; here to eliminate that call for ready device bitmaps
        
        lds     si, [si].deDriverReserved       ;   pDqueue = (PDQUEUE)lpPDevice->deDriverReserved; 
        assume  si:nothing
        mov     ax, ds
        or      ax, si
        jz      short NoWork                    ;   if (NULL != pDqueue) 
        
        ;;lds     si, [si+8]                    ;      pDevInfo = (PDEVINFO)pDqueue->pData; 
        mov     ax, [si+16+8]
        cmp     ax, 1
        jnz     short NoWork                    ;      if (DV_ONHOST == pDevInfo->wStatus)
        
        ; ok, this wasn't trivially rejected, which means we must let 
        ; _DoHostStage2 do his work
        lds     si, [bp+lpDev+6+6]
        push    di                              ; Save register
        push    es                              ; Save register
        push    ds                              ; Selector of lpPDevice
        push    si                              ; Offset of lpPDevice
        call    _DoHostStage2                   ; Move it so we can be rid of it
        pop     di                              ; clean stack
        pop     di                              ; clean stack
        pop     es                              ; restore register
        pop     di                              ; restore register
NoWork:
Endm

IFDEF STBPERF_CMBITBLT
;*----------------------------------------------------------------------
;Function name: HANDLECMBLT   ; PingZ 1/20/98
;
;Description:   Detects video to system blits whose destination is 1 bpp
;               In case this happens it evicts the cached bitmap to system
;               memory.  The reason to do this is that color to mono blit
;               is extremly slow if the source is in video memory.  We are
;               better off not to cache the bitmap.  Plus, it turns out that
;               WB99 reuses a lot of the same bitmaps for CM blit only.  Adding
;               this macro gained a minimum 2 points for 32-bit business
;
;Information:   This is a MACRO.
;
;Return:        VOID
;----------------------------------------------------------------------*
HANDLECMBLT Macro lpDst, lpSrc, PuntJump
        LOCAL   NoWork
        
        ; the reason we use [bp+lpDev+6+6] on the following line is
        ; because the BP is offset by the pushes in FN16ENTRY (3 words)
        ; as well as the push of BP and the far call return address
        ; this totals up to a 12 byte offset
        lds     si, [bp+lpDst+6+6]              ; get lpDev into DS:SI
        mov     ax, ds                          ; and immediately check
        or      ax, si                          ; for a NULL pointer
        jz      short NoWork                    ; do nothing on NULL
        
        assume  si:NEAR PTR DIBENGINE
        cmp     [si].deType, 0  
        jne     short NoWork                    ; if not, do nothing
        cmp     [si].deBitsPixel, 1
		jnz     short NoWork

        lds     si, [bp+lpSrc+6+6]              ; get lpDev into DS:SI
        mov     ax, ds                          ; and immediately check
        or      ax, si                          ; for a NULL pointer
        jz      short NoWork                    ; do nothing on NULL
        
        assume  si:NEAR PTR DIBENGINE
        cmp     [si].deType, TYPE_DIBENG        ; is this a DIBENG device type
        jnz     short NoWork                    ; if not, do nothing
        mov     ax, [si].deFlags                ; if so, check the flags
        and     ax, VRAM OR OFFSCREEN           ; for device bitmap settings
        cmp     ax, VRAM OR OFFSCREEN           ; is it a device bitmap?
        jnz     short NoWork                     ; if yes, do nothing
        cmp     [si].deBitsPixel, 16
		jl      short NoWork

        lds     si, [si].deDriverReserved       ;   pDqueue = (PDQUEUE)lpPDevice->deDriverReserved; 
        assume  si:nothing
        mov     ax, ds
        or      ax, si
        jz      short NoWork                    ;   if (NULL != pDqueue) 
        
        ;;lds     si, [si+8]                    ;      pDevInfo = (PDEVINFO)pDqueue->pData; 
        mov     ax, [si+16+8]
        cmp     ax, 2
        jnz     short NoWork                    ;      if (DV_OFFSCREEN == pDevInfo->wStatus)
        
        ; ok, this wasn't trivially rejected, which means we must let 
        ; _DoHostStage2 do his work
        lds     si, [bp+lpSrc+6+6]
        push    di                              ; Save register
        push    es                              ; Save register
        push    ds                              ; Selector of lpPDevice
        push    si                              ; Offset of lpPDevice
        call    _EvictBmp                       ; Move it so we can be rid of it
        pop     di                              ; clean stack
        pop     di                              ; clean stack
        pop     es                              ; restore register
        pop     di                              ; restore register
        jmp     PuntJump
NoWork:
Endm
ENDIF

;*----------------------------------------------------------------------
;Function name: FN16ENTRY
;
;Description:   Prolog to save registers and setup stack.
;
;Information:   This is a MACRO.
;
;Return:        VOID
;----------------------------------------------------------------------*
FN16ENTRY Macro
        ; save caller's DS, SI, and DI
        push    ds
        push    si
        push    di
        
        ; save caller's BP

        push    bp
        movzx   ebp, sp
Endm        


;*----------------------------------------------------------------------
;Function name: LOADDS
;
;Description:   Moves "@data" into DS.
;
;Information:   This is a MACRO.
;
;Return:        VOID
;----------------------------------------------------------------------*
LOADDS Macro
        ; load our DS

        mov     ax, @data
        mov     ds, ax
Endm


;*----------------------------------------------------------------------
;Function name: TRIVIAL_REJECT
;
;Description:   Attempt to reject on the 16-bit side before
;               thunking down to the 32-bit side.
;
;Information:   This is a MACRO.
;
;Return:        VOID
;----------------------------------------------------------------------*
ifndef STB_FASTER_TR
TRIVIAL_REJECT Macro lpDest, PuntJump
        lds     si, [bp+lpDest+6+6]             ; get lpDev into DS:SI
        assume  si:NEAR PTR DIBENGINE
        mov     dx, [si].deFlags
        and     dx, VRAM                        ; isolate VRAM bit
        xor     dx, VRAM                        ; and toggle the bit
        ; (we do the and then xor because we want to OR the value that
        ;  makes us want to punt, which is the inverse of the VRAM bit)
        mov     ax, @data
        mov     ds, ax
        lds     si, _lpDriverData
        lds     si, [si]        ; get lpPdevice
        mov     ax, [si].deFlags
        and     ax, BUSY OR PALETTE_XLAT        ; isolate these bits
        or      ax, dx                          ; pull in inverse of VRAM bit
        assume  si:nothing
        jnz     PuntJump
;       jmp     PuntJump                        ; !! SST2 -- change to punt

Endm                
else
TRIVIAL_REJECT Macro lpDest, PuntJump
		; No Need to test the PDevice BUSY flags
		; On VM change, all Device bitmaps
		; get flushed out and VRAM bits
		; get turned off
        lds     si, [bp+lpDest+6+6]             ; get lpDev into DS:SI
        mov     dx, [si].DIBENGINE.deFlags
        xor     dx, word ptr VRAM                        ; isolate VRAM bit
        test    dx, word ptr (VRAM + BUSY + PALETTE_XLAT)
        jnz     PuntJump
Endm                
endif

; STB Begin change
; STB-SR 12/16/98 Added this from the banshee source code
ifdef STBPERF_HANDLE1X1
;*----------------------------------------------------------------------
;Function name: HANDLE1X1
;
;Description:   Handles all 1 pixel copies by poking it 
;               out to the framebuffer
;
;Information:   This is a MACRO.
;
;Return:        VOID
;----------------------------------------------------------------------*
HANDLE1X1 Macro ExitJump
        ; Check initial cases
        cmp	    WORD PTR [bp+16h+6+6], 0        ; Is iScan == 0?
        jnz     NotSpecialCase
        cmp	    WORD PTR [bp+14h+6+6], 1        ; Is cScan == 1?
        jnz     NotSpecialCase
        cmp	    WORD PTR [bp+0h+6+6], 0         ; Is lpTranslate == NULL?
        jnz     NotSpecialCase

        lds	    si, [bp+04h+6+6]                ; load lpBitmapInfo into DS:SI

        cmp     WORD PTR ds:[si+0Eh], 8         ; Is src 8 bpp?
        jnz     NotSpecialCase

        cmp     DWORD PTR ds:[si+10h], 0        ; Is Src compression == BI_RGB?
        jnz     NotSpecialCase

        cmp     DWORD PTR ds:[si+08h], 1        ; is the Src Height == 1?
        jnz     NotSpecialCase

        mov     ecx, DWORD PTR  ds:[si+04h]     ; store src width in ecx

        cmp     DWORD PTR [bp+10h+6+6], 0       ; is lpClipRect == NULL?
        jz      SetDibClipDone

;       Clipping Check
        lds     si, DWORD PTR [bp+10h+6+6]      ; load lpClipRect into ds:si

        mov     ax, WORD PTR [bp+18h+6+6]       ; move Y into ax
        cmp     ax, WORD PTR ds:[si+2h]         ; is Y < Clip Rect top?
        jl      DstDIBDone                      ; Clipped top

        cmp     ax, WORD PTR ds:[si+6h]         ; is Y >= Clip Rect bottom?
        jge     DstDIBDone                      ; Clipped bottom

        xor     eax, eax
        mov     ax, WORD PTR [bp+1Ah+6+6]       ; move X into ax
		push    eax                             ; save X on stack for later use
        cmp     ax, WORD PTR ds:[si+4h]         ; is X >= Clip Rect right?
        jge     DstDIBDone                      ; Clipped Right

		add     ax, cx                          ; 
        cmp     ax, WORD PTR ds:[si+0h]         ; is X + width <= Clip Rect left?
        jle     DstDIBDone                      ; Clipped left

        cmp     cx, 1							; is the width == 1?
		je      SetDibClipDone                  

		mov     dx, WORD PTR ds:[si+4h]         ; get the Clip Rect right
		cmp     ax, dx                          ; is X + width <= Clip Rect Right?
		jle     @f
		sub     ax, dx							; ax has the distance from clip right to line right
		sub     cx, ax 							; adjust the line witdth
@@:     
        xor     edx, edx
        mov     ax, WORD PTR [bp+1Ah+6+6]       ; move X into ax
		mov     dx, WORD PTR ds:[si+0h]
        cmp     ax, dx                          ; is X >= Clip Rect left?
		jge     SetDibClipDone                  ; if X is to right of Clip.left then skip 
		pop     ebx    						    ; get rid of the currend X saved on stack
		push    edx                             ; save the new X on stack
		sub     dx, ax
		sub     cx, dx                          ; adjust the line width
     
SetDibClipDone:
        lds     si, [bp+1Ch+6+6]                ; load lpDst into ds:si
        cmp     WORD PTR ds:[si], 'RP'          ; make sure surface type is 'RP'
        jnz     NotSpecialCase

        cmp	    BYTE PTR ds:[si+09h], 32        ; is it 32-bit?
        je      Test32Dst

        cmp	    BYTE PTR ds:[si+09h], 16        ; is it 16-bit?
        je      Test16Dst

		jmp     NotSpecialCase                  ;only handle 16-bit and 32-bit mode for now

Test16Dst:
; Checking the busy bit in the processor
        mov     si, @data
        mov     ds, si
        lds     si, _lpDriverData ; Global Data
        mov     esi, _lph3IORegs
        mov     dx, wFlatDataSel
        mov     ds, dx
DoCpy16Read:
        mov     eax, [esi]
        test    al, 1        ; bit 0 = SST2_BUSY
        jnz     DoCpy16Read

        xor	    esi, esi
        xor	    edi, edi
        xor     eax, eax
;       calculate dest. offset

        lds     si, [bp+1Ch+6+6]                ; load lpDst into ds:si
        mov     ax, WORD PTR [bp+18h+6+6]       ; move Y into dx
        mov     edx, ds:[si+0Eh]                ; load displacement scan (stride)
        mul     edx                             ; eax = Y * Stride
        pop     edx                             ; get X from stack
        shl     edx, 1                          ; Mult by 2 for 2 Bytes per pixel (16 bpp)
        add     eax, edx                        ; eax holds pixel address offset from upper left corner in bytes
        les     edi, FWORD PTR ds:[si+16h]      ; load deBits into es:di
        add     edi, eax                        ; edi has the linear adress of the pixel from upper left corner in bytes

        push    fs

        lfs	    si, [bp+04h+6+6]                ; load lpBitmapInfo into ds:si
        add	    si, 28h					        ; offset Bitmapinfo by size to point to palette
        mov     eax, esi                       
        lds	    si, [bp+08h+6+6]                ; load lpDIBits into ds:si

SetDib16Loop:
        xor	    edx, edx
        mov	    dl, BYTE PTR [esi]              ; get the source pixel
		inc     esi

        mov	    ebx, DWORD PTR fs:[eax+edx*4]   ; load palette color into ebx

        mov	    edx, ebx                        ; copy the 32 bpp color to edx
        shr	    bh, 2                           ; remove 2 bits from the green
        and	    edx, 00f80000h                  ; Remove the lower 3 bits of the red
        shr	    bx, 3                           ; shift the green and blue over 3 bits (drop lower 3 bits of blue)
        shr	    edx, 8                          ; Shift the red into place
        or	    bx, dx                          ; bx = 16bpp pixel 5:6:5 format

        mov     es:[edi], bx                    ; store the pixel!
		add     edi, 2
        dec     cx
		jnz     SetDib16Loop

		pop     fs

        mov     ax, cx
        jmp     ExitJump


Test32Dst:
; Checking the busy bit in the processor
        mov     si, @data
        mov     ds, si
        lds     si, _lpDriverData ; Global Data
        mov     esi, _lph3IORegs
        mov     dx, wFlatDataSel
        mov     ds, dx
DoCpy32Read:
        mov     eax, [esi]
        test    al, 1        ; bit 0 = SST2_BUSY
        jnz     DoCpy32Read

        xor	    esi, esi
        xor	    edi, edi
        xor     eax, eax
;       calculate dest. offset

        lds     si, [bp+1Ch+6+6]                ; load lpDst into ds:si
        mov     ax, WORD PTR [bp+18h+6+6]       ; move Y into dx
        mov     edx, ds:[si+0Eh]                ; load displacement scan (stride)
        mul     edx                             ; eax = Y * Stride
        pop     edx                             ; get X from stack
        shl     edx, 2                          ; Mult by 4 for 4 Bytes per pixel (32 bpp)
        add     eax, edx                        ; eax holds pixel address offset from upper left corner in bytes
        les     edi, FWORD PTR ds:[si+16h]      ; load deBits into es:di
        add     edi, eax                        ; edi has the linear adress of the pixel from upper left corner in bytes

        push    fs

        lfs	    si, [bp+04h+6+6]                ; load lpBitmapInfo into ds:si
        add	    si, 28h					        ; offset Bitmapinfo by size to point to palette
        mov     eax, esi                       
        lds	    si, [bp+08h+6+6]                ; load lpDIBits into ds:si

SetDib32Loop:

        xor	    edx, edx
        mov	    dl, BYTE PTR [esi]              ; get the source pixel
		inc     esi

        mov	    ebx, DWORD PTR fs:[eax+edx*4]   ; load palette color into ebx

        mov     es:[edi], ebx                   ; store the pixel!
		add     edi, 4
        dec     cx
		jnz     SetDib32Loop

		pop     fs

DstDIBDone:
        mov     ax, cx
        jmp     ExitJump

NotSpecialCase:
Endm
endif		; end of ifdef STBPERF_HANDLE1X1

ifdef STBPERF_HANDLESTR1X1
;*----------------------------------------------------------------------
;Function name: HANDLESTR1X1
;
;Description:   Handles all 1 pixel 1 to 1 stretch blts by poking that 
;               pixel out to the framebuffer
;
;Information:   This is a MACRO.
;
;Return:        VOID
;----------------------------------------------------------------------*
HANDLESTR1X1 Macro ExitJump, PuntJump

        ; Check initial cases
        cmp	    WORD PTR [bp+18h+6+6], 0        ; Is Y == 0?
        jmp     PuntJump
        cmp	    WORD PTR [bp+24h+6+6], 1        ; Is wDestHeight == 1?
        jnz     NotStrSpecialCase
        cmp	    WORD PTR [bp+10h+6+6], 0        ; Is lpTranslate == NULL?
        jnz     NotStrSpecialCase

        lds	    si, [bp+2Eh+6+6]                ; load lpDev into DS:SI
        cmp	    BYTE PTR ds:[si+09h], 16        ; only handle 16bpp dst right now
        jnz     Test32StrDst

        lds	    si, [bp+14h+6+6]                ; load lpBitmapInfo into DS:SI

        cmp     WORD PTR ds:[si+0Eh], 8         ; Is src 8 bpp?
        jnz     NotStrSpecialCase

        cmp     DWORD PTR ds:[si+10h], 0        ; Is Src compression == BI_RGB?
        jnz     PuntJump

        cmp     DWORD PTR ds:[si+08h], 1        ; is the Src Height == 1?
        jnz     NotStrSpecialCase

        cmp     DWORD PTR ds:[si+04h], 1        ; is the Src Width == 1?
        jnz     NotStrSpecialCase

        cmp     DWORD PTR [bp+0h+6+6], 0        ; is lpClipRect == NULL?
        jz      @f

;       Clipping Check
        lds     si, DWORD PTR [bp+0h+6+6]       ; load lpClipRect into ds:si
        mov     ax, WORD PTR [bp+2Ah+6+6]       ; move X into ax
        cmp     ax, WORD PTR ds:[si+0h]         ; is X < Clip Rect left?
        jl      Dst16StrDIBDone                 ; Clipped left

        cmp     ax, WORD PTR ds:[si+4h]         ; is X >= Clip Rect right?
        jge     Dst16StrDIBDone                 ; Clipped Right

        mov     ax, WORD PTR [bp+28h+6+6]       ; move Y into ax
        cmp     ax, WORD PTR ds:[si+2h]         ; is Y < Clip Rect top?
        jl      Dst16StrDIBDone                 ; Clipped top

        cmp     ax, WORD PTR ds:[si+6h]         ; is Y >= Clip Rect bottom?
        jge     Dst16StrDIBDone                 ; Clipped bottom

@@:

; Handle a 1:1 8bpp StretchDIB operation from a single pixel to a single pixel
        xor	    edx, edx

        lds	    si, [bp+18h+6+6]                ; load lpDIBits into ds:si
        mov	    dl, BYTE PTR [si]               ; get the source pixel
        shl	    dx, 2                           ; multiply by 4 for RGBQUAD

        lds	    si, [bp+14h+6+6]                ; load lpBitmapInfo into ds:si
        add	    si, 28h                         ; offset Bitmapinfo by size to point to palette
        add	    si, dx                          ; offset by index into palette
        mov	    eax, DWORD PTR ds:[si]          ; load palette color into eax
	
        mov	    edx, eax                        ; copy the 32 bpp color to eax
        shr	    ah, 2                           ; remove 2 bits from the green
        and	    edx, 00f80000h                  ; Remove the lower 3 bits of the red
        shr	    ax, 3                           ; shift the green and blue over 3 bits (drop lower 3 bits of blue)
        shr	    edx, 8                          ; Shift the red into place
        or	    ax, dx                          ; ax = 16bpp pixel 5:6:5 format
        push    ecx                             ; store old ecx
        mov	    ecx, eax                        ; mov 16 bpp color into ecx

;       calculate dest. offset

        lds     si, [bp+2Eh+6+6]                ; load lpDst into ds:si
        cmp     WORD PTR ds:[si], 'RP'          ; make sure surface type is 'RP'
        jnz     NotStrSpecialCase

; Checking the busy bit in the processor
        mov     si, @data
        mov     ds, si
        lds     si, _lpDriverData ; Global Data
        mov     esi, _lph3IORegs
        mov     dx, wFlatDataSel
        mov     ds, dx
DoStrRead:
        mov     eax, [esi]
        test    al, 1        ; bit 0 = SST2_BUSY
        jnz     DoStrRead

        lds     si, [bp+2Eh+6+6]                ; load lpDst into ds:si
        xor     eax, eax
        mov     ax, WORD PTR [bp+28h+6+6]       ; move Y into dx
        mov     edx, ds:[si+0Eh]                ; load displacement scan (stride)
        mul     edx                             ; eax = Y * Stride
        xor	    edx, edx
        mov     dx, WORD PTR [bp+2Ah+6+6]       ; move X into dx
        shl     edx, 1                          ; Mult by 2 for 2 Bytes per pixel (16 bpp)
        add     eax, edx                        ; eax holds pixel address on screen
        lds     esi, FWORD PTR ds:[si+16h]      ; load deBits into ds:si
        mov     [esi+eax],cx                    ; store the pixel!

Dst16StrDIBDone:
        pop	    ecx
        mov     ax, 1
        jmp     ExitJump

Test32StrDst:
        cmp	    BYTE PTR ds:[si+09h+6+6], 32    ; is dst 32 bpp?
        jnz     NotStrSpecialCase

        lds	    si, [bp+14h+6+6]                ; load lpBitmapInfo into DS:SI

        cmp     WORD PTR ds:[si+0Eh], 8         ; Is src 8 bpp?
        jnz     NotStrSpecialCase

        cmp     DWORD PTR ds:[si+10h], 0        ; Is Src compression == BI_RGB?
        jnz     NotStrSpecialCase

        cmp     DWORD PTR ds:[si+08h], 1        ; is the Src Height == 1?
        jnz     NotStrSpecialCase

        cmp     DWORD PTR ds:[si+04h], 1        ; is the Src Width == 1?
        jnz     NotStrSpecialCase

        cmp     DWORD PTR [bp+0h+6+6], 0        ; is lpClipRect == NULL?
        jz      @f

;       Clipping Check
        lds     si, DWORD PTR [bp+0h+6+6]       ; load lpClipRect into ds:si
        mov     ax, WORD PTR [bp+2Ah+6+6]       ; move X into ax
        cmp     ax, WORD PTR ds:[si+0]          ; is X < Clip Rect left?
        jl      Dst32StrDIBDone                 ; Clipped left

        cmp     ax, WORD PTR ds:[si+4]          ; is X >= Clip Rect right?
        jge     Dst32StrDIBDone                 ; Clipped Right

        mov     ax, WORD PTR [bp+28h+6+6]       ; move Y into ax
        cmp     ax, WORD PTR ds:[si+2]          ; is Y < Clip Rect top?
        jl      Dst32StrDIBDone                 ; Clipped top

        cmp     ax, WORD PTR ds:[si+6]          ; is Y >= Clip Rect bottom?
        jge     Dst32StrDIBDone                 ; Clipped bottom

@@:

; Handle a 1:1 8bpp StretchDIB operation from a single pixel to a single pixel
        xor	    edx, edx

        lds	    si, [bp+18h+6+6]                ; load lpDIBits into ds:si
        mov	    dl, BYTE PTR [si]               ; get the source pixel
        shl	    dx, 2                           ; multiply by 4 for RGBQUAD

        lds	    si, [bp+14h+6+6]                ; load lpBitmapInfo into ds:si
        add	    si, 28h                         ; offset Bitmapinfo by size to point to palette
        add	    si, dx                          ; offset by index into palette
        mov	    eax, DWORD PTR ds:[si]          ; load palette color into eax
	
        push    ecx                             ; store old ecx
        mov	    ecx, eax                        ; mov 16 bpp color into ecx

;       calculate dest. offset

        lds     si, [bp+2Eh+6+6]                ; load lpDst into ds:si
        cmp     WORD PTR ds:[si], 'RP'          ; make sure surface type is 'RP'
        jnz     NotStrSpecialCase

; Checking the busy bit in the processor
        mov     si, @data
        mov     ds, si
        lds     si, _lpDriverData ; Global Data
        mov     esi, _lph3IORegs
        mov     dx, wFlatDataSel
        mov     ds, dx
DoStr32Read:
        mov     eax, [esi]
        test    al, 1        ; bit 0 = SST2_BUSY
        jnz     DoStr32Read

        lds     si, [bp+2Eh+6+6]                ; load lpDst into ds:si
        xor     eax, eax
        mov     ax, WORD PTR [bp+28h+6+6]       ; move Y into dx
        mov     edx, ds:[si+0Eh]                ; load displacement scan (stride)
        mul     edx                             ; eax = Y * Stride
        xor	    edx, edx
        mov     dx, WORD PTR [bp+2Ah+6+6]       ; move X into dx
        shl     edx, 2                          ; Mult by 4 for 4 Bytes per pixel (32 bpp)
        add     eax, edx                        ; eax holds pixel address on screen
        lds     esi, FWORD PTR ds:[si+16h]      ; load deBits into ds:si
        mov     ds:[esi+eax],ecx                ; store the pixel!

Dst32StrDIBDone:
        pop	    ecx
        mov     ax, 1
        jmp     ExitJump

NotStrSpecialCase:
Endm
endif	; End of ifdef STBPERF_HANDLESTR1X1
; STB End Changes
        

;*----------------------------------------------------------------------
;Function name: PREPARE_STACK
;
;Description:   Align the stack and make room for pParams
;
;Information:   This is a MACRO.
;
;Return:        VOID
;----------------------------------------------------------------------*
PREPARE_STACK Macro
        and     sp, NOT 3       ; align stack
        push    _dwDevNode      ; leave room on stack for pParams
Endm        


;*----------------------------------------------------------------------
;Function name: FARCALL32
;
;Description:   The call that "thunks" to the 32-bit side.
;
;Information:   This is a MACRO.
;
;Return:        VOID
;----------------------------------------------------------------------*
FARCALL32 Macro FnNumber
        mov     ax, wFlatDataSel
        mov     es, ax
        mov     ecx, _apfnTable32
        mov     edx, es:[ecx][FnNumber*4]
        mov     DWORD PTR pfnTemp32, edx
        call    [pfnTemp32]
Endm        


;*----------------------------------------------------------------------
;Function name: PUNTCHECK
;
;Description:   Test for early exit from the 16-bit functions.
;
;Information:   This is a MACRO.
;
;Return:        VOID
;----------------------------------------------------------------------*
PUNTCHECK Macro PuntJump
        test    eax, 80000000h
        jnz     short PuntJump
Endm        


;*----------------------------------------------------------------------
;Function name: GETRETURN
;
;Description:   Conditionally return either AX or DX:AX
;
;Information:   This is a MACRO.
;
;Return:        VOID
;----------------------------------------------------------------------*
GETRETURN Macro NumWords
        pop     ax
        IF NumWords GT 1
        pop     dx
        ENDIF
Endm        


;*----------------------------------------------------------------------
;Function name: FN16EXIT
;
;Description:   Epilog to resotre stack and pop registers.
;
;Information:   This is a MACRO.
;
;Return:        VOID
;----------------------------------------------------------------------*
FN16EXIT Macro RetSizeORDibFn
        mov     sp, bp
        pop     bp
        pop     di
        pop     si
        pop     ds
        
; code return and jmp individually for each because
; some of the routines do special things like mask regs, etc.
;        IF (OPATTR (RetSizeORDibFn)) AND 00000001y ; code label referenced
;        jmp     RetSizeORDibFn
;        ELSE
;        ret     RetSizeORDibFn
;        ENDIF
Endm        
        

;=======================================================================
_DATA   SEGMENT
;=======================================================================

        PUBLIC  wLdtAliasSel
        PUBLIC  wFlatCodeSel
        PUBLIC  wFlatDataSel

pfnDpmiExt      DWORD   ?
wLdtAliasSel    WORD    ?
wFlatCodeSel    WORD    ?
wFlatDataSel    WORD    ?

pfnTemp32       FWORD   ?
extrn _apfnTable32:DWORD
extrn _dwDevNode:DWORD

pfnVXDLDR       DWORD   ?
pfnCSIM         DWORD   ?
szCSIM          BYTE    'H3HAL.VXD', 0
szMsDos         BYTE    'MS-DOS', 0

extrn   _FirstEnable:WORD       ; in h3.c

_DATA   ENDS
;-----------------------------------------------------------------------

;=======================================================================
_THK16TEXT   SEGMENT
;=======================================================================

;;extrn _lpDriverData:Far
extrn _Palettized:Far

;*----------------------------------------------------------------------
;Function name:  _InitThunks
;
;Description:    Setup for thunking from the 16- to 32-bit side.
;
;Information:    Calls INT 31h and 2Fh.
;
;Return:         AX     1 for success, 0 for failuer.
;----------------------------------------------------------------------*
_InitThunks      PROC    PUBLIC USES di si

        ; get DPMI extensions entry point

        ; Undocumented Interrupt 2Fh Function 168Ah
        ;
        ; in:  DS:SI pointer to null-terminated string "MS-DOS"
        ; out: ES:DI pointer to DPMI extensions entry point

        push    bp
        mov     bp, sp
        push    di
        push    si
        push    es
        push    ds

        cmp     _FirstEnable,0
        je      gotSelectors

        lea     si, szMsDos

        mov     ax, 168Ah
        int     2Fh

        cmp     al, 8Ah
        jne     @f
        jmp     bail
@@:
        ; save the DPMI extensions entry point


        mov     WORD PTR pfnDpmiExt, di
        mov     WORD PTR pfnDpmiExt[2], es

        ; get the LDT alias selector

        mov     ax, 100h
        call    [pfnDpmiExt]

        jnc     @f
        jmp     bail
@@:
        ; save LDT alias selector

        mov     wLdtAliasSel, ax

        ; create two selectors

        mov     cx, 2

        xor     ax, ax
        int     31h

        jnc     @f
        jmp     bail
@@:
        ; the first selector will be the 32-bit flat code selector

        mov     wFlatCodeSel, ax

        ; make an extra copy in pfnTemp32

        mov     WORD PTR pfnTemp32[4], ax

        ; set code segment base to zero

        mov     bx, ax
        xor     cx, cx
        xor     dx, dx

        mov     ax, 7
        int     31h

        jnc     @f
        jmp     bail
@@:
        ; set code segment limit to four gigs

        mov     bx, wFlatCodeSel
        mov     cx, 0FFFFh
        mov     dx, 0FFFFh

        mov     ax, 8
        int     31h

        jnc     @f
        jmp     bail
@@:
        ; make it a 32-bit code selector

        mov     bx, wFlatCodeSel
        mov     cx, 0C0FAh

        mov     ax, 9
        int     31h

        jnc     @f
        jmp     bail
@@:
        ; now get the selector increment

        mov     ax, 3
        int     31h

        jnc     @f
        jmp     bail
@@:
        ; the second selector will be the 32-bit flat data selector

        add     ax, wFlatCodeSel
        mov     wFlatDataSel, ax

        ; set data segment base to zero

        mov     bx, ax
        xor     cx, cx
        xor     dx, dx

        mov     ax, 7
        int     31h

        jnc     @f
        jmp     bail
@@:
        ; set data segment limit to four gigs

        mov     bx, wFlatDataSel
        mov     cx, 0FFFFh
        mov     dx, 0FFFFh

        mov     ax, 8
        int     31h

        jnc     @f
        jmp     bail
@@:
        ; make it a 32-bit data selector

        mov     bx, wFlatDataSel
        mov     cx, 0C0F2h

        mov     ax, 9
        int     31h
        jc      bail

gotSelectors:
        push    SEG    CallVxD32
        push    OFFSET CallVxD32

        ; look up and push flat pointer to the LDT

        mov     bx, wLdtAliasSel
        mov     es, bx
        and     bx, NOT 7
        mov     al, es:[bx][4]
        mov     ah, es:[bx][7]
        push    ax
        push    es:[bx][2]

        push    dword ptr _lpDriverData

        push    _dwDevNode

        ; call 32-bit side with the flat data selector in AX

        mov     ax, wFlatDataSel
        mov     es, ax

        mov     ecx, _apfnTable32
        mov     edx, es:[ecx]

        mov     DWORD PTR pfnTemp32, edx
        call    [pfnTemp32]

        add     sp, 16

        mov     ax, 1

        ; and we're done

        jmp     done

bail:
        xor     ax, ax

done:
        pop     ds
        pop     es
        pop     si
        pop     di
        mov     sp, bp
        pop     bp
        ret

;-----------------------------------------------------------------------
_InitThunks      ENDP
;-----------------------------------------------------------------------


IFDEF STBPERF_CMBITBLT
        extern  _EvictBmp:FAR
ENDIF
;*----------------------------------------------------------------------
;Function name:  Bitblt
;
;Description:    16-bit GDI entry point of 32-bit function
;
;Information:
;
;Return:        AX      result of the return from the 32-bit side.
;----------------------------------------------------------------------*
ifndef STB_16BIT_BITBLT
BitBlt  PROC    FAR PASCAL PUBLIC
        FN16ENTRY               ; save & load EBP, save other regs
IFDEF STBPERF_CMBITBLT
		HANDLECMBLT  1Ch, 14h, PuntBlt
ENDIF
ifndef SKIP_STAGE2
        DEVBITCHECK     14h     ; check source
endif
ifndef SKIP_STAGE2
        DEVBITCHECK     1Ch     ; check destination
endif
        TRIVIAL_REJECT  1Ch, PuntBlt
        LOADDS                  ; get the driver's data segment loaded
        DIBBEGINCURSOR  28h, 20h, CURSOR_POINTS, 26h, 24h, 1Eh, 1Ch, 1Ah, 18h, 1Ah, 18h
        PREPARE_STACK
        FARCALL32       1       ; Bitblt = 1
        DIBENDCURSOR    28h, 20h
        PUNTCHECK       PuntBlt
        GETRETURN       1
        FN16EXIT        
        ret             32
PuntBlt:
        FN16EXIT        
        jmp             DIB_BitBlt
BitBlt  ENDP

else	;ifndef 
; In this case, GDI calls the 16-bit version of BitBlt 
; directly.  In cases of sofwtare cursor, and other unsupported 
; BitBlt calls we can bail out
; to the C code below and let the mini VDD do the text
; for us.

BitBltC:	
PUBLIC	BitBltC

        FN16ENTRY               ; save & load EBP, save other regs
ifndef SKIP_STAGE2
        DEVBITCHECK     14h     ; check source
endif
ifndef SKIP_STAGE2
        DEVBITCHECK     1Ch     ; check destination
endif
        TRIVIAL_REJECT  1Ch, PuntBlt
        LOADDS                  ; get the driver's data segment loaded
        DIBBEGINCURSOR  28h, 20h, CURSOR_POINTS, 26h, 24h, 1Eh, 1Ch, 1Ah, 18h, 1Ah, 18h
        PREPARE_STACK
        FARCALL32       1       ; Bitblt = 1
        DIBENDCURSOR    28h, 20h
        PUNTCHECK       PuntBlt
        GETRETURN       1
        FN16EXIT        
        retf            32
PuntBlt:
        FN16EXIT        
        jmp             DIB_BitBlt

endif	;ifndef 


;*----------------------------------------------------------------------
;Function name:  StretchBlt
;
;Description:    16-bit GDI entry point of 32-bit function
;
;Information:
;
;Return:        AX      result of the return from the 32-bit side.
;----------------------------------------------------------------------*
StretchBlt  PROC    FAR PASCAL PUBLIC
        FN16ENTRY               ; save & load EBP, save other regs
ifndef SKIP_STAGE2
        DEVBITCHECK     18h     ; check source
endif
ifndef SKIP_STAGE2
        DEVBITCHECK     24h     ; check destination
endif
        TRIVIAL_REJECT  24h, PuntStretchBlt
        LOADDS
        DIBBEGINCURSOR  30h, 24h, CURSOR_POINTS, 2Eh, 2Ch, 22h, 20h, 2Ah, 28h, 1Eh, 1Ch
        PREPARE_STACK
        FARCALL32       29      ; StretchBlt = 29
        DIBENDCURSOR    30h, 24h
        PUNTCHECK       PuntStretchBlt
        GETRETURN       1
        FN16EXIT        
        ret             40
PuntStretchBlt:
        FN16EXIT                
        and             ebx, 0ffffh
        jmp             DIB_StretchBlt
StretchBlt  ENDP


;*----------------------------------------------------------------------
;Function name:  Output
;
;Description:    16-bit GDI entry point of 32-bit function
;
;Information:
;
;Return:        AX      result of the return from the 32-bit side.
;----------------------------------------------------------------------*
Output     PROC    FAR PASCAL PUBLIC
IFNDEF STBPERF_POLYLINE16
        FN16ENTRY               ; save & load EBP, save other regs
ifndef SKIP_STAGE2
        DEVBITCHECK     18h     ; check destination
endif
        TRIVIAL_REJECT  18h, PuntOutput
        LOADDS                  ; get the driver's data segment loaded
        DIBBEGINCURSOR  24h, 0h, CURSOR_RECT, 0Ch
        PREPARE_STACK
        FARCALL32       8       ; Output = 8
        DIBENDCURSOR    24h
        PUNTCHECK       PuntOutput
        GETRETURN       2
        FN16EXIT
        ret     28
PuntOutput:
        FN16EXIT
        jmp     DIB_Output
ELSE
		extern  PolyLine16:FAR
		jmp		PolyLine16
ENDIF
Output      ENDP

ifdef STBPERF_POLYLINE16

OutputC32:	
PUBLIC	OutputC32

        FN16ENTRY               ; save & load EBP, save other regs
ifndef SKIP_STAGE2
        DEVBITCHECK     18h     ; check destination
endif
        TRIVIAL_REJECT  18h, PuntOutputC32
        LOADDS                  ; get the driver's data segment loaded
        DIBBEGINCURSOR  24h, 0h, CURSOR_RECT, 0Ch
        PREPARE_STACK
        FARCALL32       8       ; Output = 8
        DIBENDCURSOR    24h
        PUNTCHECK       PuntOutputC32
        GETRETURN       2
        FN16EXIT
        retf			28
PuntOutputC32:
        FN16EXIT
        jmp     DIB_Output

endif ;STBPERF_POLYLINE16




;*----------------------------------------------------------------------
;Function name:  DeviceBitmapBits
;
;Description:    16-bit GDI entry point of 32-bit function
;
;Information:
;
;Return:        AX      result of the return from the 32-bit side.
;----------------------------------------------------------------------*
DeviceBitmapBits    PROC    FAR PASCAL PUBLIC
        FN16ENTRY               ; save & load EBP, save other regs
ifndef SKIP_STAGE2
        DEVBITCHECK     16h     ; check destination
endif
        TRIVIAL_REJECT  16h, PuntDevBitmapBits
        LOADDS                  ; get the driver's data segment loaded
        DIBBEGINCURSOR  22h, 0h, CURSOR_SCAN, 00h, 1Eh, 00h, 1Ch
        PREPARE_STACK
        FARCALL32       20      ; DeviceBitmapBits = 20
        DIBENDCURSOR    22h
        PUNTCHECK       PuntDevBitmapBits
        GETRETURN       1
        FN16EXIT
        ret     26
PuntDevBitmapBits:
        ; punt DeviceBitmapBits
        ; We must add the Palettized parameter if we punt!
        LOADDS                  ; get the driver's data segment loaded
        mov     ax, word ptr _Palettized ; Get flag before blowing away Dataseg
        FN16EXIT
        ; DIB_DibBltExt needs an extra parameter -- the palettized flag.
        pop     ecx             ; Move return address out of the way
        push    ax              ; This is the Palettized flag
        push    ecx             ; Restore the return address
        jmp     DIB_DibBltExt
DeviceBitmapBits  ENDP


;*----------------------------------------------------------------------
;Function name:  SetDIBitsToDevice
;
;Description:    16-bit GDI entry point of 32-bit function
;
;Information:
;
;Return:        AX      result of the return from the 32-bit side.
;----------------------------------------------------------------------*
SetDIBitsToDevice   PROC    FAR PASCAL PUBLIC
        FN16ENTRY               ; save & load EBP, save other regs
        ;;DEVBITCHECK     1Ch     ; check destination
ifndef SKIP_STAGE2
        DEVBITCHECK     1Ch     ; check destination
endif
        TRIVIAL_REJECT  1Ch, PuntSetDIBits
; STB Begin Changes
ifdef STBPERF_HANDLE1X1
        HANDLE1X1       FunctionExit
endif
; STB End Changes
        LOADDS                  ; get the driver's data segment loaded
        DIBBEGINCURSOR  28h, 0h, CURSOR_RECT, 1Ch
        PREPARE_STACK
        FARCALL32       23      ; SetDIBitsToDevice = 23
        DIBENDCURSOR    28h
        PUNTCHECK       PuntSetDIBits
        GETRETURN       1
; STB Begin Changes
ifdef STBPERF_HANDLE1X1
FunctionExit:
endif
; STB End Changes
        FN16EXIT
        ret             32
PuntSetDIBits:
        FN16EXIT
        jmp             DIB_DibToDevice
SetDIBitsToDevice  ENDP


;*----------------------------------------------------------------------
;Function name:  StretchDIBits
;
;Description:    16-bit GDI entry point of 32-bit function
;
;Information:
;
;Return:        AX      result of the return from the 32-bit side.
;----------------------------------------------------------------------*
StretchDIBits  PROC    FAR PASCAL PUBLIC
        FN16ENTRY               ; save & load EBP, save other regs
        ;;DEVBITCHECK     2Eh     ; check destination
ifndef SKIP_STAGE2
        DEVBITCHECK     2Eh     ; check destination
endif
        TRIVIAL_REJECT  2Eh, PuntStretchDIB
; STB Begin Changes
ifdef STBPERF_HANDLESTR1X1
        HANDLESTR1X1    FunctionStrExit, PuntStretchDIB
endif
; STB End Changes
        LOADDS                  ; get the driver's data segment loaded
        DIBBEGINCURSOR  3Ah, 0h, CURSOR_POINTS, 36h, 34h, 0h, 0h, 32h, 30h, 0h, 0h
        PREPARE_STACK
        FARCALL32       30      ; StretchDIBits = 30
        DIBENDCURSOR    3Ah
        PUNTCHECK       PuntStretchDIB
        GETRETURN       1
; STB Begin Changes
ifdef STBPERF_HANDLESTR1X1
FunctionStrExit:
endif
; STB End Changes
        FN16EXIT
        ret             50
PuntStretchDIB:
        FN16EXIT
        jmp             DIB_StretchDIBits
StretchDIBits  ENDP


;*----------------------------------------------------------------------
;Function name:  ExtTextOut
;
;Description:    16-bit GDI entry point of 32-bit function
;
;Information:
;
;Return:        AX      result of the return from the 32-bit side.
;----------------------------------------------------------------------*

; STB Begin Changes
; STB-SR 1/13/98 Added code for bj
ifndef STB_16BIT_TEXTOUT
; STB End Changes

ExtTextOut      PROC    FAR PASCAL PUBLIC
        FN16ENTRY               ; save & load EBP, save other regs
ifndef SKIP_STAGE2
        DEVBITCHECK     24h     ; check destination
endif
        TRIVIAL_REJECT  24h, PuntExtTextOut
        DIBBEGINCURSOR  30h, 0h, CURSOR_RECT, 28h
        LOADDS                  ; get the driver's data segment loaded
        PREPARE_STACK
        FARCALL32       14      ; ExtTextOut = 14
        DIBENDCURSOR    30h
        PUNTCHECK       PuntExtTextOut
        GETRETURN       2
        FN16EXIT       
        ret             40
PuntExtTextOut:
        FN16EXIT
        jmp             DIB_ExtTextOut
ExtTextOut      ENDP

; STB Begin Changes
; STB-SR 1/13/98 Added code for bj
else ; ifndef STB_16BIT_TEXTOUT

; In this case, GDI calls the 16-bit version of ExtTextOut 
; directly.  In cases of sofwtare cursor, we can bail out
; to the C code below and let the mini VDD do the text
; for us.
ExtTextOutC:	
PUBLIC	ExtTextOutC

        FN16ENTRY               ; save & load EBP, save other regs
ifndef SKIP_STAGE2
        DEVBITCHECK     24h     ; check destination
endif
        TRIVIAL_REJECT  24h, PuntExtTextOut
        DIBBEGINCURSOR  30h, 0h, CURSOR_RECT, 28h
        LOADDS                  ; get the driver's data segment loaded
        PREPARE_STACK
        FARCALL32       14      ; ExtTextOut = 14
        DIBENDCURSOR    30h
        PUNTCHECK       PuntExtTextOut
        GETRETURN       2
        FN16EXIT       
        retf            40

PuntExtTextOut:
        FN16EXIT
        jmp             DIB_ExtTextOut


endif ; ifndef STB_16BIT_TEXTOUT
; STB End Changes

;*----------------------------------------------------------------------
;Function name:  BitmapBits
;
;Description:    16-bit GDI entry point of 32-bit function
;
;Information:
;
;Return:        AX      result of the return from the 32-bit side.
;----------------------------------------------------------------------*
BitmapBits PROC FAR PASCAL PUBLIC
        FN16ENTRY               ; save & load EBP, save other regs
        mov     ax, [bp+6+6+08h]
        test    ax, 4h          ; DBB_Copy?
        jz      short DoLoad
ifndef SKIP_STAGE2
        DEVBITCHECK     00h      ; Check Source
endif
DoLoad:
ifndef SKIP_STAGE2
        DEVBITCHECK     0Ch     ; check destination
endif
        TRIVIAL_REJECT  0Ch, PuntBitmapBits
        LOADDS                  ; get the driver's data segment loaded
        PREPARE_STACK
        FARCALL32       32      ; BitmapBits = 32
        PUNTCHECK       PuntBitmapBits
        GETRETURN       2
        FN16EXIT       
        ret             16
PuntBitmapBits:
        mov     si, @data
        mov     ds, si
        RAMPAGE_SYNC
        FN16EXIT
        jmp             DIB_BitmapBits
BitmapBits ENDP


;*----------------------------------------------------------------------
;Function name: _MapSharedData
;
;Description:   16-bit entry point of 32-bit function
;
;Information:   Performs a FarToFlat on lpDriverData on the
;               32-bit side.
;               Is this currently used?
;
;Return:        VOID
;----------------------------------------------------------------------*
_MapSharedData  PROC    FAR PASCAL PUBLIC

        ; save caller's DS, SI, and DI

        push    ds
        push    si
        push    di

        ; load our DS

        mov     ax, @data
        mov     ds, ax

        ; save caller's BP

        push    bp     
        movzx   ebp, sp

        ; align stack

        and     sp, NOT 3

        ; leave room on stack for pParams

        push    _dwDevNode
        ;;sub     sp, 4

        ; call 32-bit side with the flat data selector in AX

        mov     ax, wFlatDataSel
        mov     es, ax

        mov     ecx, _apfnTable32
        mov     edx, es:[ecx][34*4]

        mov     DWORD PTR pfnTemp32, edx
        call    [pfnTemp32]


        ; restore SP, caller's BP, DI, SI, and DS

        mov     sp, bp
        pop     bp
        pop     di
        pop     si
        pop     ds

        ret    4
_MapSharedData  ENDP


;*----------------------------------------------------------------------
;Function name: _initFifo
;
;Description:   16-bit entry point of 32-bit function to initialize
;               the command fifo.
;
;Information:   The 32-bit side is a dummy function.
;
;Return:        VOID
;----------------------------------------------------------------------*
_initFifo  PROC    FAR PASCAL PUBLIC

        ; save caller's DS, SI, and DI

        push    ds
        push    si
        push    di

        ; load our DS

        mov     ax, @data
        mov     ds, ax

        ; save caller's BP

        push    bp
        movzx   ebp, sp

        ; align stack

        and     sp, NOT 3

        ; leave room on stack for pParams

        push    _dwDevNode
        ;;sub     sp, 4

        ; call 32-bit side with the flat data selector in AX

        mov     ax, wFlatDataSel
        mov     es, ax

        mov     ecx, _apfnTable32
        mov     edx, es:[ecx][35*4]

        mov     DWORD PTR pfnTemp32, edx
        call    [pfnTemp32]


        ; restore SP, caller's BP, DI, SI, and DS

        mov     sp, bp
        pop     bp
        pop     di
        pop     si
        pop     ds
                retf


;-----------------------------------------------------------------------
_initFifo  ENDP
;-----------------------------------------------------------------------


;*----------------------------------------------------------------------
;Function name: CallVxD32
;
;Description:   Calls into the 32-bit VxD.
;
;Information:   Used by _InitThunks.
;
;Return:        VOID
;----------------------------------------------------------------------*
CallVxD32       PROC

        push    ds

        mov     dx, @data
        mov     ds, dx

        call    [pfnCSIM]

        pop     ds

        db      66h
        retf

;-----------------------------------------------------------------------
CallVxD32       ENDP
;-----------------------------------------------------------------------
IFNDEF SKIP_STAGE2
        extern  _DoHostStage2:FAR
ENDIF

;*----------------------------------------------------------------------
;Function name: DibCursor
;
;Description:   Implements the DibCursor.
;
;Information:   Why did we implement this??
;
;Return:        VOID
;----------------------------------------------------------------------*
DibCursor PROC       
DC_PARAM_SIZE equ 22
        push    bp                              ; Save Frame pointer
        mov     bp, sp                          ; Get stack pointer
        push    bx                              ; Save
        push    si                              ; Save
        push    ds                              ; Save

        mov     bx, [bp+6]                      ; Get Offset to lpDest
        cmp     bx, 0                           ; If Null Skip
        jz      SkipDst

        add     bx, bp
        lds     si, ss:[bx+DC_PARAM_SIZE+6]     ; Get Pointer
        mov     bx, ds                          ; Get Selector
        or      bx, si                          ; Or Offset
        jz      SkipDst                         ; Zero is null pointer
        assume  si:NEAR PTR DIBENGINE
        cmp     [si].deType, TYPE_DIBENG        ; Is this DIBENGINE
        jnz     SkipDst
        mov     bx, [si].deFlags                ; Get flags
        and     bx, VRAM OR OFFSCREEN           ; If DeviceBitMap Skip
        cmp     bx, VRAM                        ; Only VRAM
        assume  si:nothing
        jnz     SkipDst                         ; Nope skip rest
        push    ds 
        push    si

        ;
        ; Set flag to remember to Call DibEndAccess
        ;
        mov     bx, @data
        mov     ds, bx
        lds     si, _lpDriverData        
        bts     WORD PTR [si + 8], 10

        ;
        ; Sync with hardware
        ;
        RAMPAGE_SYNC
 
        mov     bx, [bp+10]
        cmp     bx, CURSOR_POINTS
        jz      short DoPt1
        cmp     bx, CURSOR_RECT
        jz      short Rect1
        
        ;
        ; Scan Line Based Routine
        ;
        mov     ax, 0
        push    ax        

        mov     bx, [bp+14]                     ; Top Offset
        add     bx, bp
        push    ss:[bx+DC_PARAM_SIZE+6]         ; Top Value

        mov     bx, [bp+6]                      ; Get Offset to lpDest
        add     bx, bp
        lds     si, ss:[bx+DC_PARAM_SIZE+6]     ; Get Pointer
        assume  si:NEAR PTR DIBENGINE
        push    [si].deWidth
        assume  si:nothing
        jmp     Bot1
Rect1:
        ;
        ; Do a Clip Rectangle
        ;
        mov     bx, [bp+12]                     ; Rect Offset
        add     bx, bp                          ; Stack Pointer
        lds     si, ss:[bx+DC_PARAM_SIZE+6]     ; Dereference
        mov     bx, ds                          ; Is clip rect null 0
        or      bx, si
        jz      NoRect1                         ; Yes -- Just push zero

        lodsw                                   ; Get Left           
        push    ax                              ; Stack it
        lodsw                                   ; Get Top
        push    ax                              ; Stack it
        lodsw                                   ; Get Right
        push    ax                              ; Stack it        
        lodsw                                   ; Get Bottom
        push    ax                              ; Stack it
        jmp     DoCur1

        ;
        ; Would it be better to bail here and skip the call?
        ;
NoRect1:
        mov     ax, 0
        push    ax
        push    ax
        push    ax
        push    ax
        jmp     DoCur1

DoPt1:
        ;
        ; Push Left
        ;
        mov     bx, [bp+12]                     ; Left Offset
        add     bx, bp
        push    ss:[bx+DC_PARAM_SIZE+6]         ; Left Value

        ;
        ; Push Top
        ;
        mov     bx, [bp+14]                     ; Top Offset
        add     bx, bp
        push    ss:[bx+DC_PARAM_SIZE+6]         ; Top Value

        ;
        ; Push Right
        ;
        mov     bx, [bp+12]                     ; DLeft Offset
        add     bx, bp
        mov     ax, ss:[bx+DC_PARAM_SIZE+6]     ; DLeft Value
        mov     bx, [bp+20]                     ; DWidth Offset
        add     bx, bp
        mov     bx, ss:[bx+DC_PARAM_SIZE+6]     ; DWidth Value
        add     bx, ax                          ; DLeft+width
        push    bx                              ; DRight

        ;
        ; Push Bottom
        ;
Bot1:
        mov     bx, [bp+14]                     ; DTop Offset
        add     bx, bp
        mov     ax, ss:[bx+DC_PARAM_SIZE+6]     ; DTop Value
        mov     bx, [bp+22]                     ; DHeight Offset
        add     bx, bp
        mov     bx, ss:[bx+DC_PARAM_SIZE+6]     ; DHeight Value
        add     bx, ax                          ; DTop+Height
        push    bx                              ; DBottom

DoCur1:
        push    CURSOREXCLUDE
        call    DIB_BeginAccess
SkipDst:
        mov     bx, [bp+8]                      ; Get Offset to lpSrc
        cmp     bx, 0                           ; If Null Skip
        jz      SkipSrc

        add     bx, bp
        lds     si, ss:[bx+DC_PARAM_SIZE+6]     ; Get Pointer
        mov     bx, ds                          ; Get Selector
        or      bx, si                          ; Or Offset
        jz      SkipSrc                         ; Zero is null pointer
        assume  si:NEAR PTR DIBENGINE
        cmp     [si].deType, TYPE_DIBENG        ; Is this DIBENGINE
        jnz     SkipSrc
        mov     bx, [si].deFlags                ; Get flags
        and     bx, VRAM OR OFFSCREEN           ; If DeviceBitMap Skip
        cmp     bx, VRAM                        ; Only VRAM
        assume  si:nothing
        jnz     SkipSrc                         ; Nope skip rest

        push    ds 
        push    si
        ;
        ; Set flag to remember to Call DibEndAccess
        ;
        mov     bx, @data
        mov     ds, bx
        lds     si, _lpDriverData        
        bts     WORD PTR [si + 8], 11

        ;
        ; Sync with hardware
        ;
        RAMPAGE_SYNC
        
        mov     bx, [bp+10]
        cmp     bx, CURSOR_POINTS
        jz      short DoPt2
        cmp     bx, CURSOR_RECT
        jz      short Rect2

        ;
        ; Scan Line Based Routine
        ;
        mov     ax, 0
        push    ax        

        mov     bx, [bp+18]                     ; Top Offset
        add     bx, bp
        push    ss:[bx+DC_PARAM_SIZE+6]         ; Top Value

        mov     bx, [bp+8]                      ; Get Offset to lpDest
        add     bx, bp
        lds     si, ss:[bx+DC_PARAM_SIZE+6]     ; Get Pointer
        assume  si:NEAR PTR DIBENGINE
        push    [si].deWidth
        assume  si:nothing
        jmp     Bot2

Rect2:
        ;
        ; Do a Clip Rectangle
        ;
        mov     bx, [bp+12]                     ; Rect Offset
        add     bx, bp                          ; Stack Pointer
        lds     si, ss:[bx+DC_PARAM_SIZE+6]     ; Dereference
        mov     bx, ds                          ; Is clip rect null 0
        or      bx, si
        jz      NoRect2                         ; Yes -- Just push zero

        lodsw                                   ; Get Left           
        push    ax                              ; Stack it
        lodsw                                   ; Get Top
        push    ax                              ; Stack it
        lodsw                                   ; Get Right
        push    ax                              ; Stack it        
        lodsw                                   ; Get Bottom
        push    ax                              ; Stack it
        jmp     DoCur2

        ;
        ; Would it be better to bail here and skip the call?
        ;
NoRect2:
        mov     ax, 0
        push    ax
        push    ax
        push    ax
        push    ax
        jmp     DoCur2

DoPt2:
        ;
        ; Push Left
        ;
        mov     bx, [bp+16]                     ; SLeft Offset
        add     bx, bp
        push    ss:[bx+DC_PARAM_SIZE+6]         ; SLeft Value

        ;
        ; Push Top
        ;
        mov     bx, [bp+18]                     ; STop Offset
        add     bx, bp
        push    ss:[bx+DC_PARAM_SIZE+6]         ; STop Value

        ;
        ; Push Right
        ;
        mov     bx, [bp+16]                     ; SLeft Offset
        add     bx, bp
        mov     ax, ss:[bx+DC_PARAM_SIZE+6]     ; SLeft Value
        mov     bx, [bp+24]                     ; SWidth Offset
        add     bx, bp
        mov     bx, ss:[bx+DC_PARAM_SIZE+6]     ; SWidth Value
        add     bx, ax                          ; SLeft+width
        push    bx                              ; SRight

        ;
        ; Push Bottom
        ;
Bot2:
        mov     bx, [bp+18]                     ; STop Offset
        add     bx, bp
        mov     ax, ss:[bx+DC_PARAM_SIZE+6]     ; STop Value
        mov     bx, [bp+26]                     ; SHeight Offset
        add     bx, bp
        mov     bx, ss:[bx+DC_PARAM_SIZE+6]     ; SHeight Value
        add     bx, ax                          ; STop+Height
        push    bx                              ; SBottom

DoCur2:
        push    CURSOREXCLUDE
        call    DIB_BeginAccess
SkipSrc:
        pop     ds                              ; restore
        pop     si                              ; restore
        pop     bx                              ; restore
        pop     bp                              ; restore
        retf
;-----------------------------------------------------------------------
DibCursor ENDP
;-----------------------------------------------------------------------

_THK16TEXT   ENDS
;=======================================================================
        END
;=======================================================================
