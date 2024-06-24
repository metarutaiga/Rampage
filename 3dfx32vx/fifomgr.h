/* $Header: fifomgr.h, 20, 10/17/00 1:24:51 PM PDT, Ryan Bissell$ */
/*
** Copyright (c) 1995-2000, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** File Name:   FIFOMGR.H
**
** Description: Hardware access macros for Rampage
**
** $Log: 
**  20   3dfx      1.19        10/17/00 Ryan Bissell    Reinstated the P6FENCE 
**       for AGP fifos.
**  19   3dfx      1.18        10/6/00  Brian Danielson SLI updates pulling 
**       forward from Napalm.
**  18   3dfx      1.17        10/4/00  Michel Conrad   Fix typo under the SLI 
**       compile introduced in last check in.
**  17   3dfx      1.16        10/4/00  Dale  Kenaston  New Sage macros. Changed
**       IS_RAGE in fifo_MakeRoom and fifo_Wrap to IS_SAGE_ACTIVE.
**  16   3dfx      1.15        10/2/00  Ryan Bissell    Forgot to guard some 
**       AGP-specific code with #ifdef AGP_CMDFIFO
**  15   3dfx      1.14        10/2/00  Ryan Bissell    Big honkin'  
**       host-to-screen blits that were bigger than our FIFO were causing us to 
**       spinlock in fifo_MakeRoom().  Ported Steve Roger's CMDFIFO_CHECKBUMP 
**       changes from Napalm to Rampage.  Also fixed various sundry issues 
**       noticed while debugging this issue.
**  14   3dfx      1.13        9/22/00  Dale  Kenaston  Sage packet3, register 
**       and quickturn initialization changes  Added a macro to load the sage 
**       readptr. Removed HW_TNL and renamed sage registers.
**  13   3dfx      1.12        8/29/00  Tim Little      Removed some of the 
**       #ifdef HW_TNL compile checks, more to come latter
**  12   3dfx      1.11        8/10/00  Dale  Kenaston  Sage membase, fifo 
**       initialization and packet 3 size setup
**  11   3dfx      1.10        6/15/00  Michel Conrad   Compact HW_ACCESS_ENTRY 
**       macro for easier debug in asm. Fix typo errors within SLI ifdef.
**  10   3dfx      1.9         6/7/00   Evan Leland     adds preliminary 
**       instrumentation to Rampage driver
**  9    3dfx      1.8         4/26/00  Tim Little      Added GE membase, and a 
**       small change to fifo managment if SAGE is present.
**  8    3dfx      1.7         10/27/99 Andrew Sobczyk  Changes to support 
**       CMDFIFO with SLI
**  7    3dfx      1.6         10/5/99  Mark Einkauf    remove workaround for 
**       netlist 8.0 Pkt0; in HW_ACCESS_ENTRY remove explicit stall of 2d before
**       3d since hardware changed in 8.2 to implicitly stall 2d before 3d
**  6    3dfx      1.5         10/1/99  Mark Einkauf    Complete HW_ACCESS 
**       macros work
**  5    3dfx      1.4         9/29/99  Mark Einkauf    partial rework of 
**       HW_ACCESS_ENTRY/EXIT macros 
**  4    3dfx      1.3         9/24/99  Mark Einkauf    add CMDFIFO_BUMP macro 
**       for AGP fifo
**  3    3dfx      1.2         9/22/99  Mark Einkauf    PKT0 MOP workaround for 
**       8.0 netlist - use PKT1 temporarily
**  2    3dfx      1.1         9/20/99  Mark Einkauf    debug check - int 1 if 
**       fifo ptr past end of fifo
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
**
*/

#ifndef __FIFOMGR_H__
#define __FIFOMGR_H__

/* 
   Macros:

   GDI 16-bit display driver: IS_16=1, IS_32=0, THUNK32=0
   GDI 32-bit minivdd driver: IS_16=0, IS_32=0, THUNK32=1
   DirectX drivers:           IS_16=0, IS_32=1, THUNK32=0
*/ 

#ifdef IS_16
  #define VOLATILE 
  #define INLINE __inline
  #define STDCALL
  #define DEBUG_FIX
#else // IS_16
  #define VOID void
  #define VOLATILE volatile
  #define INLINE   __inline
  #define STDCALL  __stdcall
  #include "shared.h" 
#endif // IS_16

/* We claim space at the end of the fifo for:
 *   1 nop (2 32-bit words)
 *   1 jmp (1 32-bit word)
 *   1 pad word
 */
#define FIFO_END_ADJUST  (sizeof(FxU32) << 2)


//----------------------------------------------------------------------------------//
//                         COMMAND FIFO ACCESS MACROS
//----------------------------------------------------------------------------------//

// SETCF  - Send integer data using command fifo
// SETPH  - Send integer packet header using command fifo
// SETPD  - Send integer packet data using command fifo
// SET    - Send integer data to hardware using command fifo packet
// SETFCF - Send float data using command fifo
// SETFPD - Send float packet data using command fifo
// SETF   - Send float data to hardware using command fifo packet

#ifdef CMDFIFO

#ifdef IS_16

  #define SETCF(hwPtr, data) \
          h3WRITE(NULL, ((FxU32 *)((hwPtr) + (hwIndex++ << 2))), (data))

  #undef  SETFCF  // no float transfers from 16-bit driver
  #undef  SETFPD  // no float transfers from 16-bit driver
  #undef  SETF    // no float transfers from 16-bit driver

#else // IS_16

#if defined (NULLHARDWARE)
  #define SETCF(hwPtr, data)
  #define SETFCF(hwPtr, data)
  #define SETFPD(hwPtr, hwReg, data)
  #define SETF(hwPtr, hwReg, data)
#else
  #define SETCF(hwPtr, data) \
          *((volatile FxU32 *)(hwPtr) + hwIndex++) = (data)

  #define SETFCF(hwPtr, data) \
          *((volatile float *)(hwPtr) + hwIndex++) = (data)

  #define SETFPD(hwPtr, hwReg, data ) \
          SETFCF((hwPtr), (data));

  #define SETF(hwPtr, hwReg, data) \
          SETFCF((hwPtr), (data));
#endif // NULLHARDWARE

