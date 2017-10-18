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

#pragma once

#include <stdint.h>
#include <system/audio.h>

#include "hardware/audio_effect.h"

#define NUMCHANNEL 2

class Effect
{
protected:
    bool mEnable;
    float mSamplingRate;

    int32_t configure(void *pCmdData, size_t* frameCountInit, effect_buffer_access_e* mAccessMode);

public:
    Effect();
    virtual ~Effect();
    virtual int32_t process(audio_buffer_t *in, audio_buffer_t *out) = 0;
    virtual int32_t command(uint32_t cmdCode, uint32_t cmdSize, void* pCmdData, uint32_t* replySize, void* pReplyData) = 0;
};
