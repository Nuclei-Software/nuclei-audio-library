/* ------------------------------------------------------------------
 * Copyright (C) 2009 Martin Storsjo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdint.h>
#include <enc_if.h>
#include <unistd.h>
#include <stdlib.h>
#include "wavreader.h"

#include "input.h"
#include "enc_amr.h"
#include "nmsis_bench.h"

#define INPUT_LEN (sizeof(input))
#define ENC_AMR_LEN (sizeof(enc_amr))

BENCH_DECLARE_VAR();

int findMode(int rate) {
	struct {
		int mode;
		int rate;
	} modes[] = {
		{ 0,  6600 },
		{ 1,  8850 },
		{ 2, 12650 },
		{ 3, 14250 },
		{ 4, 15850 },
		{ 5, 18250 },
		{ 6, 19850 },
		{ 7, 23050 },
		{ 8, 23850 }
	};
	int closest = -1;
	int closestdiff = 0;
	unsigned int i;
	for (i = 0; i < sizeof(modes)/sizeof(modes[0]); i++) {
		if (modes[i].rate == rate)
			return modes[i].mode;
		if (closest < 0 || closestdiff > abs(modes[i].rate - rate)) {
			closest = i;
			closestdiff = abs(modes[i].rate - rate);
		}
	}
	printf("Using bitrate %d\n", modes[closest].rate);
	return modes[closest].mode;
}

uint8_t enc_result[ENC_AMR_LEN] = {0};

int main(int argc, char *argv[]) {
	int mode;
	int ch, dtx = 0;
	MemoryFile* out;
	void *wav, *amr;
	int format, sampleRate, channels, bitsPerSample;
	int inputSize;
	uint8_t* inputBuf;
	uint8_t* pref; // the reference data pointer

	int rate = BITRATE;
	dtx = 1;
	mode = findMode(rate);

	wav = wav_read_open(input, INPUT_LEN);
	if (!wav) {
		printf("Unable to open wav file\n");
		return 1;
	}
	if (!wav_get_header(wav, &format, &channels, &sampleRate, &bitsPerSample, NULL)) {
		printf("Bad wav file\n");
		return 1;
	}
	if (format != 1) {
		printf( "Unsupported WAV format %d\n", format);
		return 1;
	}
	if (bitsPerSample != 16) {
		printf("Unsupported WAV sample depth %d\n", bitsPerSample);
		return 1;
	}
	if (channels != 1)
		printf("Warning, only compressing one audio channel\n");
	if (sampleRate != 16000)
		printf("Warning, AMR-WB uses 16000 Hz sample rate (WAV file has %d Hz)\n", sampleRate);
	inputSize = channels*2*320;
	inputBuf = (uint8_t*) malloc(inputSize);

	amr = E_IF_init();
	out = memfopen(enc_result, ENC_AMR_LEN);
	if (!out) {
		return 1;
	}

	memfwrite("#!AMR-WB\n", 1, 9, out);
	// set reference data start from 9
    pref = enc_amr + 9;
	while (1) {
		int read, i, n;
		short buf[320];
		uint8_t outbuf[500], *p;

		read = wav_read_data(wav, inputBuf, inputSize);
		read /= channels;
		read /= 2;
		if (read < 320)
			break;
		for (i = 0; i < 320; i++) {
			const uint8_t* in = &inputBuf[2*channels*i];
			buf[i] = in[0] | (in[1] << 8);
		}
    	BENCH_START(encode);
		n = E_IF_encode(amr, mode, buf, outbuf, dtx);
    	BENCH_SAMPLE(encode);
		memfwrite(outbuf, 1, n, out);

		// check result
        for (p = outbuf, i = 0; i < n; i++) {
            if (*p++ != *pref++) {
                printf("FAIL\r\n");
                return 1;
            }
        }
	}
    double avg_cycle = BENCH_GET_SUMCYC() * 1.0 / BENCH_GET_LPCNT();
    // 16k sample rate, process 320 samples each frame, so multiply with 50
    double mcps = avg_cycle * 50 / 1000000;
    printf("CSV, amrwb_encode, %.02f\r\n", mcps);

	free(inputBuf);
	memfclose(out);
	E_IF_exit(amr);
	wav_read_close(wav);
  	printf("PASS\r\n");

	return 0;
}