#endif // IS_16

  #define SETPH(hwPtr, data) \
          SETCF((hwPtr), (data));

  #define SETPD(hwPtr, hwReg, data) \
          SETCF((hwPtr), (data));

  #define SET(hwPtr, hwReg, data) \
          SETCF((hwPtr), (data));

  #define SETC(hwPtr, hwReg, data) \
          SETCF((hwPtr), (data));

  #define SETMOP(hwPtr, mopBits) \
          SETPH( (hwPtr), CMDFIFO_BUILD_MOP_PK0(mopBits));

#endif // CMDFIFO

//----------------------------------------------------------------------------------//
//                         DIRECT WRITE ACCESS MACROS
//----------------------------------------------------------------------------------//

// SETDW  - Direct write of integer data.
// SETFDW - Direct write of float data. 
// GET    - Direct read of integer data.

#ifdef IS_16

  #define SETDW(hwReg, data) \
          h3WRITE(NULL, (FxU32 *)&(hwReg), (data));

  #define GET(hwPtr) \
          h3READ(NULL, (FxU32 *)&(hwPtr))

  #undef  SETFDW  // no float transfers from 16-bit driver

#else // IS_16

  #define SETDW(hwReg, data) \
          (*(volatile FxU32 *)&(hwReg)) = (data);

  #define SETFDW(hwReg, data) \
          (*(volatile float *)&(hwReg)) = (data);

  #define GET(hwPtr) \
          (*(volatile FxU32 *)&(hwPtr))

#endif // IS_16

#ifndef CMDFIFO

  /* Make command fifo access macros use direct writes! */

  #define SETPH(hwPtr,data)

  #define SETPD(hwPtr, hwReg, data) \
          SETDW((hwReg), (data))

  #define SETFPD(hwPtr, hwReg, data) \
          SETFDW((hwReg), (data))
      
  #define SET(hwPtr, hwReg, data) \
          SETDW((hwReg), (data))
  
  #define SETC(hwPtr, hwReg, data) \
          SETDW((hwReg), (data))

  #define SETF(hwPtr, hwReg, data) \
          SETFDW((hwReg), (data))

#ifdef IS_32

  #define SETMOP(hwPtr, mopBits) \
          SETDW( ghw0->mopCMD, mopBits );

#else // IS_32

  #define SETMOP(hwPtr, mopBits) \
          SETDW( _FF(lpGRegs)->mopCMD2d, mopBits );

#endif // IS_32

#endif // !CMDFIFO


//----------------------------------------------------------------------------------//
//                         16,32-BIT COMMON MACROS
//----------------------------------------------------------------------------------//

  #define BUMP(n) // as nothing

  #define RESET_HW_PTR(ptr)  // as nothing

  #define LOAD_CMDFIFO_RDPTR(p) \
          GET(((SstCRegs *) p)->PRIMARY_CMDFIFO.readPtr )

  #define LOAD_SAGE_CMDFIFO_RDPTR(p) \
          GET(((Sst2GeCfeCmdRegs *) p)->cfeGeCmdRdPtr )

  #define ACCESS_NONE   0
  #define ACCESS_2D     BIT(0)
  #define ACCESS_3D     BIT(1)

  #define HW_ACCESS_ENTRY(hwPtr, access_type)                                        \
          /* if requested access type same as last, no action needed */              \
          if (access_type != _FF(lastAccessType))                                    \
          {                                                                          \
              CMDFIFO_CHECKROOM( (hwPtr), MOP_SIZE );                                \
              if (access_type == ACCESS_2D)                                          \
              {   /* want 2d, so flush previous 3d */                                \
                  SETMOP( (hwPtr),                                                   \
                     SST_MOP_FLUSH_TCACHE |                                          \
                     SST_MOP_FLUSH_PCACHE |                                          \
                     SST_MOP_STALL_3D |                                              \
                     (SST_MOP_STALL_3D_PE << SST_MOP_STALL_3D_SEL_SHIFT));           \
              }                                                                      \
              else                                                                   \
              {   /* want 3d, so flush previous 2d */                                \
                  SETMOP( (hwPtr),SST_MOP_STALL_2D );                                \
              }                                                                      \
              CMDFIFO_SAVE((hwPtr)); /* combine SAVE/RELOAD for performance */       \
              CMDFIFO_RELOAD((hwPtr));                                               \
          }

  // set global flag = access_type, which HW_ACCESS_ENTRY will check */
  #define HW_ACCESS_EXIT(access_type)   _FF(lastAccessType) = access_type;


//----------------------------------------------------------------------------------//
//                         COMMAND FIFO MANAGEMENT MACROS
//----------------------------------------------------------------------------------//

// CMDFIFO_PROLOG    - create local fifo pointer and local fifo count
// CMDFIFO_SETUP     - transfer global fifo pointer to local, zero fifo count
// CMDFIFO_EPILOG    - update global fifo pointer and count from locals, zero fifo count
// CMDFIFO_SAVE      - same as CMDFIFO_EPILOG, without zero fifo count
// CMDFIFO_RELOAD    - same as CMDFIFO_SETUP, with no differences
// CMDFIFO_CHECKROOM - check for space in command fifo using locals fifo count
// CMDFIFO_CHECKBUMP - check whether add'l bumping needs to occur, to prevent bump overflow

#ifdef CMDFIFO

#define SETCMDFIFOPTR(hwPtr)                    \
    if ((hwPtr) > CMDFIFOEND) {_asm int 1}      \
    CMDFIFOPTR = (hwPtr);


#ifdef AGP_CMDFIFO  //[

  #define CMDFIFO_CHECKBUMP(hwPtr, n)                                      \
          CMDFIFOBUMP += (n);                                              \
          if (CMDFIFOBUMP >= ((CMDFIFOEND - CMDFIFOSTART) / 32))           \
          {                                                                \
            P6FENCE;                                                       \
            CMDFIFOBUMP = (n);                                             \
            if (_FF(doAgpCF))                                              \
              MYAGPEPILOG(ppdev, (DWORD)hwPtr);                            \
          }

#else //][ AGP_CMDFIFO

  #define CMDFIFO_CHECKBUMP(hwPtr, n)  /*                                  \
  // uncomment all this if a Coppermine-esque OoOW issue comes up.         \
          CMDFIFOBUMP += (n);                                              \
          if (CMDFIFOBUMP >= ((CMDFIFOEND - CMDFIFOSTART) / 32))           \
          {                                                                \
            P6FENCE;                                                       \
            CMDFIFOBUMP = (n);                                             \
          } 
  */

