;/* $Header: /devel/sst2/Win95/dx/dd16/ccursor.asm 2     6/11/99 7:02p Einkauf $ */
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
;** File name:   ccursor.asm
;**
;** Description: Cursor MASM support functions
;**
;** $Revision: 2 $
;** $Date: 6/11/99 7:02p $
;**
;** $History: ccursor.asm $
;; 
;; *****************  Version 2  *****************
;; User: Einkauf      Date: 6/11/99    Time: 7:02p
;; Updated in $/devel/sst2/Win95/dx/dd16
;; use SST2 cursor regs (guarded w/ ifdef R21)
;; 
;; *****************  Version 1  *****************
;; User: Peterm       Date: 5/18/99    Time: 2:41p
;; Created in $/devel/sst2/Win95/dx/dd16
;; copied over from h3\win95\dx\dd16 with merges for csim server and qt
;; 
;; *****************  Version 10  *****************
;; User: Michael      Date: 12/28/98   Time: 11:22a
;; Updated in $/devel/h3/Win95/dx/dd16
;; Added the 3Dfx/STB unified file/funciton header.  Add VSS keywords to
;; files where this was missing.
;** 
;**
;*/

        option OLDSTRUCTS
        .MODEL LARGE
        .586p

;----------------------------------------------------------------------------
;----------------------------------------------------------------------------
_TEXT   SEGMENT  WORD USE16 PUBLIC 'CODE'
_TEXT   ENDS
_DATA   SEGMENT  WORD USE16 PUBLIC 'DATA'
_DATA   ENDS

        ASSUME DS: _DATA
        ASSUME SS: NOTHING
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
; D A T A
;----------------------------------------------------------------------------
_DATA segment WORD PUBLIC 'DATA' use16

IFDEF GBLDATA_IN_PDEV
        extrn   lpDriverPDevice:DWORD
ELSE
        ; our PDevice
        extrn _DriverData:DWORD
        lpDriverPDevice equ _DriverData
ENDIF
        extern  wFlatDataSel:WORD

_DATA ends

;----------------------------------------------------------------------------
; C O D E
;----------------------------------------------------------------------------
_TEXT segment WORD PUBLIC 'CODE' use16

        extrn DIB_CheckCursorExt:FAR      ; in DIBENG

_TEXT ends

;----------------------------------------------------------------------------
; C O D E
;----------------------------------------------------------------------------
_TEXT segment WORD PUBLIC 'CODE' use16
        assume cs:_TEXT
        assume ds:nothing
        assume es:nothing
        assume fs:nothing
        assume gs:nothing



        assume ds:_DATA

;*----------------------------------------------------------------------
;Function name:  SetCursorBusy
;
;Description:    Sets the cursor busy semaphore (busy bit).
;
;Information:
;
;Return:         AX     1 if function set the cursor busy bit
;                       0 if cursor busy bit was already set
;----------------------------------------------------------------------*
SetCursorBusy PROC C USES edi es  lpCursorSemaphore :DWORD,
 
        les di, lpCursorSemaphore
        bts WORD PTR es:[di], 1
        jc  SetCursorAlreadyBusy         ;  already busy
        mov ax , 1
        ret
        
SetCursorAlreadyBusy:
       mov ax, 0
       ret
 
SetCursorBusy endp


;*----------------------------------------------------------------------
;Function name:  ClearCursorBusy
;
;Description:    Clears the cursor busy semaphore (busy bit).
;
;Information:
;
;Return:         AX     1 if function cleared the cursor busy bit
;                       0 if cursor busy bit was not previously set
;----------------------------------------------------------------------*
ClearCursorBusy PROC C USES  edi es  lpCursorSemaphore :DWORD,
 
	les di, lpCursorSemaphore
        btr WORD PTR es:[di], 1
        jnc  ClearCursorNotBusy         ;  Clearing busy thats not set.
        mov ax , 1
        ret
        
ClearCursorNotBusy:
       mov ax, 0 
       ret
 
ClearCursorBusy endp


