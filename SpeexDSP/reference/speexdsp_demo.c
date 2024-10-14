#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"

#define FRAME_SIZE (256)
#define FILTER_LENGTH (256 * 20)
#define SAMPLING_RATE (16000)

int16_t frame_buffer[FRAME_SIZE * 2] = {0};
int16_t input_frame[FRAME_SIZE] = {0};

int main(int argc, char *argv[])
{
    FILE *fin = NULL;
    FILE *fout = NULL;

    fin = fopen("in.raw", "rb");
    if (fin == NULL) {
        printf("Error opening input file.\r\n");
        return EXIT_FAILURE;
    }
    fout = fopen("out.raw", "wb");
    if (fout == NULL) {
        printf("Error opening output file.\r\n");
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
    spx_int32_t agc = 1; // on(1) off(2)
    speex_preprocess_ctl(preprocess_state, SPEEX_PREPROCESS_SET_AGC, &agc);
#endif

    size_t num_read = 0;
    int16_t *filtered_frame = frame_buffer;
    int16_t *last_frame = frame_buffer + FRAME_SIZE;

    for (;;) {
        num_read = fread(input_frame, sizeof(int16_t), FRAME_SIZE, fin);
        if (num_read == 0) {
            break;
        }

        // fill zero when data number is smaller than FRAME_SIZE
        if (num_read < FRAME_SIZE) {
            memset(input_frame + num_read, 0,
                   (FRAME_SIZE - num_read) * sizeof(int16_t));
        }

        // run echo cancellation
        speex_echo_cancellation(echo_state, input_frame, last_frame,
                                filtered_frame);
        // run preprocessing
        speex_preprocess_run(preprocess_state, filtered_frame);

        // write output
        if (fwrite(filtered_frame, sizeof(int16_t), num_read, fout) !=
            num_read) {
            printf("Error writing output file.\r\n");
            return EXIT_FAILURE;
        }

        // update
        int16_t *tmp = last_frame;
        last_frame = filtered_frame;
        filtered_frame = last_frame;
    }

    fclose(fin);
    fclose(fout);

    return EXIT_SUCCESS;
}