#endif  //] AGP_CMDFIFO


#ifdef IS_32

  #define CMDFIFO_PROLOG(hwPtr)                  \
          FxU32 *(hwPtr)= (FxU32 *)CMDFIFOPTR;   \
          FxU32 hwIndex=0;

  #ifdef AGP_CMDFIFO
    #define CMDFIFO_EPILOG(hwPtr)                \
            hwPtr += hwIndex;                    \
            SETCMDFIFOPTR((FxU32)hwPtr);         \
            CMDFIFOSPACE -= hwIndex;             \
            if (_FF(doAgpCF))                    \
              MYAGPEPILOG(ppdev, (DWORD)hwPtr);  \
            hwIndex = 0;

    // BUMP is EPILOG without the ACCESS_EXIT, since may do in middle of blt, for example
    #define CMDFIFO_BUMP(hwPtr)                  \
            hwPtr += hwIndex;                    \
            SETCMDFIFOPTR((FxU32)hwPtr);         \
            CMDFIFOSPACE -= hwIndex;             \
            if (_FF(doAgpCF))                    \
              MYAGPEPILOG(ppdev, (DWORD)hwPtr);  \
            hwIndex = 0;

  #else // AGP_CMDFIFO
    #define CMDFIFO_EPILOG(hwPtr)                \
            hwPtr += hwIndex;                    \
            SETCMDFIFOPTR((FxU32)hwPtr);         \
            CMDFIFOSPACE -= hwIndex;             \
            hwIndex = 0;

    // BUMP is NO-OP if not AGP fifo
    #define CMDFIFO_BUMP(hwPtr)

  #endif // AGP_CMDFIFO

  #define CMDFIFO_SAVE(hwPtr)                    \
          hwPtr += hwIndex;                      \
          CMDFIFOSPACE -= hwIndex;               \
          SETCMDFIFOPTR((FxU32)hwPtr);
  
  #define CMDFIFO_RELOAD( hwPtr )                \
          hwPtr = (FxU32 *)CMDFIFOPTR;           \
          hwIndex = 0;

  #define CMDFIFO_CHECKROOM(hwPtr, n)            \
          CMDFIFO_CHECKBUMP(hwPtr, n)            \
          CMDFIFOSPACE -= hwIndex;               \
          hwPtr += hwIndex;                      \
          hwIndex = 0;                           \
          if ((DWORD)(n) > CMDFIFOSPACE)         \
          {                                      \
            fifo_MakeRoom(ppdev, &(hwPtr), (n)); \
          }

#else // IS_32

  #define CMDFIFO_PROLOG(hwPtr)                  \
          FxU32 (hwPtr);                         \
          FxU32 hwIndex;

  #define CMDFIFO_SETUP(hwPtr)                   \
          (hwPtr) = CMDFIFOPTR;                  \
          hwIndex = 0;                           \
          HW_ACCESS_ENTRY(hwPtr,ACCESS_2D);

  #ifdef AGP_CMDFIFO
    #define CMDFIFO_EPILOG(hwPtr)                \
            HW_ACCESS_EXIT(ACCESS_2D);           \
            hwPtr += (hwIndex << 2);             \
            CMDFIFOSPACE -= hwIndex;             \
            SETCMDFIFOPTR(hwPtr);                \
            if (_FF(doAgpCF))                    \
              MYAGPEPILOG(ppdev, (DWORD)hwPtr);  \
            hwIndex = 0;

    // BUMP is EPILOG without the ACCESS_EXIT, since may do in middle of blt, for example
    #define CMDFIFO_BUMP(hwPtr)                  \
            hwPtr += (hwIndex << 2);             \
            CMDFIFOSPACE -= hwIndex;             \
            SETCMDFIFOPTR(hwPtr);                \
            if (_FF(doAgpCF))                    \
              MYAGPEPILOG(ppdev, (DWORD)hwPtr);  \
            hwIndex = 0;

  #else // AGP_CMDFIFO
    #define CMDFIFO_EPILOG(hwPtr)                \
            HW_ACCESS_EXIT(ACCESS_2D);           \
            hwPtr += (hwIndex << 2);             \
            CMDFIFOSPACE -= hwIndex;             \
            SETCMDFIFOPTR(hwPtr);                \
            hwIndex = 0;

    // BUMP is NO-OP if not AGP fifo
    #define CMDFIFO_BUMP(hwPtr)

  #endif // AGP_CMDFIFO

  #define CMDFIFO_SAVE(hwPtr)                    \
          hwPtr += (hwIndex << 2);               \
          CMDFIFOSPACE -= hwIndex;               \
          SETCMDFIFOPTR(hwPtr);

  #define CMDFIFO_RELOAD(hwPtr)                  \
          (hwPtr) = CMDFIFOPTR;                  \
          hwIndex = 0;

  #define CMDFIFO_CHECKROOM(hwPtr, n)            \
          CMDFIFO_CHECKBUMP(hwPtr, n)            \
          CMDFIFOSPACE -= hwIndex;               \
          hwPtr += (hwIndex << 2);               \
          hwIndex = 0;                           \
          if( ((DWORD)(n))>CMDFIFOSPACE )        \
          {                                      \
            hwPtr = fifo_MakeRoom((hwPtr), (n)); \
          }

#endif // IS_32

#else // CMDFIFO

  #define CMDFIFO_PROLOG(hwptr)

#ifdef IS_32
  // HW access macros are explicit in DDraw and D3D
  #define CMDFIFO_SETUP(hwPtr)
  #define CMDFIFO_EPILOG(hwptr)
#else // IS_32
  // dd16 and minivdd hw access macros are implicit in CMDFIFO_SETUP and EPILOG
  #define CMDFIFO_SETUP(hwPtr)      HW_ACCESS_ENTRY(hwPtr,ACCESS_2D)
  #define CMDFIFO_EPILOG(hwptr)     HW_ACCESS_EXIT(ACCESS_2D)
#endif // IS_32

  #define CMDFIFO_SAVE(hwPtr)
  #define CMDFIFO_RELOAD(hwPtr)
  #define CMDFIFO_CHECKROOM(hwPtr, n)
  #define CMDFIFO_BUMP(hwPtr)

