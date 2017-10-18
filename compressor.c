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

#include "compressor.h"
#include <math.h>
#include <string.h>

static inline float db2lin(float db){
    return powf(10.0f, 0.05f * db);
}

static inline float lin2db(float lin){
    return 20.0f * log10f(lin);
}

static inline float kneecurve(float x, float k, float linearthreshold){
    return linearthreshold + (1.0f - expf(-k * (x - linearthreshold))) / k;
}

float kneeslope(float x, float k, float linearthreshold){
    return k * x / ((k * linearthreshold + 1.0f) * expf(k * (x - linearthreshold)) - 1);
}

static inline float compcurve(float x, float k, float slope, float linearthreshold,
                              float linearthresholdknee, float threshold, float knee, float kneedboffset){
    if (x < linearthreshold)
        return x;
    if (knee <= 0.0f)
        return db2lin(threshold + slope * (lin2db(x) - threshold));
    if (x < linearthresholdknee)
        return kneecurve(x, k, linearthreshold);
    return db2lin(kneedboffset + slope * (lin2db(x) - threshold - knee));
}

void sf_advancecomp(sf_compressor_state_st *state, int rate, float pregain, float threshold,
                    float knee, float ratio, float attack, float release, float predelay, float releasezone1,
                    float releasezone2, float releasezone3, float releasezone4, float postgain){

    int delaybufsize = rate * predelay;
    if (delaybufsize < 1)
        delaybufsize = 1;
    else if (delaybufsize > SF_COMPRESSOR_MAXDELAY)
        delaybufsize = SF_COMPRESSOR_MAXDELAY;
    if (delaybufsize > 0)
    {
        memset(state->delaybufL, 0, sizeof(float) * delaybufsize);
        memset(state->delaybufR, 0, sizeof(float) * delaybufsize);
    }

    float linearpregain = db2lin(pregain);
    float linearthreshold = db2lin(threshold);
    float slope = 1.0f / ratio;
    float attacksamples = rate * attack;
    float attacksamplesinv = 1.0f / attacksamples;
    float releasesamples = rate * release;
    float satrelease = 0.0025f;
    float satreleasesamplesinv = 1.0f / ((float)rate * satrelease);

#ifdef METER
    float metergain = 1.0f;
    float meterfalloff = 0.325f;
    float meterrelease = 1.0f - expf(-1.0f / ((float)rate * meterfalloff));
#endif

    float k = 5.0f;
    float kneedboffset;
    float linearthresholdknee;
    if (knee > 0.0f)
    {
        float xknee = db2lin(threshold + knee);
        float mink = 0.1f;
        float maxk = 10000.0f;

        for (int i = 0; i < 15; i++)
        {
            if (kneeslope(xknee, k, linearthreshold) < slope)
                maxk = k;
            else
                mink = k;
            k = sqrtf(mink * maxk);
        }
        kneedboffset = lin2db(kneecurve(xknee, k, linearthreshold));
        linearthresholdknee = db2lin(threshold + knee);
    }

    // calculate a master gain based on what sounds good
    float fulllevel = compcurve(1.0f, k, slope, linearthreshold, linearthresholdknee,
                                threshold, knee, kneedboffset);
    float mastergain = db2lin(postgain) * powf(1.0f / fulllevel, 0.6f);

    // calculate the adaptive release curve parameters
    // solve a,b,c,d in `y = a*x^3 + b*x^2 + c*x + d`
    // interescting points (0, y1), (1, y2), (2, y3), (3, y4)
    float y1 = releasesamples * releasezone1;
    float y2 = releasesamples * releasezone2;
    float y3 = releasesamples * releasezone3;
    float y4 = releasesamples * releasezone4;
    float a = (-y1 + 3.0f * y2 - 3.0f * y3 + y4) / 6.0f;
    float b = y1 - 2.5f * y2 + 2.0f * y3 - 0.5f * y4;
    float c = (-11.0f * y1 + 18.0f * y2 - 9.0f * y3 + 2.0f * y4) / 6.0f;
    float d = y1;

#ifdef METER
    state->metergain            = 1.0f;
    state->meterrelease         = meterrelease;
#endif
    state->threshold            = threshold;
    state->knee                 = knee;
    state->linearpregain        = linearpregain;
    state->linearthreshold      = linearthreshold;
    state->slope                = slope;
    state->attacksamplesinv     = attacksamplesinv;
    state->satreleasesamplesinv = satreleasesamplesinv;
    state->k                    = k;
    state->kneedboffset         = kneedboffset;
    state->linearthresholdknee  = linearthresholdknee;
    state->mastergain           = mastergain;
    state->a                    = a;
    state->b                    = b;
    state->c                    = c;
    state->d                    = d;
    state->detectoravg          = 0.0f;
    state->compgain             = 1.0f;
    state->maxcompdiffdb        = -1.0f;
    state->delaybufsize         = delaybufsize;
    state->delaywritepos        = 0;
    state->delayreadpos         = delaybufsize > 1 ? 1 : 0;
}

static inline float adaptivereleasecurve(float x, float a, float b, float c, float d){
    float x2 = x * x;
    return a * x2 * x + b * x2 + c * x + d;
}

static inline float clampf(float v, float min, float max){
    return v < min ? min : (v > max ? max : v);
}

