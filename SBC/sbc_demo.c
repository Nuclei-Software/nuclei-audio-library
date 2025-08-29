#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dec_wav.h"
#include "enc_sbc.h"
#include "input.h"

#include "memfop.h"
#include "wave.h"

#include "sbc.h"

#include "nmsis_bench.h"

struct parameters {
    const char *fname_in;
    const char *fname_out;
    struct sbc_frame frame;
};

uint8_t enc_result[ENC_SBC_LEN] = {0};
uint8_t dec_result[DEC_WAV_LEN] = {0};

BENCH_DECLARE_VAR();

int verify_result(const uint8_t *ref, const uint8_t *res, int len,
                  int threshold) {
    for (int i = 0; i < len; i++) {
        if (abs(ref[i] - res[i]) > threshold) {
            printf(
                "Result mismatch at byte %d!, expected 0x%02x, got 0x%02x\r\n",
                i, ref[i], res[i]);
            return EXIT_FAILURE;
        }
    }
    printf("Result matches!\r\n");
    return EXIT_SUCCESS;
}

int encode() {
    struct parameters p = {};
    struct sbc_frame *frame = &p.frame;
    *frame = (struct sbc_frame){.mode = SBC_MODE_STEREO,
                                .nsubbands = 8,
                                .nblocks = 16,
                                .bam = SBC_BAM_LOUDNESS,
                                .bitpool = 35};

    MemoryFile *fp_in, *fp_out;

    fp_in = memfopen((void *)in_1s_wav, in_1s_wav_len);
    if (fp_in == NULL) {
        printf("Error opening wav file!\r\n");
        return EXIT_FAILURE;
    }

    fp_out = memfopen(enc_result, ENC_SBC_LEN);
    if (fp_out == NULL) {
        printf("Error opening sbc file!\r\n");
        return EXIT_FAILURE;
    }

    /* --- Check parameters --- */

    int srate_hz, nch, nsamples;
    int pcm_sbits, pcm_sbytes;

    if (wave_read_header(fp_in, &pcm_sbits, &pcm_sbytes, &srate_hz, &nch,
                         &nsamples) < 0) {
        printf("Bad or unsupported WAVE input file!\r\n");
        return EXIT_FAILURE;
    }

    frame->freq = srate_hz == 16000   ? SBC_FREQ_16K
                  : srate_hz == 32000 ? SBC_FREQ_32K
                  : srate_hz == 44100 ? SBC_FREQ_44K1
                  : srate_hz == 48000 ? SBC_FREQ_48K
                                      : SBC_NUM_FREQ;

    if (nch == 1) {
        frame->mode = SBC_MODE_MONO;
    }

    if (frame->freq >= SBC_NUM_FREQ) {
        printf("Unsupported samplerate %d!\r\n", srate_hz);
        return EXIT_FAILURE;
    }

    if (pcm_sbits != 16 || pcm_sbytes != sizeof(int16_t)) {
        printf("Unsupported bitdepth %d!\r\n", pcm_sbits);
        return EXIT_FAILURE;
    }

    if (nch < 1 || nch > 2) {
        printf("Unsupported number of channels %d!\r\n", nch);
        return EXIT_FAILURE;
    }

    /* --- Setup decoding --- */

    uint8_t data[2 * SBC_MAX_SAMPLES * sizeof(int16_t)];
    int16_t pcm[2 * SBC_MAX_SAMPLES];
    sbc_t sbc;

    int npcm = frame->nblocks * frame->nsubbands;

    sbc_reset(&sbc);

    /* --- Encoding loop --- */

    for (int i = 0; wave_read_pcm(fp_in, pcm_sbytes, nch, npcm, pcm) >= npcm;
         i++) {

        BENCH_START(sbc_encode);
        sbc_encode(&sbc, pcm + 0, nch, pcm + 1, 2, frame, data, sizeof(data));
        BENCH_END(sbc_encode);

        memfwrite(data, sbc_get_frame_size(frame), 1, fp_out);
    }

    /* --- Cleanup --- */

    memfclose(&fp_in);
    memfclose(&fp_out);

    return EXIT_SUCCESS;
}