#endif // CMDFIFO

//----------------------------------------------------------------------------------//
//                         COMMAND FIFO PACKET MACROS                 
//----------------------------------------------------------------------------------//
// CMDFIFO_BUILD_MOP_PK0   - build a command FIFO packet #0, for MOP Cmd
// CMDFIFO_BUILD_PK1       - build a command FIFO packet #1, with 3D registers, for all chips
// CMDFIFO_BUILD_PK1CHIP   - build a command FIFO packet #1, with 3D registers, for specified chips
// CMDFIFO_BUILD_2DPK1     - build a command FIFO packet #1, with 2D registers
// CMDFIFO_BUILD_IOPK1     - build a command FIFO packet #1, with I/O registers
// CMDFIFO_BUILD_PK1R      - build a command FIFO packet #1, with 3D registers, specified by offset
// CMDFIFO_BUILD_PK2       - build a command FIFO packet #2
// CMDFIFO_BUILD_PK3       - build a command FIFO packet #3
// CMDFIFO_BUILD_PK4       - build a command FIFO packet #4
// CMDFIFO_BUILD_PK5       - build a command FIFO packet #5
// CMDFIFO_BUILD_PK6_0     - build a command FIFO packet #6, word #0
// CMDFIFO_BUILD_PK6_1     - build a command FIFO packet #6, word #1
// CMDFIFO_BUILD_PK6_2     - build a command FIFO packet #6, word #2
// CMDFIFO_BUILD_PK6_3     - build a command FIFO packet #6, word #3
// CMDFIFO_BUILD_PK6_4     - build a command FIFO packet #6, word #4

  #define PH0_SIZE  1
  #define MOP_SIZE  PH0_SIZE
  #define PH1_SIZE  1
  #define PH2_SIZE  1
  #define PH3_SIZE  1
  #define PH4_SIZE  1
  #define PH5_SIZE  2
  #define PH6_SIZE  1
  #define PH7_SIZE  1