;*----------------------------------------------------------------------
;Function name:  SetBusy
;
;Description:    Sets the busy semaphore.
;
;Information:
;
;Return:         AX     1 if function set the busy bit
;                       0 if busy bit was already set
;----------------------------------------------------------------------*
SetBusy PROC C USES edi es  lpSemaphore :DWORD,
 
	les di, lpSemaphore
        bts WORD PTR es:[di], 1
        jc  SetAlreadyBusy         ;  already busy
        mov ax , 1
        ret
        
SetAlreadyBusy:
       mov ax, 0
       ret
 
SetBusy endp


;*----------------------------------------------------------------------
;Function name:  ClearBusy
;
;Description:    Clear the busy semaphore.
;
;Information:
;
;Return:         AX     1 if function cleared the busy bit
;                       0 if busy bit was not previously set
;----------------------------------------------------------------------*
ClearBusy PROC C USES  edi es  lpSemaphore :DWORD,
 
	les di, lpSemaphore
        btr WORD PTR es:[di], 0
        jnc  ClearNotBusy         ;  Clearing busy thats not set.
        mov ax , 1
        ret
        
ClearNotBusy:
       mov ax, 0 
       ret
 
ClearBusy endp


;*----------------------------------------------------------------------
;Function name:  DoCursor
;
;Description:    Stores cursor's AND and XOR masks.
;
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
        assume ds:_DATA
DoCursor PROC C uses eax ebx cx edx edi si ds es DataOffset:DWORD, lpMask:DWORD
        mov     ax, wFlatDataSel        ; 32 Bit Data Sel
        mov     es, ax                  ; Load ds
        mov     edi, DataOffset         ; Get Offset
        lds     si, lpMask              ; Get Mask
        xor     edx, edx                ; eax=0
        mov     ebx, edx                ; ebx=0
        not     ebx                     ; ebx=FFFFFFFF
        mov     cx, 32
MaskStore:
        mov     eax, DWORD PTR [si]     ; Get And Mask
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, ebx                ; Store 1's
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, DWORD PTR [si+80h] ; Get Xor Mask
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, edx                ; 0's
        db      67h                     ; Address Override
        stosd                           ; Store Data
        add     si, 4                   ; Next Mask
        loop    MaskStore

        mov     cx, 32
UnusedStore:
        mov     eax, ebx                ; Store 1's
        db      67h                     ; Address Override
        stosd                           ; Store Data
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, edx                ; 0's
        db      67h                     ; Address Override
        stosd                           ; Store Data 
        db      67h                     ; Address Override
        stosd                           ; Store Data
        loop    UnusedStore
        ret
DoCursor        ENDP


        assume ds:_DATA
DoSST2Cursor PROC C uses eax ebx cx edx edi si ds es VidMem:DWORD, lpCursor:DWORD
        mov     ax, wFlatDataSel        ; 32 Bit Data Sel
        mov     es, ax                  ; Load ds
        mov     edi, VidMem             ; Get ptr to Video memory
        lds     si, lpCursor            ; Get ptr to Cursor data
        xor     edx, edx                ; eax=0
        mov     ebx, edx                ; ebx=0
        not     ebx                     ; ebx=FFFFFFFF
        mov     cx, 64
CursorStore:
        mov     eax, DWORD PTR [si]     ; Get Cursor data
        db      67h                     ; Address Override
        stosd                           ; Store Data
        add     si, 4                   ; Next data
        loop    CursorStore
        ret
DoSST2Cursor        ENDP


;*----------------------------------------------------------------------
;Function name:  DoBigCursor
;
;Description:    Doubles the cursor's AND and XOR masks (32x32 -> 64x64).
;
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
ifdef BIG_CURSOR
DoBigCursor PROC C uses eax ebx cx edx edi si ds es DataOffset:DWORD, lpMask:DWORD
        mov     ax, wFlatDataSel        ; 32 Bit Data Sel
        mov     es, ax                  ; Load ds
        mov     edi, DataOffset         ; Get Offset
        lds     si, lpMask              ; Get Mask
        mov     cx, 32
