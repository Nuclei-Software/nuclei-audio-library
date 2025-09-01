#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"

#include "input.h"
#include "memfop.h"
#include "output.h"
#include "nmsis_bench.h"

#define FRAME_SIZE (256)
#define FILTER_LENGTH (256 * 20)
#define SAMPLING_RATE (16000)

int16_t frame_buffer[FRAME_SIZE * 2] = {0};
int16_t input_frame[FRAME_SIZE] = {0};
uint8_t output_result[OUT_RAW_LEN] = {0};

BENCH_DECLARE_VAR();

int verify_result(const uint8_t *ref, const uint8_t *res, int len,
                  int threshold)
{
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

int verify_result_i16(const int16_t *ref, const int16_t *res, int len,
                      int threshold) {
    for (int i = 0; i < len; i++) {
        if (abs(ref[i] - res[i]) > threshold) {
            printf(
                "Result mismatch at byte %d!, expected 0x%04x, got 0x%04x\r\n",
                i, ref[i], res[i]);
            return EXIT_FAILURE;
        }
    }
    printf("Result matches!\r\n");
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    MemoryFile *fin = NULL;
    MemoryFile *fout = NULL;

    fin = memfopen((void *)in_raw, in_raw_len);
    if (fin == NULL) {
        printf("Error opening input file.\r\n");
        printf("FAIL\r\n");
        return EXIT_FAILURE;
    }
    fout = memfopen(output_result, OUT_RAW_LEN);
    if (fout == NULL) {
        printf("Error opening output file.\r\n");
        printf("FAIL\r\n");
        return EXIT_FAILURE;
    }

    SpeexEchoState *echo_state =
        speex_echo_state_init(FRAME_SIZE, FILTER_LENGTH);
    SpeexPreprocessState *preprocess_state =
        speex_preprocess_state_init(FRAME_SIZE, SAMPLING_RATE);

    spx_int32_t denoise = 1; // on(1) off(2)
    speex_preprocess_ctl(preprocess_state, SPEEX_PREPROCESS_SET_DENOISE,
                         &denoise);
    // enable residual echo cancellation
    speex_preprocess_ctl(preprocess_state, SPEEX_PREPROCESS_SET_ECHO_STATE,
                         echo_state);
    spx_int32_t max_suppress = -10; // max suppress -10 dB
    speex_preprocess_ctl(preprocess_state, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS,
                         &max_suppress);
    speex_preprocess_ctl(preprocess_state, SPEEX_PREPROCESS_SET_ECHO_SUPPRESS,
                         &max_suppress);

#ifndef FIXED_POINT
    // NOTE: AGC is not supported in fixed-point version
    spx_int32_t agc = 1; // on(1) off(2)
    speex_preprocess_ctl(preprocess_state, SPEEX_PREPROCESS_SET_AGC, &agc);
#endif

    printf("Start processing...\r\n");

    size_t num_read = 0;
    int16_t *filtered_frame = frame_buffer;
    int16_t *last_frame = frame_buffer + FRAME_SIZE;

    for (;;) {
        num_read = memfread(input_frame, sizeof(int16_t), FRAME_SIZE, fin);
        if (num_read == 0) {
            break;
        }

        // fill zero when data number is smaller than FRAME_SIZE
        if (num_read < FRAME_SIZE) {
            memset(input_frame + num_read, 0,
                   (FRAME_SIZE - num_read) * sizeof(int16_t));
        }

        // run echo cancellation
        BENCH_START(speex_echo_cancellation)
        speex_echo_cancellation(echo_state, input_frame, last_frame,
                                filtered_frame);
        BENCH_END(speex_echo_cancellation)
        // run preprocessing
        BENCH_START(speex_preprocess_run)
        speex_preprocess_run(preprocess_state, filtered_frame);
        BENCH_END(speex_preprocess_run)

        // write output
        if (memfwrite(filtered_frame, sizeof(int16_t), num_read, fout) !=
            num_read) {
            printf("Error writing output file.\r\n");
            printf("FAIL\r\n");
            return EXIT_FAILURE;
        }

        // update
        int16_t *tmp = last_frame;
        last_frame = filtered_frame;
        filtered_frame = last_frame;
    }

    memfclose(&fin);
    memfclose(&fout);

#if defined(FIXED_POINT)
    if (verify_result(out_raw, output_result, OUT_RAW_LEN, 0) != EXIT_SUCCESS) {
        printf("FAIL\r\n");
        return EXIT_FAILURE;
    }
#else
    if (verify_result_i16((const int16_t *)out_raw, (int16_t *)output_result,
                          OUT_RAW_LEN / 2, 1) != EXIT_SUCCESS) {
        printf("FAIL\r\n");
        return EXIT_FAILURE;
    }
#endif

    printf("PASS\r\n");
    return EXIT_SUCCESS;
}