// regoffsetof             - get offset, in Dwords, of register from start of group
  #define regoffsetof(s,m)  ((unsigned int)&(((s *)0)->m)>>2)

  #define CMDFIFO_BUILD_MOP_PK0( mopBits )\
	(((mopBits) << SST_CP_PKT0_MOP_DATA_SHIFT) |\
	(SSTCP_PKT0_MOP << SSTCP_PKT0_FUNC_SHIFT) |\
	SSTCP_PKT0)

  #define CMDFIFO_BUILD_2DPK1( n, regName )\
    (SSTCP_PKT1_2D |\
	(((FxU32)regoffsetof(Sst2WxRegs, regName)) << SSTCP_REGBASE_SHIFT)|\
    ((n) << SSTCP_PKT1_2D_NWORDS_SHIFT) |\
	SSTCP_INC |\
	SSTCP_PKT1)

  #define CMDFIFO_BUILD_PK1( n, unit, regName )\
    (((unit) << SSTCP_PKT1_3D_UNIT_SHIFT) |\
	(((FxU32)regoffsetof(Sst2Regs, regName)) << SSTCP_REGBASE_SHIFT)|\
	((n) << SSTCP_PKT1_3D_NWORDS_SHIFT) |\
    SSTCP_INC |\
	SSTCP_PKT1)

  #define CMDFIFO_BUILD_PAL_PK1( n, unit, regBase )\
    (((unit) << SSTCP_PKT1_3D_UNIT_SHIFT) |\
	((regBase) << SSTCP_REGBASE_SHIFT) |\
    ((n) << SSTCP_PKT1_3D_NWORDS_SHIFT) |\
	SSTCP_INC |\
	SSTCP_PKT1)

  #define CMDFIFO_BUILD_IOPK1(n, inc, regName, chip) \
    (((n) << 16) | ((inc) << 15) | ((offsetof(SstIORegs, regName) / 4) << 3) | SSTCP_PKT1 )

  #define CMDFIFO_BUILD_PK1R(n, inc, reg, chip) \
    (((n) << 16) | ((inc) << 15) | (reg << 3) | SSTCP_PKT1 )

  #define CMDFIFO_BUILD_PK2(mask)\
    ((mask) | SSTCP_PKT2 )

  #define CMDFIFO_BUILD_PK3( AIDX,BIDX,CIDX,DIDX, LOAD_FIELD,PRIMTYPE,DRAW_FIELD)\
		   ((AIDX << SSTCP_PKT3_IDX_A_SHIFT) |\
			(BIDX << SSTCP_PKT3_IDX_B_SHIFT) |\
			(CIDX << SSTCP_PKT3_IDX_C_SHIFT) |\
			(DIDX << SSTCP_PKT3_IDX_D_SHIFT) |\
			LOAD_FIELD |\
			PRIMTYPE |\
			DRAW_FIELD |\
			SSTCP_PKT3)

  #define CMDFIFO_SIMPLE_TRI \
	CMDFIFO_BUILD_PK3(0,1,2,0,SSTCP_PKT3_LOAD_A|SSTCP_PKT3_LOAD_B|SSTCP_PKT3_LOAD_C,SSTCP_PKT3_TRIANGLE,SSTCP_PKT3_DRAW_ABC)

  #define CMDFIFO_BUILD_PK4( mask, unit, regName )\
    (((unit) << SSTCP_PKT4_3D_UNIT_SHIFT) |\
	(((FxU32)regoffsetof(Sst2Regs, regName)) << SSTCP_REGBASE_SHIFT)|\
    ((mask) << SSTCP_PKT4_3D_MASK_SHIFT) |\
	SSTCP_PKT4)

  /// unmodified H5 CMDFIFO_BUILD macros follow

  #define CMDFIFO_BUILD_PK5(n, byteEnableW2, byteEnableWN, memPort) \
    ((n << SSTCP_PKT5_NWORDS_SHIFT) | (byteEnableW2 << SSTCP_PKT5_BYTEN_W2_SHIFT) \
    | (byteEnableWN << SSTCP_PKT5_BYTEN_WN_SHIFT) | (memPort) | SSTCP_PKT5 )

  #define CMDFIFO_BUILD_PK6_0(n, memPort) \
    ((n << SSTCP_PKT6_NBYTES_SHIFT) | (memPort) | SSTCP_PKT6 )

  #define CMDFIFO_BUILD_PK6_1(physAddrLow) \
    ((physAddrLow) & SSTCP_PKT6_SRC_BASELOW )

  #define CMDFIFO_BUILD_PK6_2(physAddrHi, srcStride, srcWidth) \
    ((((physAddrHi)<<SSTCP_PKT6_SRC_BASEHIGH_SHIFT) & SSTCP_PKT6_SRC_BASEHIGH) \
    | (((srcStride)<<SSTCP_PKT6_SRC_STRIDE_SHIFT) & SSTCP_PKT6_SRC_STRIDE) \
    |  ((srcWidth) & SSTCP_PKT6_SRC_WIDTH))

  #define CMDFIFO_BUILD_PK6_3(dstAddr) \
    ((dstAddr) & SSTCP_PKT6_FRAME_BUFFER_OFFSET )

  #define CMDFIFO_BUILD_PK6_4(dstStride) \
    (((dstStride) & SSTCP_PKT6_DST_STRIDE) )

  /// sage CMDFIFO_BUILD macros

  #define CMDFIFO_BUILD_PK6_a(indA, indB, indC, loadA, loadB, loadC, cmd, ind) \
    ( ((indA   & kPkt6aAIndexMask ) << kPkt6aAIndexShift ) \
    | ((indB   & kPkt6aBIndexMask ) << kPkt6aBIndexShift ) \
    | ((indC   & kPkt6aCIndexMask ) << kPkt6aCIndexShift ) \
    | ((cmd    & kPkt6aCmdMask    ) << kPkt6aCmdShift    ) \
    | ((loadA  & kPkt6aALoadMask  ) << kPkt6aALoadShift  ) \
    | ((loadB  & kPkt6aBLoadMask  ) << kPkt6aBLoadShift  ) \
    | ((loadC  & kPkt6aCLoadMask  ) << kPkt6aCLoadShift  ) \
    | ((ind    & kPkt6IndirectMask) << kPkt6IndirectShift) \
    | ((kPkt6a & kPkt6SubTypeMask ) << kPkt6SubTypeShift ) \
    | ((kPkt6  & kPktTypeMask     ) << kPktTypeShift     ) )

  #define CMDFIFO_BUILD_PK6_b(alloc, wo, n, cmd, ind)      \
    ( ((alloc  & kPkt6bAllocMask  ) << kPkt6bAllocShift  ) \
    | ((wo     & kPkt6bWindingMask) << kPkt6bWindingShift) \
    | ((n      & kPkt6bCnt6bMask  ) << kPkt6bCnt6bShift  ) \
    | ((cmd    & kPkt6bCmdMask    ) << kPkt6bCmdShift    ) \
    | ((ind    & kPkt6IndirectMask) << kPkt6IndirectShift) \
    | ((kPkt6b & kPkt6SubTypeMask ) << kPkt6SubTypeShift ) \
    | ((kPkt6  & kPktTypeMask     ) << kPktTypeShift     ) )

  #define CMDFIFO_BUILD_PK7_w(n, reg, inc, sel)            \
    ( ((n      & kPkt7wCountMask  ) << kPkt7wCountShift  ) \
    | ((reg    & kPkt7wAddrMask   ) << kPkt7wAddrShift   ) \
    | ((inc    & kPkt7wAutoIncMask) << kPkt7wAutoIncShift) \
    | ((sel    & kPkt7wSelMask    ) << kPkt7wSelShift    ) \
    | ((kPkt7w & kPkt7SubTypeMask ) << kPkt7SubTypeShift ) \
    | ((kPkt7  & kPktTypeMask     ) << kPktTypeShift     ) )

  #define CMDFIFO_BUILD_PK7_m(n)                           \
    ( ((n      & kPkt7mCountMask  ) << kPkt7mCountShift  ) \
    | ((kPkt7m & kPkt7SubTypeMask ) << kPkt7SubTypeShift ) \
    | ((kPkt7  & kPktTypeMask     ) << kPktTypeShift     ) )

  #define CMDFIFO_BUILD_PK7_r(n, reg, inc, sel, stall)     \
    ( ((n      & kPkt7rCountMask  ) << kPkt7rCountShift  ) \
    | ((reg    & kPkt7rAddrMask   ) << kPkt7rAddrShift   ) \
    | ((inc    & kPkt7rAutoIncMask) << kPkt7rAutoIncShift) \
    | ((sel    & kPkt7rSelMask    ) << kPkt7rSelShift    ) \
    | ((stall  & kPkt7rStallMask  ) << kPkt7rStallShift  ) \
    | ((kPkt7r & kPkt7SubTypeMask ) << kPkt7SubTypeShift ) \
    | ((kPkt7  & kPktTypeMask     ) << kPktTypeShift     ) )


//----------------------------------------------------------------------------------//
//                         WRITE COMBINE FLUSH MACRO
//----------------------------------------------------------------------------------//

#undef P6FENCE

#ifdef WRITECOMBINE

#ifdef IS_16

  DWORD p6FenceVar;
  #define  P6FENCE {_asm _emit 66h _asm xchg ax, word ptr p6FenceVar}

#else // IS_16

  DWORD p6FenceVar;
  #define  P6FENCE {_asm xchg eax, p6FenceVar}

#endif // IS_16

#else // WRITECOMBINE

  #define P6FENCE

#endif // WRITECOMBINE


//----------------------------------------------------------------------------------//
//                         AGP COMMAND FIFO MACROS
//----------------------------------------------------------------------------------//

#ifdef AGP_CMDFIFO

#ifdef IS_32

  #define BUMPAGP(arg1,arg2) bumpAgp(arg1,arg2)
  #define FLUSHAGP(arg1) flushAgp(arg1)
  #define MYWRAPAGP(arg1,arg2) myWrapAgp(arg1,arg2)
  #define MYAGPEPILOG(arg1,arg2) myAgpEpilog(arg1,arg2)

#else // IS_32

  #define BUMPAGP(arg1,arg2) bumpAgp(arg2)
  #define FLUSHAGP(arg1) flushAgp()
  #define MYWRAPAGP(arg1,arg2) myWrapAgp(arg2)
  #define MYAGPEPILOG(arg1,arg2) myAgpEpilog(arg2)

