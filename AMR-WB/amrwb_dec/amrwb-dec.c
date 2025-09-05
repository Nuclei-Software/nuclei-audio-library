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
#include <string.h>
#include "wavwriter.h"
#include "dec_if.h"

#include "enc_amr.h"
#include "dec_wav.h"
#include "memfop.h"
#include "nmsis_bench.h"

#define ENC_AMR_LEN (sizeof(enc_amr))
#define DEC_WAV_LEN (sizeof(dec_wav))

BENCH_DECLARE_VAR();

uint8_t dec_result[DEC_WAV_LEN] = {0};

/* From pvamrwbdecoder_api.h, by dividing by 8 and rounding up */
const int sizes[] = { 17, 23, 32, 36, 40, 46, 50, 58, 60, 5, -1, -1, -1, -1, -1, 0 };

int main(int argc, char *argv[]) {
	MemoryFile* in;
	char header[9];
	int n;
	void* amr;
	struct wav_writer* wav;
	const uint8_t* pref; // the reference data pointer

	// open file
	in = memfopen(enc_amr, ENC_AMR_LEN);
	
	// read input data and check header
	n = memfread(header, 1, 9, in);
	if (n != 9 || memcmp(header, "#!AMR-WB\n", 9)) {
		fprintf(stderr, "Bad header\n");
		return 1;
	}

	MemoryFile *out = memfopen(dec_result, DEC_WAV_LEN);
	wav = wav_write_open(out, 16000, 16, 1);
	if (!wav) {
		fprintf(stderr, "Unable to open %s\n", argv[2]);
		return 1;
	}

	amr = D_IF_init();
	pref = dec_wav + 44;
	while (1) {
		uint8_t buffer[500], littleendian[640], *ptr, *p;
		int size, i;
		int16_t outbuffer[320];
		/* Read the mode byte */
		n = memfread(buffer, 1, 1, in);
		if (n <= 0)
			break;
		/* Find the packet size */
		size = sizes[(buffer[0] >> 3) & 0x0f];
		if (size < 0)
			break;
		n = memfread(buffer + 1, 1, size, in);
		if (n != size)
			break;

		/* Decode the packet */
		BENCH_START(decode);
		D_IF_decode(amr, buffer, outbuffer, 0);
		BENCH_END(decode);

		/* Convert to little endian and write to wav */
		ptr = littleendian;
		for (i = 0; i < 320; i++) {
			*ptr++ = (outbuffer[i] >> 0) & 0xff;
			*ptr++ = (outbuffer[i] >> 8) & 0xff;
		}
		wav_write_data(wav, littleendian, 640);

		// check result
		for(p = littleendian, i = 0; i < 640; ++i) {
			if(*p++ != *pref++) {
				printf("FAIL\r\n");
				return 1;
			}
		}
	}
	memfclose(in);
	D_IF_exit(amr);
	wav_write_close(wav);
	printf("PASS\r\n");
	return 0;
}

