/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef AUDIO_H_
#define AUDIO_H_

#include "commands.h"

void update_audio_cfg(audioParams_t * user, audioParams_t * cfg);
void print_audioinfo(audioParams_t *pAudio);

#endif /* AUDIO_H_ */