BigMaskStore:
        mov     bx, 2
MaskDbl:
        mov     eax, DWORD PTR [si]     ; Get And Mask
        call    BitDbl
        mov     eax, DWORD PTR [si+80h] ; Get Xor Mask
        call    BitDbl
        dec     bx
        jnz     MaskDbl
        add     si, 4                   ; Next Mask
        loop    BigMaskStore
        ret
DoBigCursor        ENDP


;*----------------------------------------------------------------------
;Function name:  BitDbl
;
;Description:    Doubles a cursor 32x32 mask.  Called for both
;                AND and XOR.
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
BitDbl  PROC
        push    ebx
        push    ecx
        push    edx        

        mov     bx, 4
StDbl:
        xor     edx, edx
        mov     cx, 8
Dbl:
        ror     al, 1
        jnc     SkipOr
        or      dl, 3h
SkipOr:
        ror     dx, 2
        loop    Dbl
        xchg    dh, dl
        xchg    edx, eax
        db      67h                     ; Address Override
        stosw                           ; Store Data
        xchg    edx, eax
        shr     eax, 8
        dec     bx
        jnz     StDbl
        pop     edx
        pop     ecx
        pop     ebx
        ret
BitDbl  ENDP   
endif


;*----------------------------------------------------------------------
;Function name:  DoCursorDblX
;
;Description:    Stores cursor's doubled AND and XOR masks.
;
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
        assume ds:_DATA
DoCursorDblX PROC C uses eax ebx cx edx edi si ds es DataOffset:DWORD, lpMask:DWORD
        mov     ax, wFlatDataSel        ; 32 Bit Data Sel
        mov     es, ax                  ; Load ds
        mov     edi, DataOffset         ; Get Offset
        lds     si, lpMask              ; Get Mask
        xor     edx, edx                ; eax=0
        mov     ebx, edx                ; ebx=0
        not     ebx                     ; ebx=FFFFFFFF
        mov     cx, 32
MaskStoreI:
        mov     eax, DWORD PTR [si]     ; Get And Mask
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, ebx                ; Store 1's
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, DWORD PTR [si+80h] ; Get Xor Mask
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, edx                ; 0's
        db      67h                     ; Address Override
        stosd                           ; Store Data

        mov     eax, DWORD PTR [si]     ; Get And Mask
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, ebx                ; Store 1's
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, DWORD PTR [si+80h] ; Get Xor Mask
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, edx                ; 0's
        db      67h                     ; Address Override
        stosd                           ; Store Data
        add     si, 4                   ; Next Mask
        loop    MaskStoreI

        ret
DoCursorDblX        ENDP


;*----------------------------------------------------------------------
;Function name:  DoBitCopy
;
;Description:    Moves cursor from a Source to Destination via
;                SW (REP MOVS).
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
DoBitCopy PROC C uses eax ebx ecx edx edi esi ds es SrcAddr:FWORD, DestAddr:FWORD, dwSrcAdj:DWORD, dwDestAdj:DWORD, wWidth:WORD, wHeight:WORD
        lds     esi, FWORD PTR SrcAddr          ; ds:esi = Source
        les     edi, FWORD PTR DestAddr         ; es:edi = DestStart
        mov     eax, dwSrcAdj                   ; Source Adjust
        mov     edx, dwDestAdj                  ; Dest Adjust
        mov     bx, wHeight                     ; Number of Times
        shl     ebx, 16                         ; Height in Upper word of ebx
        xor     ecx, ecx                        ; ecx=0
DoLoop:
        mov     bx, wWidth                      ; Get Width
        shr     bx, 1                           ; Number of Words
        adc     cx, 0                           ; Carry has # of bytes
        db      67h                             ; Address Override
        rep     movsb                           ; Move a byte
        shr     bx, 1                           ; Number of Dwords
        mov     cx, bx                          ; cx=#Dwords
        db      67h                             ; Address Override
        rep     movsd                           ; Move it
        adc     cx, 0                           ; Carry has #words
        db      67h                             ; Address Override
        rep     movsw                           ; Move a word
        add     esi, eax                        ; Start of Next Source Line
        add     edi, edx                        ; Start of Next Dest Line
        xor     bx, bx                          ; bx=0
        sub     ebx, 10000h                     ; Height=Height - 1
        jnz     DoLoop

        ret
