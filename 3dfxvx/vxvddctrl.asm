;/* $Header: /devel/sst2/Win95/dx/minivdd/vxvddctrl.asm 1     5/24/99 5:12p Peterm $ */
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
;** File name:   vxvddctrl.asm
;**
;** Description: Mini-VDD Support Functions.
;**
;** $Revision: 1 $
;** $Date: 5/24/99 5:12p $
;**
;** $History: vxvddctrl.asm $
;; 
;; *****************  Version 1  *****************
;; User: Peterm       Date: 5/24/99    Time: 5:12p
;; Created in $/devel/sst2/Win95/dx/minivdd
;; Added new VoodooX files
;; 
;; *****************  Version 2  *****************
;; User: Peterm       Date: 5/21/99    Time: 4:45p
;; Updated in $/devel/sst2/Win95/dx/minivdd
;; merged in v3 tot changes
;; 
;**
;*/
        
;****************************************************************************
; VXVDDCTRL.ASM
;----------------------------------------------------------------------------
;
; VoodooX Mini-VDD Support Functions C Wrapper
;
;****************************************************************************

title VXVDD Mini-VDD Support Functions
.386p

.xlist
MINIVDD  EQU 1
WIN40COMPAT EQU 1
WIN41SERVICES EQU 1
include      VMM.INC
include      VMMREG.INC
include      VWIN32.INC
include      MINIVDD.INC
.list

;; lock.c 
EXTRN        _LockAPI_Dispatch:  NEAR
EXTRN        _LockAPI_Cleanup:   NEAR

;; kmvt.asm
;extrn       GetDDHAL:proc
;extrn       _GetKernelInfo:proc

VxD_DATA_SEG
; DEV CONTROL Vars
DevCtlRet             dd 0
VxD_DATA_ENDS

Declare_Virtual_Device  H6VDD, \
    3,                         \
    1,                         \
    MiniVDD_Control,           \
    Undefined_Device_ID,       \
    VDD_Init_Order,            \
    ,                          \
    _LockAPI_Dispatch,         \
    ,

VxD_LOCKED_CODE_SEG

Begin_Control_Dispatch  MiniVDD
        Control_Dispatch Sys_Dynamic_Device_Init,   MiniVDD_Dynamic_Init,sCall,<ebx>
        Control_Dispatch Device_Init,               MiniVDD_Dynamic_Init,sCall,<ebx>
        Control_Dispatch System_Exit,               MiniVDD_System_Exit,sCall
        Control_Dispatch Sys_VM_Terminate,          MiniVDD_Sys_VM_Terminate,sCall
        Control_Dispatch PNP_NEW_DEVNODE,           MiniVDD_PNP_NEW_DEVNODE
        Control_Dispatch W32_DeviceIOControl        MiniVDD_W32_DevIoCtl,sCall, <esi>
        Control_Dispatch VM_Terminate               _LockAPI_Cleanup
End_Control_Dispatch MiniVDD

PNP_NewDevNode  PROTO NEAR C :DWORD,:DWORD,:DWORD

;*----------------------------------------------------------------------
;Function name:  MiniVDD_PNP_NEW_DEVNODE
;
;Description:    A new PNP Dev Node.
;
;Information:
;
;Return:         DWORD   0 is always returned.
;----------------------------------------------------------------------*
public  MiniVDD_PNP_NEW_DEVNODE
BeginProc MiniVDD_PNP_NEW_DEVNODE, RARE

    push    ebx
    push    ebp
    push    edi
    push    esi

    invoke  PNP_NewDevNode, eax, ebx, edx

    ; Right now we don't care about return value so say we are always successful
    xor     eax, eax

    clc
    pop     esi
    pop     edi
    pop     ebp
    pop     ebx

    ret                                     
EndProc MiniVDD_PNP_NEW_DEVNODE


;*----------------------------------------------------------------------
;Function name:  _HookInt10
;
;Description:    This routine is to get into the Int10 chain so
;                that we can catch the DDC calls at the start of
;                Windows '95.
;
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
public  _HookInt10
_HookInt10 PROC
        mov     eax, 10h
        mov     esi, OFFSET32 MiniVDD_Int10
        VMMCall Hook_V86_Int_Chain
        ret
_HookInt10 ENDP


;*----------------------------------------------------------------------
;Function name:  MiniVDD_Int10
;
;Description:    
;
;Information:
; On Entry: eax = int number (should be 10h)
;           ebx = current Virtual Machine handle
;           ebp = pointer to client register structure
;
;Return:         CF cleared if routine handled the int10h request.
;----------------------------------------------------------------------*
VESASupport PROTO NEAR C
public MiniVDD_Int10
BeginProc MiniVDD_Int10, DOSVM

    pushad
    cmp     eax,10h                 ; Handle an Int 10h?
    jne     I10_PassIt              ; No
                                    ; Yes
    mov     eax, [ebp].Client_EAX
    cmp     ax,4F15h                ; Is it a VESA VBE/DDC call?
    je      I10_HandleIt            ; Yes
                                    ; No
I10_PassIt:
    popad
    stc                             ; Signal request not handled
    ret

I10_HandleIt:                       ; Handle the request here.
    invoke  VESASupport
    jnc I10_PassIt                  ; VESA Support is opposite Int10 wrt carry flag    
    popad
    clc
    ret

EndProc MiniVDD_Int10


;*----------------------------------------------------------------------
;Function name:  _HelpVDD
;
;Description:    The VDD has a variable called Vid_Flags.  The
;                first time the VDD_Int_10 hook flag is called
;                it wants to set mode 3.  I don't know why it
;                wants to do this.  To make sure that this happens
;                at a opportune time I call this function once 
;                on the first mode set.  It calls the bios using
;                function ax=1400 which for us is OEMExtensions1
;                which based on the version V210D is nothing.
;
;Information:
;
;Return:         CF cleared if routine handled the int10h request.
;----------------------------------------------------------------------*
        public _HelpVDD
BeginProc _HelpVDD
        push    ebx                             ; Save ebx
        push    ebp                             ; Save ebp
        mov     ebp, [esp+0ch]                  ; Get Client Structure
        mov     ebx, [esp+10h]                  ; Current vm
        pushad
        Push_Client_State
        VMMcall Begin_Nest_V86_Exec             ; 
        mov     [ebp.Client_AX], 1400h          ; Benign Function 
        mov     eax,10h
        VMMcall Exec_Int
        VMMcall End_Nest_Exec                   ; All done with software ints
        Pop_Client_State
        popad
        pop     ebp                             ; Restore ebp
        pop     ebx                             ; Restore ebx
        ret
EndProc _HelpVDD

VxD_LOCKED_CODE_ENDS

 end
