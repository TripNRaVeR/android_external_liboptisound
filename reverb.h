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

#ifndef OPTISOUND_REVERB__H
#define OPTISOUND_REVERB__H

#define SF_REVERB_DS        3000

typedef struct {
    int pos;
    int size;
    float buf[SF_REVERB_DS];
} sf_rv_delay_st;

typedef struct {
    float a2;
    float b1;
    float b2;
    float y1;
} sf_rv_iir1_st;

typedef struct {
    float b0;
    float b1;
    float b2;
    float a1;
    float a2;
    float xn1;
    float xn2;
    float yn1;
    float yn2;
} sf_rv_biquad_st;

typedef struct {
    int             delaytblL[18], delaytblR[18];
    sf_rv_delay_st  delayPWL, delayPWR;
    sf_rv_delay_st  delayRL, delayLR;
    sf_rv_biquad_st allpassXL, allpassXR;
    sf_rv_biquad_st allpassL, allpassR;
    sf_rv_iir1_st   lpfL, lpfR;
    sf_rv_iir1_st   hpfL, hpfR;
    float wet1, wet2;
} sf_rv_earlyref_st;

#define SF_REVERB_OF        2
typedef struct {
    int factor;
    sf_rv_biquad_st lpfU;
    sf_rv_biquad_st lpfD;
} sf_rv_oversample_st;

typedef struct {
    float gain;
    float y1;
    float y2;
} sf_rv_dccut_st;

#define SF_REVERB_NS        (1<<11)

typedef struct {
    int pos;
    float buf[SF_REVERB_NS];
} sf_rv_noise_st;

typedef struct {
    float re;
    float im;
    float sn;
    float co;
    int count;
} sf_rv_lfo_st;

#define SF_REVERB_APS       3400

typedef struct {
    int pos;
    int size;
    float feedback;
    float decay;
    float buf[SF_REVERB_APS];
} sf_rv_allpass_st;

#define SF_REVERB_AP2S1     4200
#define SF_REVERB_AP2S2     3000

typedef struct {
    int   pos1, pos2;
    int   size1, size2;
    float feedback1, feedback2;
    float decay1, decay2;
    float buf1[SF_REVERB_AP2S1], buf2[SF_REVERB_AP2S2];
} sf_rv_allpass2_st;

#define SF_REVERB_AP3S1     4000
#define SF_REVERB_AP3M1     600
#define SF_REVERB_AP3S2     2000
#define SF_REVERB_AP3S3     3000

typedef struct {
    int   rpos1, wpos1, pos2, pos3;
    int   size1, msize1, size2, size3;
    float feedback1, feedback2, feedback3;
    float decay1, decay2, decay3;
    float buf1[SF_REVERB_AP3S1 + SF_REVERB_AP3M1], buf2[SF_REVERB_AP3S2], buf3[SF_REVERB_AP3S3];
} sf_rv_allpass3_st;

#define SF_REVERB_APMS      3600
#define SF_REVERB_APMM      137

typedef struct {
    int rpos, wpos;
    int size, msize;
    float feedback;
    float decay;
    float z1;
    float buf[SF_REVERB_APMS + SF_REVERB_APMM];
} sf_rv_allpassm_st;

#define SF_REVERB_CS        1500

typedef struct {
    int pos;
    int size;
    float buf[SF_REVERB_CS];
} sf_rv_comb_st;

typedef struct {
    sf_rv_earlyref_st   earlyref;
    sf_rv_oversample_st oversampleL, oversampleR;
    sf_rv_dccut_st      dccutL, dccutR;
    sf_rv_noise_st      noise;
    sf_rv_lfo_st        lfo1;
    sf_rv_iir1_st       lfo1_lpf;
    sf_rv_allpassm_st   diffL[10], diffR[10];
    sf_rv_allpass_st    crossL[4], crossR[4];
    sf_rv_iir1_st       clpfL, clpfR;
    sf_rv_delay_st      cdelayL, cdelayR;
    sf_rv_biquad_st     bassapL, bassapR;
    sf_rv_biquad_st     basslpL, basslpR;
    sf_rv_iir1_st       damplpL, damplpR;
    sf_rv_allpassm_st   dampap1L, dampap1R;
    sf_rv_delay_st      dampdL, dampdR;
    sf_rv_allpassm_st   dampap2L, dampap2R;
    sf_rv_delay_st      cbassd1L, cbassd1R;
    sf_rv_allpass2_st   cbassap1L, cbassap1R;
    sf_rv_delay_st      cbassd2L, cbassd2R;
    sf_rv_allpass3_st   cbassap2L, cbassap2R;
    sf_rv_lfo_st        lfo2;
    sf_rv_iir1_st       lfo2_lpf;
    sf_rv_comb_st       combL, combR;
    sf_rv_biquad_st     lastlpfL, lastlpfR;
    sf_rv_delay_st      lastdelayL, lastdelayR;
    sf_rv_delay_st      inpdelayL, inpdelayR;
    int outco[32];
    float loopdecay;
    float wet1, wet2;
    float wander;
    float bassb;
    float ertolate;
    float erefwet;
    float dry;
} sf_reverb_state_st;

typedef enum
{
    SF_REVERB_PRESET_DEFAULT,
    SF_REVERB_PRESET_SMALLHALL1,
    SF_REVERB_PRESET_SMALLHALL2,
    SF_REVERB_PRESET_MEDIUMHALL1,
    SF_REVERB_PRESET_MEDIUMHALL2,
    SF_REVERB_PRESET_LARGEHALL1,
    SF_REVERB_PRESET_LARGEHALL2,
    SF_REVERB_PRESET_SMALLROOM1,
    SF_REVERB_PRESET_SMALLROOM2,
    SF_REVERB_PRESET_MEDIUMROOM1,
    SF_REVERB_PRESET_MEDIUMROOM2,
    SF_REVERB_PRESET_LARGEROOM1,
    SF_REVERB_PRESET_LARGEROOM2,
    SF_REVERB_PRESET_MEDIUMER1,
    SF_REVERB_PRESET_MEDIUMER2,
    SF_REVERB_PRESET_PLATEHIGH,
    SF_REVERB_PRESET_PLATELOW,
    SF_REVERB_PRESET_LONGREVERB1,
    SF_REVERB_PRESET_LONGREVERB2
} sf_reverb_preset;

void sf_presetreverb(sf_reverb_state_st *state, int rate, sf_reverb_preset preset);

void sf_advancereverb(sf_reverb_state_st *rv,
                      int rate,
                      int oversamplefactor,
                      float ertolate,
                      float erefwet,
                      float dry,
                      float ereffactor,
                      float erefwidth,
                      float width,
                      float wet,
                      float wander,
                      float bassb,
                      float spin,
                      float inputlpf,
                      float basslpf,
                      float damplpf,
                      float outputlpf,
                      float rt60,
                      float delay
                     );

void sf_reverb_process(sf_reverb_state_st *rv, float inputL, float inputR, float *outputL, float *outputR);
#endif // OPTISOUND_REVERB__H