DoBitCopy       ENDP


;*----------------------------------------------------------------------
;Function name:  ClearMem
;
;Description:    Clears a specified size of memory.
;
;Information:    Function appears to not be used.
;
;Return:         VOID
;----------------------------------------------------------------------*
ClearMem PROC C uses eax ecx edi es dwStart:DWORD, dwSize:DWORD
        mov     ax, wFlatDataSel
        mov     es, ax
        mov     edi, dwStart
        xor     eax, eax
        mov     ecx, dwSize
        db      67h
        db      66h
        rep     stosd
        ret
ClearMem        ENDP


;*----------------------------------------------------------------------
;Function name:  Force1XRate
;
;Description:    Forces 1X Rate for Intel Host Chip Sets
;
;Information:    
;
;Return:         VOID
;----------------------------------------------------------------------*
Force1XRate PROC C uses eax ebx ecx dx

        mov   dx, 0cf8h                 ; Conf Addr
        in    eax, dx                   ; Save it
        mov   ebx, eax                  ; in ebx
        mov   dx, 0cfch                 ; Conf Addr
        in    eax, dx                   ; Save it
        mov   ecx, eax                  ; in ecx

        ; Disable Us
        mov   dx, 0cf8h                 ; Conf Addr
        mov   eax, 08001005ch           ; Avenger's AGP
        out   dx, eax                   ; doit
        mov   dx, 0cfch                 ; Conf Data
        in    eax, dx                   ; Get it
        and   ah, 0FEh                  ; Disable AGP
        out   dx, eax                   ; Doit

        ; Disable Them
        mov   dx, 0cf8h                 ; Conf Addr
        mov   eax, 0800000a8h           ; Host Chip Set
        out   dx, eax                   ; Doit
        mov   dx, 0cfch                 ; Conf Data
        in    eax, dx                   ; Get It
        and   ah, 0FEh                  ; Disable AGP
        out   dx, eax                   ; Doit

        ; 1x Them
        and   al, 0FCh                  ; Mask off 1x 2x Bits
        or    al, 01h                   ; Or in 1x
        out   dx, eax                   ; Do it

        ; 1x Us
        mov   dx, 0cf8h                 ; Conf Addr
        mov   eax, 08001005ch           ; It's Us
        out   dx, eax                   ; Do it
        mov   dx, 0cfch                 ; Conf Data
        in    eax, dx                   ; get it
        and   al, 0FCh                  ; AGP 1x 2x Bits
        or    al, 01h                   ; Or in 1x
        out   dx, eax                   ; Do it

        ; Enable Them
        mov   dx, 0cf8h                 ; Conf Addr
        mov   eax, 0800000a8h           ; Host Chip Set
        out   dx, eax                   ; Do it
        mov   dx, 0cfch                 ; Conf Data
        in    eax, dx                   ; Get it
        or    ah, 01h                   ; Enable AGP
        out   dx, eax                   ; Do it

        ; Enable Us
        mov   dx, 0cf8h                 ; Conf Data
        mov   eax, 08001005ch           ; It's Us
        out   dx, eax                   ; Do it
        mov   dx, 0cfch                 ; Conf Addr
        in    eax, dx                   ; Get it
        or    ah, 01h                   ; Enable AGP
        out   dx, eax                   ; Do it

        ; Restore cf8
        mov   dx, 0cf8h                 ; Conf Addr
        mov   eax, ebx                  ; get it
        out   dx, eax                   ; Restore

        ; Restore cfc
        ;;mov   dx, 0cfch                 ; Conf Data        
        ;;mov   eax, ecx                  ; Get it
        ;;out   dx, eax                   ; Restore
        ret                             
Force1XRate ENDP

_TEXT ends
END
