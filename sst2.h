#pragma once

#define SST_TA_CONSTANT_COLOR0_RED_SHIFT    0
#define SST_TA_CONSTANT_COLOR0_GREEN_SHIFT  0
#define SST_TA_CONSTANT_COLOR0_BLUE_SHIFT   16
#define SST_TA_CONSTANT_COLOR0_ALPHA_SHIFT  16

#define SST_TA_INV_NONE                     0
#define SST_TA_INV_MINUS                    1
#define SST_TA_INV_MINUS_HALF               2
#define SST_TA_INV_ONE_MINUS                3

#define SST_TA_TCC_ZERO                     0
#define SST_TA_TCC_CPREV                    1
#define SST_TA_TCC_CITER                    2   // From Patents
#define SST_TA_TCC_CTEX                     3
#define SST_TA_TCC_C0                       4
#define SST_TA_TCC_LOD2TCU                  5   // From Patents
#define SST_TA_TCC_CREG                     6   //
#define SST_TA_TCC_APREV                    7   // From Patents
#define SST_TA_TCC_AITER                    8   // From Patents
#define SST_TA_TCC_ATEX                     9
#define SST_TA_TCC_A0                       10  // From Patents
#define SST_TA_TCC_AREG                     11
#define SST_TA_TCC_A_SELECT                 0x0000000F
#define SST_TA_TCC_B_SELECT                 0x000000F0
#define SST_TA_TCC_C_SELECT                 0x00000F00
#define SST_TA_TCC_D_SELECT                 0x0000F000
#define SST_TA_TCC_A_SELECT_SHIFT           0
#define SST_TA_TCC_B_SELECT_SHIFT           4
#define SST_TA_TCC_C_SELECT_SHIFT           8
#define SST_TA_TCC_D_SELECT_SHIFT           12
#define SST_TA_TCC_A_MODE                   0x00030000
#define SST_TA_TCC_B_MODE                   0x000C0000
#define SST_TA_TCC_C_MODE                   0x00300000
#define SST_TA_TCC_D_MODE                   0x00C00000
#define SST_TA_TCC_A_MODE_SHIFT             16
#define SST_TA_TCC_B_MODE_SHIFT             18
#define SST_TA_TCC_C_MODE_SHIFT             20
#define SST_TA_TCC_D_MODE_SHIFT             22
#define SST_TA_TCC_OUT_CLAMP                0x01000000
#define SST_TA_TCC_EN_MIN_MAX               0x02000000
#define SST_TA_TCC_TEX_SHIFT                0x1C000000
#define SST_TA_TCC_TEX_SHIFT_SHIFT          26
#define SST_TA_TCC_POP_DWALL                0x40000000

#define SST_TA_TCA_ZERO                     0
#define SST_TA_TCA_APREV                    1
#define SST_TA_TCA_AITER                    2   // From Patents
#define SST_TA_TCA_ATEX                     3
#define SST_TA_TCA_A0                       4
#define SST_TA_TCA_LOD2TCU                  5   // From Patents
#define SST_TA_TCA_AREG                     6
#define SST_TA_TCA_A_SELECT                 0x0000000F
#define SST_TA_TCA_B_SELECT                 0x000000F0
#define SST_TA_TCA_C_SELECT                 0x00000F00
#define SST_TA_TCA_D_SELECT                 0x0000F000
#define SST_TA_TCA_A_SELECT_SHIFT           0
#define SST_TA_TCA_B_SELECT_SHIFT           4
#define SST_TA_TCA_C_SELECT_SHIFT           8
#define SST_TA_TCA_D_SELECT_SHIFT           12
#define SST_TA_TCA_A_MODE                   0x00030000
#define SST_TA_TCA_B_MODE                   0x000C0000
#define SST_TA_TCA_C_MODE                   0x00300000
#define SST_TA_TCA_D_MODE                   0x00C00000
#define SST_TA_TCA_A_MODE_SHIFT             16
#define SST_TA_TCA_B_MODE_SHIFT             18
#define SST_TA_TCA_C_MODE_SHIFT             20
#define SST_TA_TCA_D_MODE_SHIFT             22
#define SST_TA_TCA_OUT_CLAMP                0x01000000
#define SST_TA_TCA_EN_MIN_MAX               0x02000000
#define SST_TA_TCA_TEX_SHIFT                0x1C000000
#define SST_TA_TCA_TEX_SHIFT_SHIFT          26

