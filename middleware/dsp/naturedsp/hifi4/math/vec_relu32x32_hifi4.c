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
  NatureDSP Signal Processing Library. Vector matematics
    Rectifier functions
    Code optimized for HiFi4
  IntegrIT, 2006-2018
*/
#include "NatureDSP_Signal_math.h"
#include "NatureDSP_types.h"
#include "common.h"

/*-------------------------------------------------------------------------
  Rectifier function
  The functions compute the rectifier linear unit function of input argument. 
  32-bit fixed-point functions accept inputs in Q6.25 and form outputs in 
  Q16.15 format. Parameter K allows to set upper threshold for proper 
  compression of output signal.

  Precision:
  32x32  32-bit inputs, 32-bit output. Accuracy: 2 LSB.
  f      floating point input, floating point output. Accuracy 2 ULP
  Input:
  x[N]   input data, Q6.25 or floating point
  K      threshold, Q16.15 or floating point
  N      length of vectors
  Output:
  y[N]   result, Q16.15 or floating point

  Restriction:
  x,y should not overlap

  Scalar versions:
  ----------------
  return result, Q16.15 or floating point
-------------------------------------------------------------------------*/
void vec_relu32x32 (int32_t   * y, const int32_t   * x, int32_t   K, int N)
{
    ae_valign aY;
    const ae_int32x2 * restrict pX=(const ae_int32x2 *)x;
          ae_int32x2 * restrict pY=(      ae_int32x2 *)y;
    ae_int32x2 t;
    int n;
    if (N<=0) return;
    if (((uintptr_t)pX)&7)
    {
        AE_L32_IP(t,castxcc(ae_int32,pX),sizeof(int32_t));
        t=AE_SRAI32(t,10); // convert from Q25 to Q15
        t=AE_MIN32(K,t);
        t=AE_MAX32(t,AE_ZERO32());
        AE_S32_L_IP(t,castxcc(ae_int32,pY),sizeof(int32_t));
        N--;
    }
    aY=AE_ZALIGN64();
    for(n=0; n<(N>>1); n++)
    {
        AE_L32X2_IP(t,pX,sizeof(ae_int32x2));
        t=AE_SRAI32(t,10); // convert from Q25 to Q15
        t=AE_MIN32(K,t);
        t=AE_MAX32(t,AE_ZERO32());
        AE_SA32X2_IP(t,aY,pY);
    }
    AE_SA64POS_FP(aY,pY);
    if (N&1)
    {
        t=AE_L32_I((const ae_int32*)pX,0);
        t=AE_SRAI32(t,10); // convert from Q25 to Q15
        t=AE_MIN32(K,t);
        t=AE_MAX32(t,AE_ZERO32());
        AE_S32_L_I(t,(ae_int32*)pY,0);
    }
}