static inline float absf(float v){
    return v < 0.0f ? -v : v;
}

static inline float fixf(float v, float def){
    if (isnan(v) || isinf(v))
        return def;
    return v;
}

void sf_compressor_process(sf_compressor_state_st *state, int size, float *inputL, float *inputR, float *outputL, float *outputR){

#ifdef METER
    float metergain            = state->metergain;
    float meterrelease         = state->meterrelease;
#endif

    float threshold            = state->threshold;
    float knee                 = state->knee;
    float linearpregain        = state->linearpregain;
    float linearthreshold      = state->linearthreshold;
    float slope                = state->slope;
    float attacksamplesinv     = state->attacksamplesinv;
    float satreleasesamplesinv = state->satreleasesamplesinv;
    float k                    = state->k;
    float kneedboffset         = state->kneedboffset;
    float linearthresholdknee  = state->linearthresholdknee;
    float mastergain           = state->mastergain;
    float a                    = state->a;
    float b                    = state->b;
    float c                    = state->c;
    float d                    = state->d;
    float detectoravg          = state->detectoravg;
    float compgain             = state->compgain;
    float maxcompdiffdb        = state->maxcompdiffdb;
    int delaybufsize           = state->delaybufsize;
    int delaywritepos          = state->delaywritepos;
    int delayreadpos           = state->delayreadpos;
    float *delaybufL           = state->delaybufL;
    float *delaybufR           = state->delaybufR;
    int chunks = size / SF_COMPRESSOR_SPU;
    float ang90 = 1.57079625f;
    float ang90inv = 0.63661978f;
    int samplepos = 0;

    for (int ch = 0; ch < chunks; ch++)
    {
        detectoravg = fixf(detectoravg, 1.0f);
        float desiredgain = detectoravg;
        float scaleddesiredgain = asinf(desiredgain) * ang90inv;
        float compdiffdb = lin2db(compgain / scaleddesiredgain);

        float enveloperate;
        if (compdiffdb < 0.0f)
        {
            compdiffdb = fixf(compdiffdb, -1.0f);
            maxcompdiffdb = -1;

            float x = (clampf(compdiffdb, -12.0f, 0.0f) + 12.0f) * 0.25f;
            float releasesamples = adaptivereleasecurve(x, a, b, c, d);
            enveloperate = db2lin(SF_COMPRESSOR_SPACINGDB / releasesamples);
        }
        else
        {
            compdiffdb = fixf(compdiffdb, 1.0f);
            if (maxcompdiffdb == -1 || maxcompdiffdb < compdiffdb)
                maxcompdiffdb = compdiffdb;
            float attenuate = maxcompdiffdb;
            if (attenuate < 0.5f)
                attenuate = 0.5f;
            enveloperate = 1.0f - powf(0.25f / attenuate, attacksamplesinv);
        }

        for (int chi = 0; chi < SF_COMPRESSOR_SPU; chi++, samplepos++,
                delayreadpos = (delayreadpos + 1) % delaybufsize,
                delaywritepos = (delaywritepos + 1) % delaybufsize){

            float inL = inputL[samplepos] * linearpregain;
            float inR = inputR[samplepos] * linearpregain;
            delaybufL[delaywritepos] = inL;
            delaybufR[delaywritepos] = inR;
            inL = absf(inL);
            inR = absf(inR);
            float inputmax = inL > inR ? inL : inR;
            float attenuation;
            if (inputmax < 0.0001f)
                attenuation = 1.0f;
            else
            {
                float inputcomp = compcurve(inputmax, k, slope, linearthreshold,
                                            linearthresholdknee, threshold, knee, kneedboffset);
                attenuation = inputcomp / inputmax;
            }
            float rate;
            if (attenuation > detectoravg)
            {
                float attenuationdb = -lin2db(attenuation);
                if (attenuationdb < 2.0f)
                    attenuationdb = 2.0f;
                float dbpersample = attenuationdb * satreleasesamplesinv;
                rate = db2lin(dbpersample) - 1.0f;
            }
            else
                rate = 1.0f;
            detectoravg += (attenuation - detectoravg) * rate;
            if (detectoravg > 1.0f)
                detectoravg = 1.0f;
            detectoravg = fixf(detectoravg, 1.0f);
            if (enveloperate < 1)
                compgain += (scaleddesiredgain - compgain) * enveloperate;
            else
            {
                compgain *= enveloperate;
                if (compgain > 1.0f)
                    compgain = 1.0f;
            }

            float premixgain = sinf(ang90 * compgain);
            float gain = mastergain * premixgain;
#ifdef METER
            float premixgaindb = lin2db(premixgain);
            if (premixgaindb < metergain)
                metergain = premixgaindb;
            else
                metergain += (premixgaindb - metergain) * meterrelease;
#endif

            outputL[samplepos] = delaybufL[delayreadpos] * gain;
            outputR[samplepos] = delaybufR[delayreadpos] * gain;
        }
    }

#ifdef METER
    state->metergain     = metergain;
#endif

    state->detectoravg   = detectoravg;
    state->compgain      = compgain;
    state->maxcompdiffdb = maxcompdiffdb;
    state->delaywritepos = delaywritepos;
    state->delayreadpos  = delayreadpos;
}
