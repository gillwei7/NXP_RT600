/* ------------------------------------------------------------------------ */
/* Copyright (c) 2018 by Cadence Design Systems, Inc. ALL RIGHTS RESERVED.  */
/* These coded instructions, statements, and computer programs ("Cadence    */
/* Libraries") are the copyrighted works of Cadence Design Systems Inc.	    */
/* Cadence IP is licensed for use with Cadence processor cores only and     */
/* must not be used for any other processors and platforms. Your use of the */
/* Cadence Libraries is subject to the terms of the license agreement you   */
/* have entered into with Cadence Design Systems, or a sublicense granted   */
/* to you by a direct Cadence licensee.                                     */
/* ------------------------------------------------------------------------ */
/*  IntegrIT, Ltd.   www.integrIT.com, info@integrIT.com                    */
/*                                                                          */
/* DSP Library                                                              */
/*                                                                          */
/* This library contains copyrighted materials, trade secrets and other     */
/* proprietary information of IntegrIT, Ltd. This software is licensed for  */
/* use with Cadence processor cores only and must not be used for any other */
/* processors and platforms. The license to use these sources was given to  */
/* Cadence, Inc. under Terms and Condition of a Software License Agreement  */
/* between Cadence, Inc. and IntegrIT, Ltd.                                 */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2015-2018 IntegrIT, Limited.                      */
/*                      All Rights Reserved.                                */
/* ------------------------------------------------------------------------ */

/*
  NatureDSP Signal Processing Library. FIR part
    Linear Convolution, 32x32-bit, no requirements on vectors 
    length and alignment.
    C code optimized for HiFi4
  IntegrIT, 2006-2018
*/

/*-------------------------------------------------------------------------
  Linear Convolution
  Functions perform linear convolution between vectors x (of length N) and
  y (of length M) resulting in vector r of length N+M-1.

  Precision: 
  16x16       16x16-bit data, 16-bit outputs
  32x32       32x32-bit data, 32-bit outputs 

  Input:
  s[]         Scratch memory,
                FIR_LCONVOLA16X16_SCRATCH_SIZE( N, M ) or
                FIR_LCONVOLA32X32_SCRATCH_SIZE( N, M ) bytes
  x[N]        input data, Q15, Q31
  y[M]        input data, Q15, Q31
  N           length of x
  M           length of y
  Output:
  r[N+M-1]    output data, Q15, Q31

  Restriction:
  x,y,r       should not overlap
  s           must be aligned on an 8-bytes boundary
  N,M         must be >0
  N >= M-1    minimum allowed length of vector x is the length of y minus one
-------------------------------------------------------------------------*/

/* Portable data types. */
#include "NatureDSP_types.h"
/* Signal Processing Library API. */
#include "NatureDSP_Signal_fir.h"
/* Common utility and macros declarations. */
#include "common.h"

#include "raw_lxcorr32x32.h"

/* Align address on a specified boundary. */
#define ALIGNED_ADDR( addr, align ) \
    (void*)(((uintptr_t)(addr)+((align)-1)) & ~((align)-1))

void fir_lconvola32x32 (void    * restrict s,
                       int32_t  * restrict r,
                 const int32_t  * restrict x, 
                 const int32_t  * restrict y, int N, int M)
{
    void * s_ptr;
    int32_t  * x_buf;
    int32_t  * y_buf;
    const ae_f32x2 * S;
    ae_f32x2 * restrict D;
    ae_valign S_va, D_va;
    ae_f32x2   y0, x0;
    ae_int32x2 t0;

    int m;

    NASSERT(s);
    NASSERT(r);
    NASSERT(x);
    NASSERT(y);
    NASSERT_ALIGN(s, 8);
    NASSERT(N > 0 && M > 0);
    NASSERT(N >= M - 1);
    
    //----------------------------------------------------------------------------
    // Partition the scratch memory area.
    s_ptr = s;
    x_buf = (int32_t*)ALIGNED_ADDR(s_ptr, 8);
    s_ptr = x_buf + N + 4;
    y_buf = (int32_t*)ALIGNED_ADDR(s_ptr, 8);
    s_ptr = y_buf + M;

    ASSERT((int8_t *)s_ptr - (int8_t *)s <= (int)FIR_LCONVOLA32X32_SCRATCH_SIZE(N, M));
    //----------------------------------------------------------------------------
    S = (const ae_f32x2 *)x;
    D = (ae_f32x2 *)(x_buf);
    S_va = AE_LA64_PP(S);
    D_va = AE_ZALIGN64();
    for (m = 0; m < ((N + 1) >> 1); m++)
    {
        AE_LA32X2_IP(t0, S_va, S); x0 = (t0);
        AE_S32X2_IP(x0, D, +8);
    }

    D = (ae_f32x2 *)(x_buf+N);
    AE_S32_L_I( AE_ZERO32(),(ae_int32*) D, 0*sizeof(int32_t) );
    AE_S32_L_I( AE_ZERO32(),(ae_int32*) D, 1*sizeof(int32_t) );
    AE_S32_L_I( AE_ZERO32(),(ae_int32*) D, 2*sizeof(int32_t) );
    AE_S32_L_I( AE_ZERO32(),(ae_int32*) D, 3*sizeof(int32_t) );
    //----------------------------------------------------------------------------
    S = (const ae_f32x2 *)(y + M - 1);
    D = (ae_f32x2 *)y_buf;
    S_va = AE_LA64_PP(S);
    for (m = 0; m<((M + 1) >> 1); m++)
    {
        AE_LA32X2_RIP(t0, S_va, S); y0 = (t0);
        AE_S32X2_IP(y0, D, +8);
    }

    // Append three zeros to allow for block-4 processing.
    D = (ae_f32x2 *)(y_buf+M);
    AE_S32_L_I( AE_ZERO32(),(ae_int32*) D, 0*sizeof(int32_t) );
    AE_S32_L_I( AE_ZERO32(),(ae_int32*) D, 1*sizeof(int32_t) );
    AE_S32_L_I( AE_ZERO32(),(ae_int32*) D, 2*sizeof(int32_t) );
    AE_S32_L_I( AE_ZERO32(),(ae_int32*) D, 3*sizeof(int32_t) );
    fir_lxcorr32x32(r, x_buf, y_buf, N, M);

} /* fir_lconvola32x32() */