#endif // IS_32

  void BUMPAGP(NT9XDEVICEDATA * ppdev, FxU32 nWords);
  void FLUSHAGP(NT9XDEVICEDATA * ppdev);
  void MYWRAPAGP(NT9XDEVICEDATA * ppdev, FxU32 hwPtr);
  void MYAGPEPILOG(NT9XDEVICEDATA * ppdev, FxU32 hwPtr);

#endif // AGP_CMDFIFO

#define AGP_FLUSH

#ifdef IS_32
#ifdef CMDFIFO
#ifdef AGP_CMDFIFO

  #undef AGP_FLUSH
  #define AGP_FLUSH                                        \
          {                                                \
            if (_FF(doAgpCF))                              \
              MYAGPEPILOG(ppdev, (DWORD)CMDFIFOPTR);       \
          }

#endif // AGP_CMDFIFO
#endif // CMDFIFO
#endif // IS_32


//----------------------------------------------------------------------------------//
//                         COMMAND FIFO INLINE FUNCTIONS
//----------------------------------------------------------------------------------//

// fifo_Wrap         - wrap back to the beginning of the command fifo
// fifo_MakeRoom     - make room for data in the command fifo

#define FIFO_MIN(A,B) ((A) < (B) ? (A) : (B))
#ifdef CMDFIFO

#ifdef IS_32

  static INLINE FxU32* fifo_Wrap(GLOBALDATA * ppdev, FxU32 *hwPtr )
  {
    FxU32 *rdPtr;
#ifdef SLI
    DWORD i;
    DWORD dwChipMask;
    DWORD dwsliBroadcast;
    FxU32 *rdPtrMin;

    if (IS_SAGE_ACTIVE)
    {
        do
        {
           rdPtr = (FxU32*)(LOAD_SAGE_CMDFIFO_RDPTR(_FF(lpGeCfeRegs)) + CMDFIFOOFFSET);
        } while( (rdPtr > hwPtr) || (rdPtr == (FxU32 *)CMDFIFOSTART) );
    }
    else
    {
        dwsliBroadcast = GET(ghwIO->sliBroadcast);
        // If SLI is enabled then check all Chip's CMDFIFO
        if (_FF(gdiFlags) & SDATA_GDIFLAGS_SLI_MASTER)
           {
           // this is the case where only some units are selected
           if (dwsliBroadcast & SST_SLI_ENDBG_MEMBASE1)         
              dwChipMask = dwsliBroadcast >> SST_SLI_EN0_MEMBASE1_SHIFT;
           else
              dwChipMask = _FF(dwSlaveMask) | BIT(_FF(dwChipID));
           }
        else 
          dwChipMask = BIT(_FF(dwChipID));

        rdPtrMin = (FxU32 *)CMDFIFOEND;
        for (i=0; i<_FF(dwNumUnits); i++)
           {
           if (dwChipMask & BIT(i))
             {
             SETDW(ghwIO->sliBroadcast, (i << SST_SLI_RENID_MEMBASE0_SHIFT) | (BIT(i) << SST_SLI_WEN0_MEMBASE0_SHIFT));
             do
                {
                   rdPtr = (FxU32*)(LOAD_CMDFIFO_RDPTR(ghwAC) + CMDFIFOOFFSET);
                } 
             while( (rdPtr > hwPtr) || (rdPtr == (FxU32 *)CMDFIFOSTART) );
             rdPtrMin = FIFO_MIN(rdPtrMin, rdPtr);
             }
           }
        rdPtr = rdPtrMin;
        SETDW(ghwIO->sliBroadcast, dwsliBroadcast);
    }
#else
    do
    {
      if (IS_SAGE_ACTIVE)
         rdPtr = (FxU32*)(LOAD_SAGE_CMDFIFO_RDPTR(_FF(lpGeCfeRegs)) + CMDFIFOOFFSET);
      else
         rdPtr = (FxU32*)(LOAD_CMDFIFO_RDPTR(ghwAC) + CMDFIFOOFFSET);
    } while( (rdPtr > hwPtr) || (rdPtr == (FxU32 *)CMDFIFOSTART) );
#endif

    *(volatile FxU32 *)(hwPtr) = CMDFIFOJMP;
    #ifdef AGP_CMDFIFO
      if (_FF(doAgpCF))
        *((volatile FxU32 *)(hwPtr)+1) = CMDFIFOJMP2;
    #endif // AGP_CMDFIFO

    P6FENCE;

    #ifdef AGP_CMDFIFO
      if (_FF(doAgpCF))
        MYWRAPAGP(ppdev, (DWORD)hwPtr);
    #endif // AGP_CMDFIFO

    hwPtr = (FxU32 *)CMDFIFOSTART;

//RYAN@PRS15383, begin
    if (IS_SAGE_ACTIVE)
      rdPtr = (FxU32*)(LOAD_SAGE_CMDFIFO_RDPTR(_FF(lpGeCfeRegs)) + CMDFIFOOFFSET);
    else
      rdPtr = (FxU32*)(LOAD_CMDFIFO_RDPTR(ghwAC) + CMDFIFOOFFSET);

    if (rdPtr <= hwPtr)
      CMDFIFOSPACE = (FxU32)(CMDFIFOEND - (FxU32)hwPtr)/4;
    else
      CMDFIFOSPACE = (FxU32)((rdPtr - hwPtr)/4 - 1);
//RYAN@PRS15383, end
  
    return( hwPtr );
  }

  static INLINE void fifo_MakeRoom(GLOBALDATA * ppdev, FxU32 **hwPtr, FxU32 n )
  {
    FxU32 *rdPtr;
    FxU32 resetIdleCount = 0;

#ifdef SLI
    DWORD i;
    DWORD dwChipMask;
    DWORD dwsliBroadcast;
    FxU32 dwCmdFifoSpace;
#endif

    if ( (*hwPtr + n) > (FxU32 *)CMDFIFOEND )
      *hwPtr = fifo_Wrap(ppdev, *hwPtr);

#ifdef SLI    
    if (IS_SAGE_ACTIVE)
    {
        while (CMDFIFOSPACE < n)
        {
          rdPtr = (FxU32*)(LOAD_SAGE_CMDFIFO_RDPTR(_FF(lpGeCfeRegs)) + CMDFIFOOFFSET);

          if (rdPtr <= *hwPtr)
          {
            CMDFIFOSPACE = (FxU32)(CMDFIFOEND - (FxU32)*hwPtr)/4;
          }
          else
          {
            CMDFIFOSPACE = (FxU32)((rdPtr - *hwPtr)/4 - 1);
          }
        }
    }
    else
    {
        dwsliBroadcast = GET(ghwIO->sliBroadcast);
        // If SLI is enabled then check all Chip's CMDFIFO
        if (_FF(gdiFlags) & SDATA_GDIFLAGS_SLI_MASTER)
           {
           // this is the case where only some units are selected
           if (dwsliBroadcast & SST_SLI_ENDBG_MEMBASE1)         
              dwChipMask = dwsliBroadcast >> SST_SLI_EN0_MEMBASE1_SHIFT;
           else
              dwChipMask = _FF(dwSlaveMask) | BIT(_FF(dwChipID));
           }
        else 
          dwChipMask = BIT(_FF(dwChipID));

        for (i=0; i<_FF(dwNumUnits); i++)
           {
           if (dwChipMask & BIT(i))
             {
             dwCmdFifoSpace = CMDFIFOSPACE;
             SETDW(ghwIO->sliBroadcast, (i << SST_SLI_RENID_MEMBASE0_SHIFT) | (BIT(i) << SST_SLI_WEN0_MEMBASE0_SHIFT));
             while (dwCmdFifoSpace < n)
                {
                rdPtr = (FxU32*)(LOAD_CMDFIFO_RDPTR(ghwAC) + CMDFIFOOFFSET);

                if (rdPtr <= *hwPtr)
                   {
                   dwCmdFifoSpace = (FxU32)(CMDFIFOEND - (FxU32)*hwPtr)/4;
                   }
                else
                   {
                   dwCmdFifoSpace = (FxU32)((rdPtr - *hwPtr)/4 - 1);
                   }
                }
             }
           }
        CMDFIFOSPACE = dwCmdFifoSpace;
        SETDW(ghwIO->sliBroadcast, dwsliBroadcast);
    }
#else
    while (CMDFIFOSPACE < n)
    {
      if (IS_SAGE_ACTIVE)
         rdPtr = (FxU32*)(LOAD_SAGE_CMDFIFO_RDPTR(_FF(lpGeCfeRegs)) + CMDFIFOOFFSET);
      else
         rdPtr = (FxU32*)(LOAD_CMDFIFO_RDPTR(ghwAC) + CMDFIFOOFFSET);

      if (rdPtr <= *hwPtr)
      {
        CMDFIFOSPACE = (FxU32)(CMDFIFOEND - (FxU32)*hwPtr)/4;
      }
      else
      {
        CMDFIFOSPACE = (FxU32)((rdPtr - *hwPtr)/4 - 1);
      }
    }
#endif
  }

