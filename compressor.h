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

#ifndef OPTISOUND_COMPRESSOR__H
#define OPTISOUND_COMPRESSOR__H

// dynamic range compression is a complex topic with many different algorithms
// this API works by first initializing an sf_compressor_state_st structure, then using it to
// process a sample in chunks

// maximum number of samples in the delay buffer
#define SF_COMPRESSOR_MAXDELAY   256

// samples per update; the compressor works by dividing the input chunks into even smaller sizes,
// and performs heavier calculations after each mini-chunk
#define SF_COMPRESSOR_SPU        8

// the release curve
#define SF_COMPRESSOR_SPACINGDB  5.0f

typedef struct
{
#ifdef METER
    float metergain;
    float meterrelease;
#endif

    float threshold;
    float knee;
    float linearpregain;
    float linearthreshold;
    float slope;
    float attacksamplesinv;
    float satreleasesamplesinv;
    float k;
    float kneedboffset;
    float linearthresholdknee;
    float mastergain;
    float a;
    float b;
    float c;
    float d;
    float detectoravg;
    float compgain;
    float maxcompdiffdb;
    int delaybufsize;
    int delaywritepos;
    int delayreadpos;
    float delaybufL[SF_COMPRESSOR_MAXDELAY];
    float delaybufR[SF_COMPRESSOR_MAXDELAY];
} sf_compressor_state_st;

// populate a compressor state with advanced parameters
void sf_advancecomp(sf_compressor_state_st *state,
                    int rate, float pregain, float threshold, float knee, float ratio, float attack, float release,
                    float predelay,
                    float releasezone1,
                    float releasezone2,
                    float releasezone3,
                    float releasezone4,
                    float postgain
                   );

void sf_compressor_process(sf_compressor_state_st *state, int size, float *inputL, float *inputR, float *outputL, float *outputR);

#endif // OPTISOUND_COMPRESSOR__H