int decode() {
    /* --- Read parameters --- */

    struct parameters p;
    MemoryFile *fp_in, *fp_out;

    fp_in = memfopen((void *)enc_sbc, ENC_SBC_LEN);
    if (fp_in == NULL) {
        printf("Error opening sbc file!\r\n");
        return EXIT_FAILURE;
    }

    fp_out = memfopen(dec_result, DEC_WAV_LEN);
    if (fp_out == NULL) {
        printf("Error opening wav file!\r\n");
        return EXIT_FAILURE;
    }

    /* --- Setup decoding --- */

    static const char *sbc_mode_str[] = {
        [SBC_MODE_MONO] = "Mono",
        [SBC_MODE_DUAL_CHANNEL] = "Dual-Channel",
        [SBC_MODE_STEREO] = "Stereo",
        [SBC_MODE_JOINT_STEREO] = "Joint-Stereo"};

    uint8_t data[2 * SBC_MAX_SAMPLES * sizeof(int16_t)];
    int16_t pcm[2 * SBC_MAX_SAMPLES];
    struct sbc_frame frame;
    sbc_t sbc;

    if (memfread(data, SBC_PROBE_SIZE, 1, fp_in) < 1 ||
        sbc_probe(data, &frame) < 0) {
        printf("SBC input format error!\r\n");
        return EXIT_FAILURE;
    }

    int srate_hz = sbc_get_freq_hz(frame.freq);

    // fprintf(stderr,
    //         "%s %d Hz -- %.1f kbps (bitpool %d)"
    //         " -- %d blocks, %d subbands\n",
    //         sbc_mode_str[frame.mode], srate_hz,
    //         sbc_get_frame_bitrate(&frame) * 1e-3, frame.bitpool,
    //         frame.nblocks, frame.nsubbands);

    int nch = 1 + (frame.mode != SBC_MODE_MONO);

    wave_write_header(fp_out, 16, sizeof(*pcm), sbc_get_freq_hz(frame.freq),
                      nch, -1);

    sbc_reset(&sbc);

    /* --- Decoding loop --- */

    for (int i = 0; i == 0 || (memfread(data, SBC_PROBE_SIZE, 1, fp_in) >= 1 &&
                               sbc_probe(data, &frame) == 0);
         i++) {

        if (memfread(data + SBC_PROBE_SIZE,
                     sbc_get_frame_size(&frame) - SBC_PROBE_SIZE, 1, fp_in) < 1)
            break;

        BENCH_START(sbc_decode);
        sbc_decode(&sbc, data, sizeof(data), &frame, pcm + 0, nch, pcm + 1, 2);
        BENCH_END(sbc_decode);

        int npcm = frame.nblocks * frame.nsubbands;
        wave_write_pcm(fp_out, sizeof(*pcm), pcm, nch, 0, npcm);
    }

    /* --- Cleanup --- */

    memfclose(&fp_in);
    memfclose(&fp_out);

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    printf("Start Encoding...\r\n");
    if (encode() != EXIT_SUCCESS) {
        printf("FAIL\r\n");
        return EXIT_FAILURE;
    }
    if (verify_result(enc_sbc, enc_result, ENC_SBC_LEN, 0) != EXIT_SUCCESS) {
        printf("FAIL\r\n");
        return EXIT_FAILURE;
    }

    printf("Start Decoding...\r\n");
    if (decode() != EXIT_SUCCESS) {
        printf("FAIL\r\n");
        return EXIT_FAILURE;
    }
    if (verify_result(dec_wav, dec_result, DEC_WAV_LEN, 0) != EXIT_SUCCESS) {
        printf("FAIL\r\n");
        return EXIT_FAILURE;
    }
    printf("PASS\r\n");
    return EXIT_SUCCESS;
}