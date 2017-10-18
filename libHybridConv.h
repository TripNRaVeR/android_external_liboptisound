/*
 * Copyright (C) 2017 TripNDroid Mobile Engineering
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __LIBHYBRIDCONV_H__
#define __LIBHYBRIDCONV_H__

#include "fftw3.h"

typedef struct str_HConvSingle {
    int step;
    int maxstep;
    int mixpos;
    int framelength;
    int frameLenMulFloatSize;
    int *steptask;
    float *dft_time;
    fftwf_complex *dft_freq;
    float *in_freq_real;
    float *in_freq_imag;
    int num_filterbuf;
    float **filterbuf_freq_real;
    float **filterbuf_freq_imag;
    int num_mixbuf;
    float **mixbuf_freq_real;
    float **mixbuf_freq_imag;
    float *history_time;
    fftwf_plan fft;
    fftwf_plan ifft;
} HConvSingle;

int hcFFTWThreadInit();
void hcFFTWClean(int threads);
void hcProcess(HConvSingle *filter, float *x, float *y);
void hcProcessAdd(HConvSingle *filter, float *x, float *y);
void hcInitSingle(HConvSingle *filter, float *h, int hlen, int flen, int steps, int fftOptimize, int fftwThreads);
void hcCloseSingle(HConvSingle *filter);

#endif
