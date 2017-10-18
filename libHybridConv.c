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

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "libHybridConv.h"

int hcFFTWThreadInit() {
	return fftwf_init_threads();
}

void hcFFTWClean(int threads) {
	if(threads)
		fftwf_cleanup_threads();
	else
		fftwf_cleanup();
}

void hcProcess(HConvSingle *filter, float *x, float *y) {

    int flen, size, mpos, s, n, start, stop;
	float *out, *hist, *x_real, *x_imag, *h_real, *h_imag, *y_real, *y_imag;
    flen = filter->framelength;
    size = filter->frameLenMulFloatSize;
    memcpy(filter->dft_time, x, size);
    memset(&(filter->dft_time[flen]), 0, size);
    fftwf_execute(filter->fft);
    for (n = 0; n < flen + 1; n++)
    {
        filter->in_freq_real[n] = filter->dft_freq[n][0];
        filter->in_freq_imag[n] = filter->dft_freq[n][1];
    }

	x_real = filter->in_freq_real;
	x_imag = filter->in_freq_imag;
	start = filter->steptask[filter->step];
	stop = filter->steptask[filter->step + 1];
	for (s = start; s < stop; s++)
	{
		n = (s + filter->mixpos) % filter->num_mixbuf;
		y_real = filter->mixbuf_freq_real[n];
		y_imag = filter->mixbuf_freq_imag[n];
		h_real = filter->filterbuf_freq_real[s];
		h_imag = filter->filterbuf_freq_imag[s];
		for (n = 0; n < flen + 1; n++)
		{
			y_real[n] += x_real[n] * h_real[n] -
				x_imag[n] * h_imag[n];
			y_imag[n] += x_real[n] * h_imag[n] +
				x_imag[n] * h_real[n];
		}
	}
	filter->step = (filter->step + 1) % filter->maxstep;

	mpos = filter->mixpos;
	out = filter->dft_time;
	hist = filter->history_time;
	for (n = 0; n < flen + 1; n++)
	{
		filter->dft_freq[n][0] = filter->mixbuf_freq_real[mpos][n];
		filter->dft_freq[n][1] = filter->mixbuf_freq_imag[mpos][n];
		filter->mixbuf_freq_real[mpos][n] = 0.0;
		filter->mixbuf_freq_imag[mpos][n] = 0.0;
	}
	fftwf_execute(filter->ifft);
	for (n = 0; n < flen; n++)
		y[n] = out[n] + hist[n];
	memcpy(hist, &(out[flen]), size);
	filter->mixpos = (filter->mixpos + 1) % filter->num_mixbuf;
}

void hcProcessAdd(HConvSingle *filter, float *x, float *y) {

	int flen, size, mpos, s, n, start, stop;
	float *out, *hist, *x_real, *x_imag, *h_real, *h_imag, *y_real, *y_imag;
	flen = filter->framelength;
	size = filter->frameLenMulFloatSize;
	memcpy(filter->dft_time, x, size);
	memset(&(filter->dft_time[flen]), 0, size);
	fftwf_execute(filter->fft);
	for (n = 0; n < flen + 1; n++)
	{
		filter->in_freq_real[n] = filter->dft_freq[n][0];
		filter->in_freq_imag[n] = filter->dft_freq[n][1];
	}

	x_real = filter->in_freq_real;
	x_imag = filter->in_freq_imag;
	start = filter->steptask[filter->step];
	stop = filter->steptask[filter->step + 1];
	for (s = start; s < stop; s++)
	{
		n = (s + filter->mixpos) % filter->num_mixbuf;
		y_real = filter->mixbuf_freq_real[n];
		y_imag = filter->mixbuf_freq_imag[n];
		h_real = filter->filterbuf_freq_real[s];
		h_imag = filter->filterbuf_freq_imag[s];
		for (n = 0; n < flen + 1; n++)
		{
			y_real[n] += x_real[n] * h_real[n] -
				x_imag[n] * h_imag[n];
			y_imag[n] += x_real[n] * h_imag[n] +
				x_imag[n] * h_real[n];
		}
	}
	filter->step = (filter->step + 1) % filter->maxstep;

	mpos = filter->mixpos;
	out = filter->dft_time;
	hist = filter->history_time;
	for (n = 0; n < flen + 1; n++)
	{
		filter->dft_freq[n][0] = filter->mixbuf_freq_real[mpos][n];
		filter->dft_freq[n][1] = filter->mixbuf_freq_imag[mpos][n];
		filter->mixbuf_freq_real[mpos][n] = 0.0;
		filter->mixbuf_freq_imag[mpos][n] = 0.0;
	}
	fftwf_execute(filter->ifft);
	for (n = 0; n < flen; n++)
		y[n] += out[n] + hist[n];
	memcpy(hist, &(out[flen]), size);
	filter->mixpos = (filter->mixpos + 1) % filter->num_mixbuf;
}

