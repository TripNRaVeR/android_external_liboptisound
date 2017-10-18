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

#include <string.h>
#include <media/AudioEffect.h>
#include <hardware/audio_effect.h>

#include "Effect.h"
#include "EffectDSPMain.h"

static effect_descriptor_t optisound_descriptor =
{
	{ 0xee1a4bcb, 0xc787, 0x4bf1, 0xa079, { 0x07, 0xfd, 0x35, 0xe3, 0xd9, 0xa7 } },
    { 0xd46ca451, 0xbe8f, 0x4d6c, 0xa2a3, { 0x52, 0x92, 0x6c, 0x3a, 0x04, 0x39 } },
    EFFECT_CONTROL_API_VERSION,
    EFFECT_FLAG_INSERT_FIRST,
	300,
    1,
    "OptiSound effect",
    "OptiSound"
};

extern "C" {

    struct effect_module_s {
        const struct effect_interface_s *itfe;
        Effect *effect;
        effect_descriptor_t *descriptor;
    };

    static int32_t generic_process(effect_handle_t self, audio_buffer_t *in, audio_buffer_t *out) {
        struct effect_module_s *e = (struct effect_module_s *) self;
        return e->effect->process(in, out);
    }

    static int32_t generic_command(effect_handle_t self, uint32_t cmdCode, uint32_t cmdSize, void *pCmdData, uint32_t *replySize, void *pReplyData) {
        struct effect_module_s *e = (struct effect_module_s *) self;
        return e->effect->command(cmdCode, cmdSize, pCmdData, replySize, pReplyData);
    }

    static int32_t generic_getDescriptor(effect_handle_t self, effect_descriptor_t *pDescriptor) {
        struct effect_module_s *e = (struct effect_module_s *) self;
        memcpy(pDescriptor, e->descriptor, sizeof(effect_descriptor_t));
        return 0;
    }

    static const struct effect_interface_s generic_interface = {
        generic_process,
        generic_command,
        generic_getDescriptor,
        NULL
    };

    int32_t EffectCreate(const effect_uuid_t *uuid, int32_t sessionId, int32_t ioId, effect_handle_t *pEffect) {

        if (memcmp(uuid, &optisound_descriptor.uuid, sizeof(effect_uuid_t)) == 0) {
            struct effect_module_s *e = (struct effect_module_s *) calloc(1, sizeof(struct effect_module_s));
            e->itfe = &generic_interface;
            e->effect = new EffectDSPMain();
            e->descriptor = &optisound_descriptor;
            *pEffect = (effect_handle_t) e;
            return 0;
        }
        return -EINVAL;
    }

    int32_t EffectRelease(effect_handle_t ei) {
        struct effect_module_s *e = (struct effect_module_s *) ei;
        delete e->effect;
        free(e);
        return 0;
    }

    int32_t EffectGetDescriptor(const effect_uuid_t *uuid, effect_descriptor_t *pDescriptor) {

        if (memcmp(uuid, &optisound_descriptor.uuid, sizeof(effect_uuid_t)) == 0) {
            memcpy(pDescriptor, &optisound_descriptor, sizeof(effect_descriptor_t));
            return 0;
        }
        return -EINVAL;
    }

    audio_effect_library_t AUDIO_EFFECT_LIBRARY_INFO_SYM = {
        .tag = AUDIO_EFFECT_LIBRARY_TAG,
        .version = EFFECT_LIBRARY_API_VERSION,
        .name = "OptiSound Effect Library",
        .implementor = "OptiSound",
        .create_effect = EffectCreate,
        .release_effect = EffectRelease,
        .get_descriptor = EffectGetDescriptor,
    };
}