#else // IS_32

  /***************************************************************
   * To get the 16-bit compiler to emit the right code for P6FENCE
   * I had to turn optimization off for this routine.  In general
   * I suspect anywhere P6FENCE is used this will need to be done. 
   ***************************************************************/

  #ifdef IS_16
  #pragma optimize("", off)
  #endif

  static INLINE FxU32 fifo_Wrap(FxU32 hwPtr)
  {
    FxU32 rdPtr;

#ifdef SLI
    DWORD i;
    DWORD dwChipMask;
    DWORD dwsliBroadcast;
    FxU32 rdPtrMin;

    DEBUG_FIX;

    if (IS_SAGE_ACTIVE)
    {
        do
        {
          rdPtr = LOAD_SAGE_CMDFIFO_RDPTR(_FF(lpGeCfeRegs)) + CMDFIFOOFFSET;
        } while ((rdPtr > hwPtr) || (rdPtr == CMDFIFOSTART));
    }
    else
    {
        dwsliBroadcast = GET(_FF(lpIOregs)->sliBroadcast);
        // If SLI is enabled then check all Chip's CMDFIFO
        if (_FF(gdiFlags) & SDATA_GDIFLAGS_SLI_MASTER)
           {
           // this is the case where only some units are selected
           if (dwsliBroadcast & SST_SLI_ENDBG_MEMBASE1)         
              dwChipMask = dwsliBroadcast >> SST_SLI_EN0_MEMBASE1_SHIFT;
           else
              dwChipMask = _FF(dwSlaveMask) | BIT(_FF(dwChipID));
           }
        else 
          dwChipMask = BIT(_FF(dwChipID));

        rdPtrMin = CMDFIFOEND;
        for (i=0; i<_FF(dwNumUnits); i++)
           {
           if (dwChipMask & BIT(i))
             {
             SETDW(_FF(lpIOregs)->sliBroadcast, (i << SST_SLI_RENID_MEMBASE0_SHIFT) | (BIT(i) << SST_SLI_WEN0_MEMBASE0_SHIFT));
             do
                {
                rdPtr = LOAD_CMDFIFO_RDPTR(_FF(lpCRegs)) + CMDFIFOOFFSET;
                } 
             while( (rdPtr > hwPtr) || (rdPtr == CMDFIFOSTART) );
             rdPtrMin = FIFO_MIN(rdPtrMin, rdPtr);
             }
           }
        rdPtr = rdPtrMin;
        SETDW(_FF(lpIOregs)->sliBroadcast, dwsliBroadcast);
    }
#else
    DEBUG_FIX;

    do
    {
      if (IS_SAGE_ACTIVE)
         rdPtr = LOAD_SAGE_CMDFIFO_RDPTR(_FF(lpGeCfeRegs)) + CMDFIFOOFFSET;
      else
         rdPtr = LOAD_CMDFIFO_RDPTR(_FF(lpCRegs)) + CMDFIFOOFFSET;
    } while ((rdPtr > hwPtr) || (rdPtr == CMDFIFOSTART));
#endif
    #ifndef THUNK32
      h3WRITE(NULL, (DWORD *)(hwPtr), CMDFIFOJMP); 
      #ifdef AGP_CMDFIFO
        if (_FF(doAgpCF))
          h3WRITE(NULL, ((DWORD *)(hwPtr))+1, CMDFIFOJMP2); 
      #endif // AGP_CMDFIFO 
    #else // !THUNK32
      *(volatile FxU32 *)(hwPtr) = CMDFIFOJMP;
      #ifdef AGP_CMDFIFO
        if (_FF(doAgpCF))
          *((volatile FxU32 *)(hwPtr)+1) = CMDFIFOJMP2;
      #endif // AGP_CMDFIFO 
    #endif // !THUNK32

    P6FENCE;

    #ifdef AGP_CMDFIFO
      if (_FF(doAgpCF))
        MYWRAPAGP(ppdev, (DWORD)hwPtr);
    #endif // AGP_CMDFIFO 

    hwPtr = CMDFIFOSTART;

//RYAN@PRS15383, begin
    if (IS_SAGE_ACTIVE)
      rdPtr = LOAD_SAGE_CMDFIFO_RDPTR(_FF(lpGeCfeRegs)) + CMDFIFOOFFSET;
    else
      rdPtr = LOAD_CMDFIFO_RDPTR(_FF(lpCRegs)) + CMDFIFOOFFSET;

    if (rdPtr <= hwPtr)
      CMDFIFOSPACE = (FxU32)(CMDFIFOEND - hwPtr)/4;
    else
      CMDFIFOSPACE = (FxU32)((rdPtr - hwPtr)/4 - 1);
//RYAN@PRS15383, end

    return (hwPtr);
  }

  #ifdef IS_16
  #pragma optimize("", on)
  #endif

  static INLINE FxU32 fifo_MakeRoom( DWORD hwPtr, FxU32 n )
  {
    DWORD rdPtr;
#ifdef SLI
    DWORD i;
    DWORD dwChipMask;
    DWORD dwsliBroadcast;
    FxU32 dwCmdFifoSpace;
    int nFirst;
#endif

    DEBUG_FIX;

    if ((hwPtr + (n*4)) >= CMDFIFOEND)
    {
      hwPtr = fifo_Wrap(hwPtr);
    }

#ifdef SLI
    if (IS_SAGE_ACTIVE)
    {
        while (CMDFIFOSPACE < n)
        {
          rdPtr = LOAD_SAGE_CMDFIFO_RDPTR(_FF(lpGeCfeRegs)) + CMDFIFOOFFSET;
          if (rdPtr <= hwPtr)
          {
            if ((CMDFIFOSPACE = (FxU32)((CMDFIFOEND - hwPtr))/4) < n)
            {
              hwPtr = fifo_Wrap(hwPtr);
            }
          }
          else 
          {
            CMDFIFOSPACE = (FxU32)((rdPtr - hwPtr)/4) - 1;
          }
        }
    }
    else
    {
        nFirst = TRUE;
        dwsliBroadcast = GET(_FF(lpIOregs)->sliBroadcast);
        // If SLI is enabled then check all Chip's CMDFIFO
        if (_FF(gdiFlags) & SDATA_GDIFLAGS_SLI_MASTER)
           {
           // this is the case where only some units are selected
           if (dwsliBroadcast & SST_SLI_ENDBG_MEMBASE1)         
              dwChipMask = dwsliBroadcast >> SST_SLI_EN0_MEMBASE1_SHIFT;
           else
              dwChipMask = _FF(dwSlaveMask) | BIT(_FF(dwChipID));
           }
        else 
          dwChipMask = BIT(_FF(dwChipID));

        for (i=0; i<_FF(dwNumUnits); i++)
           {
           if (dwChipMask & BIT(i))
             {
             dwCmdFifoSpace = CMDFIFOSPACE;
             SETDW(_FF(lpIOregs)->sliBroadcast, (i << SST_SLI_RENID_MEMBASE0_SHIFT) | (BIT(i) << SST_SLI_WEN0_MEMBASE0_SHIFT));
             while (dwCmdFifoSpace < n)
                {
                rdPtr = LOAD_CMDFIFO_RDPTR(_FF(lpCRegs)) + CMDFIFOOFFSET;

                if (rdPtr <= hwPtr)
                   {
                   if ((dwCmdFifoSpace = (FxU32)((CMDFIFOEND - hwPtr))/4) < n)
                      {
                      if (nFirst)
                         {
                         nFirst = FALSE;
                         hwPtr = fifo_Wrap(hwPtr);
                         }
                      }
                   }
                else
                   {
                   dwCmdFifoSpace = (FxU32)((rdPtr - hwPtr)/4) - 1;
                   }
                }
             }
           }
        CMDFIFOSPACE = dwCmdFifoSpace;
        SETDW(_FF(lpIOregs)->sliBroadcast, dwsliBroadcast);
    }
#else       
    while (CMDFIFOSPACE < n)
    {
      if (IS_SAGE_ACTIVE)
         rdPtr = LOAD_SAGE_CMDFIFO_RDPTR(_FF(lpGeCfeRegs)) + CMDFIFOOFFSET;
      else
         rdPtr = LOAD_CMDFIFO_RDPTR(_FF(lpCRegs)) + CMDFIFOOFFSET;
      if (rdPtr <= hwPtr)
      {
        if ((CMDFIFOSPACE = (FxU32)((CMDFIFOEND - hwPtr))/4) < n)
        {
          hwPtr = fifo_Wrap(hwPtr);
        }
      }
      else 
      {
        CMDFIFOSPACE = (FxU32)((rdPtr - hwPtr)/4) - 1;
      }
    }
#endif

    return (hwPtr);
  }

#endif // IS_32

#endif // CMDFIFO


//----------------------------------------------------------------------------------//
//                         CLEAR COMMANDEX REGISTER MACROS
//----------------------------------------------------------------------------------//

#ifdef THUNK32

#ifdef CMDFIFO

  #define CLEAR_COMMAND_EX(cf)                   \
          if (_FF(ClearCommandEx))               \
          {                                      \
            CMDFIFO_CHECKROOM(cf, 2);            \
            SETPH(cf, SSTCP_PKT2 | commandExBit);\
            SET(cf, _FF(lpGRegs->commandEx), 0); \
            _FF(ClearCommandEx) = 0;             \
          }

#else // CMDFIFO

  #define CLEAR_COMMAND_EX(cf)                   \
          if (_FF(ClearCommandEx))               \
          {                                      \
            SET(cf, _FF(lpGRegs->commandEx), 0); \
            _FF(ClearCommandEx)=0;               \
          }

#endif // CMDFIFO

#endif // THUNK32

#endif //__FIFOMGR_H__