#define SST_TA_CCC_ZERO                     0
#define SST_TA_CCC_CPREV                    1
#define SST_TA_CCC_CITER                    2
#define SST_TA_CCC_CTEX                     3
#define SST_TA_CCC_C0                       4   // From Patents
#define SST_TA_CCC_C1                       5
#define SST_TA_CCC_CTCU                     6
#define SST_TA_CCC_CTCUSUM                  7
#define SST_TA_CCC_CREG                     8
#define SST_TA_CCC_APREV                    9
#define SST_TA_CCC_AITER                    10  // From Patents
#define SST_TA_CCC_ATEX                     11
#define SST_TA_CCC_A0                       12  // From Patents
#define SST_TA_CCC_A1                       13  // From Patents
#define SST_TA_CCC_ATCU                     14  // From Patents
#define SST_TA_CCC_Z                        15  // From Patents
#define SST_TA_CCC_AREG                     16
#define SST_TA_CCC_A_SELECT                 0x0000001F
#define SST_TA_CCC_B_SELECT                 0x000003E0
#define SST_TA_CCC_C_SELECT                 0x00007C00
#define SST_TA_CCC_D_SELECT                 0x000F8000
#define SST_TA_CCC_A_SELECT_SHIFT           0
#define SST_TA_CCC_B_SELECT_SHIFT           5
#define SST_TA_CCC_C_SELECT_SHIFT           10
#define SST_TA_CCC_D_SELECT_SHIFT           15
#define SST_TA_CCC_A_MODE                   0x00300000
#define SST_TA_CCC_B_MODE                   0x00C00000
#define SST_TA_CCC_C_MODE                   0x03000000
#define SST_TA_CCC_D_MODE                   0x0C000000
#define SST_TA_CCC_A_MODE_SHIFT             20
#define SST_TA_CCC_B_MODE_SHIFT             22
#define SST_TA_CCC_C_MODE_SHIFT             24
#define SST_TA_CCC_D_MODE_SHIFT             26
#define SST_TA_CCC_OUT_CLAMP                0x10000000
#define SST_TA_CCC_EN_MIN_MAX               0x20000000
#define SST_TA_CCC_OVERRIDE_ATEX            0x40000000
#define SST_TA_CCC_A_ZERO                   0x80000000

#define SST_TA_CCA_ZERO                     0
#define SST_TA_CCA_APREV                    1
#define SST_TA_CCA_AITER                    2
#define SST_TA_CCA_ATEX                     3
#define SST_TA_CCA_A0                       4   // From Patents
#define SST_TA_CCA_A1                       5   // From Patents
#define SST_TA_CCA_ATCU                     6
#define SST_TA_CCA_CTCUSUM                  7
#define SST_TA_CCA_Z                        8   // From Patents
#define SST_TA_CCA_AREG                     9
#define SST_TA_CCA_A_SELECT                 0x0000000F
#define SST_TA_CCA_B_SELECT                 0x000000F0
#define SST_TA_CCA_C_SELECT                 0x00000F00
#define SST_TA_CCA_D_SELECT                 0x0000F000
#define SST_TA_CCA_A_SELECT_SHIFT           0
#define SST_TA_CCA_B_SELECT_SHIFT           4
#define SST_TA_CCA_C_SELECT_SHIFT           8
#define SST_TA_CCA_D_SELECT_SHIFT           12
#define SST_TA_CCA_A_MODE                   0x00030000
#define SST_TA_CCA_B_MODE                   0x000C0000
#define SST_TA_CCA_C_MODE                   0x00300000
#define SST_TA_CCA_D_MODE                   0x00C00000
#define SST_TA_CCA_A_MODE_SHIFT             16
#define SST_TA_CCA_B_MODE_SHIFT             18
#define SST_TA_CCA_C_MODE_SHIFT             20
#define SST_TA_CCA_D_MODE_SHIFT             22
#define SST_TA_CCA_OUT_CLAMP                0x01000000
#define SST_TA_CCA_EN_MIN_MAX               0x02000000
#define SST_TA_CCA_EN_ALPHA_MASK            0x04000000
#define SST_TA_CCA_A_ZERO                   0x08000000

#define TCC(a,b,c,d) ( \
    (SST_TA_TCC_##a)<<SST_TA_TCC_A_SELECT_SHIFT) | \
    (SST_TA_TCC_##b)<<SST_TA_TCC_B_SELECT_SHIFT) | \
    (SST_TA_TCC_##c)<<SST_TA_TCC_C_SELECT_SHIFT) | \
    (SST_TA_TCC_##d)<<SST_TA_TCC_D_SELECT_SHIFT) )

#define TCA(a,b,c,d) ( \
    (SST_TA_TCA_##a)<<SST_TA_TCA_A_SELECT_SHIFT) | \
    (SST_TA_TCA_##b)<<SST_TA_TCA_B_SELECT_SHIFT) | \
    (SST_TA_TCA_##c)<<SST_TA_TCA_C_SELECT_SHIFT) | \
    (SST_TA_TCA_##d)<<SST_TA_TCA_D_SELECT_SHIFT) )

#define CCC(a,b,c,d) ( \
    (SST_TA_CCC_##a)<<SST_TA_CCC_A_SELECT_SHIFT) | \
    (SST_TA_CCC_##b)<<SST_TA_CCC_B_SELECT_SHIFT) | \
    (SST_TA_CCC_##c)<<SST_TA_CCC_C_SELECT_SHIFT) | \
    (SST_TA_CCC_##d)<<SST_TA_CCC_D_SELECT_SHIFT) )

#define CCA(a,b,c,d) ( \
    (SST_TA_CCA_##a)<<SST_TA_CCA_A_SELECT_SHIFT) | \
    (SST_TA_CCA_##b)<<SST_TA_CCA_B_SELECT_SHIFT) | \
    (SST_TA_CCA_##c)<<SST_TA_CCA_C_SELECT_SHIFT) | \
    (SST_TA_CCA_##d)<<SST_TA_CCA_D_SELECT_SHIFT) )