void hcInitSingle(HConvSingle *filter, float *h, int hlen, int flen, int steps, int fftOptimize, int fftwThreads) {

    int i, j, size, num, pos, size2;
    float gain;
    filter->step = 0;
    filter->maxstep = steps;
    filter->mixpos = 0;
    filter->framelength = flen;
    size2 = 2 * flen;
    filter->frameLenMulFloatSize = flen * sizeof(float);
    size = sizeof(float) * size2;
    filter->dft_time = (float *)fftwf_malloc(size);
    size = sizeof(fftwf_complex) * (flen + 1);
    filter->dft_freq = (fftwf_complex*)fftwf_malloc(size);
    size = sizeof(float) * (flen + 1);
    filter->in_freq_real = (float*)fftwf_malloc(size);
    filter->in_freq_imag = (float*)fftwf_malloc(size);
    filter->num_filterbuf = (hlen + flen - 1) / flen;
    size = sizeof(int) * (steps + 1);
    filter->steptask = (int *)malloc(size);
    num = filter->num_filterbuf / steps;
    for (i = 0; i <= steps; i++)
        filter->steptask[i] = i * num;
    if (filter->steptask[1] == 0)
        pos = 1;
    else
        pos = 2;
    num = filter->num_filterbuf % steps;
    for (j = pos; j < pos + num; j++)
    {
        for (i = j; i <= steps; i++)
            filter->steptask[i]++;
    }
    size = sizeof(float*) * filter->num_filterbuf;
    filter->filterbuf_freq_real = (float**)fftwf_malloc(size);
    filter->filterbuf_freq_imag = (float**)fftwf_malloc(size);
    for (i = 0; i < filter->num_filterbuf; i++)
    {
        size = sizeof(float) * (flen + 1);
        filter->filterbuf_freq_real[i] = (float*)fftwf_malloc(size);
        filter->filterbuf_freq_imag[i] = (float*)fftwf_malloc(size);
    }
    filter->num_mixbuf = filter->num_filterbuf + 1;
    size = sizeof(float*) * filter->num_mixbuf;
    filter->mixbuf_freq_real = (float**)fftwf_malloc(size);
    filter->mixbuf_freq_imag = (float**)fftwf_malloc(size);
    for (i = 0; i < filter->num_mixbuf; i++)
    {
        size = sizeof(float) * (flen + 1);
        filter->mixbuf_freq_real[i] = (float*)fftwf_malloc(size);
        filter->mixbuf_freq_imag[i] = (float*)fftwf_malloc(size);
        memset(filter->mixbuf_freq_real[i], 0, size);
        memset(filter->mixbuf_freq_imag[i], 0, size);
    }
    size = sizeof(float) * flen;
    filter->history_time = (float *)fftwf_malloc(size);
    memset(filter->history_time, 0, size);
    if (fftOptimize == 0)
        fftOptimize = FFTW_ESTIMATE;
    else if (fftOptimize == 1)
        fftOptimize = FFTW_MEASURE;
    if(fftwThreads)
    	fftwf_plan_with_nthreads(fftwThreads);
    filter->fft = fftwf_plan_dft_r2c_1d(size2, filter->dft_time, filter->dft_freq, fftOptimize | FFTW_PRESERVE_INPUT);
    filter->ifft = fftwf_plan_dft_c2r_1d(size2, filter->dft_freq, filter->dft_time, fftOptimize | FFTW_PRESERVE_INPUT);
    gain = 0.5f / flen;
    size = sizeof(float) * size2;
    memset(filter->dft_time, 0, size);
    for (i = 0; i < filter->num_filterbuf - 1; i++)
    {
        for (j = 0; j < flen; j++)
            filter->dft_time[j] = gain * h[i * flen + j];
        fftwf_execute(filter->fft);
        for (j = 0; j < flen + 1; j++)
        {
            filter->filterbuf_freq_real[i][j] = filter->dft_freq[j][0];
            filter->filterbuf_freq_imag[i][j] = filter->dft_freq[j][1];
        }
    }
    for (j = 0; j < hlen - i * flen; j++)
        filter->dft_time[j] = gain * h[i * flen + j];
    size = sizeof(float) * ((i + 1) * flen - hlen);
    memset(&(filter->dft_time[hlen - i * flen]), 0, size);
    fftwf_execute(filter->fft);
    for (j = 0; j < flen + 1; j++)
    {
        filter->filterbuf_freq_real[i][j] = filter->dft_freq[j][0];
        filter->filterbuf_freq_imag[i][j] = filter->dft_freq[j][1];
    }
}

void hcCloseSingle(HConvSingle *filter) {
    int i;
    fftwf_destroy_plan(filter->ifft);
    fftwf_destroy_plan(filter->fft);
    fftwf_free(filter->history_time);
    for (i = 0; i < filter->num_mixbuf; i++)
    {
        fftwf_free(filter->mixbuf_freq_real[i]);
        fftwf_free(filter->mixbuf_freq_imag[i]);
    }
    fftwf_free(filter->mixbuf_freq_real);
    fftwf_free(filter->mixbuf_freq_imag);
    for (i = 0; i < filter->num_filterbuf; i++)
    {
        fftwf_free(filter->filterbuf_freq_real[i]);
        fftwf_free(filter->filterbuf_freq_imag[i]);
    }
    fftwf_free(filter->filterbuf_freq_real);
    fftwf_free(filter->filterbuf_freq_imag);
    fftwf_free(filter->in_freq_real);
    fftwf_free(filter->in_freq_imag);
    fftwf_free(filter->dft_freq);
    fftwf_free(filter->dft_time);
    free(filter->steptask);
    memset(filter, 0, sizeof(HConvSingle));
}
