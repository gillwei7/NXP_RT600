/* ------------------------------------------------------------------------ */
/* Copyright (c) 2019 by Cadence Design Systems, Inc. ALL RIGHTS RESERVED.  */
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
/*          Copyright (C) 2015-2019 IntegrIT, Limited.                      */
/*                      All Rights Reserved.                                */
/* ------------------------------------------------------------------------ */
#ifndef IMG_COEF_UP_CUBIC_H__
#define IMG_COEF_UP_CUBIC_H__
#include "NatureDSP_types.h"
#include "NatureDSP_Signal_img.h"
#include "common.h"

/* coefficients for bicubic upsampler with ratio 1..2 
   M - input length, N - output length, 1<N/M<2
*/

typedef struct
{
    int16_t *coef;    // [4][N] coefficients, aligned, Q14 format
    int16_t *left;    // [N]    indices, aligned
}
img_coefup_cubic_t;

size_t img_getCoef_up_cubic_alloc(int M,int N);

/* function returns w(N,2) coefficients and left(N) indices
  for upsampler
  M - input length, N - output length, 1<N/M<2
*/
img_coefup_cubic_t* img_getCoef_up_cubic_init(void* objmem,int M,int N);

#endif
