#include <errno.h>
#include <helix_mp3.h>
#include <stdio.h>
#include <stdlib.h>

#include "input.h"
#include "memfop.h"
#include "output.h"

#define SAMPLES_PER_FRAME 2
#define PCM_BUFFER_SIZE_SAMPLES (1024 * 32)
#define PCM_BUFFER_SIZE_FRAMES (PCM_BUFFER_SIZE_SAMPLES / SAMPLES_PER_FRAME)

uint8_t output_result[OUT_RAW_LEN] = {0};

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

int main(int argc, char *argv[]) {
    helix_mp3_t mp3;
    int16_t pcm_buffer[PCM_BUFFER_SIZE_SAMPLES];
    int err;
    MemoryFile *out_fd;

    /* Initialize decoder */
    err = helix_mp3_init_file(&mp3, in_1s_32k_mp3, in_1s_32k_mp3_len);
    if (err) {
        printf("Failed to init decoder, error: %d\r\n", err);
        printf("FAIL\r\n");
        return err;
    }

    do {
        /* Open output file */
        out_fd = memfopen(output_result, OUT_RAW_LEN);
        if (out_fd == NULL) {
            printf("Failed to open output file\r\n");
            printf("FAIL\r\n");
            err = -EIO;
            return err;
        }

        printf("Start Decoding...\r\n");

        /* Decode the whole file */
        while (1) {
            const size_t frames_read = helix_mp3_read_pcm_frames_s16(
                &mp3, pcm_buffer, PCM_BUFFER_SIZE_FRAMES);
            if (frames_read == 0) {
                printf("Reached EOF!\r\n");
                break;
            }
            const size_t frames_written =
                memfwrite(pcm_buffer, sizeof(*pcm_buffer) * SAMPLES_PER_FRAME,
                          frames_read, out_fd);
            if (frames_written != frames_read) {
                printf("Failed to write decoded frames, expected %u frames, "
                       "written %u frames!\r\n",
                       frames_read, frames_written);
                printf("FAIL\r\n");
                err = -EIO;
                return err;
            }
        }

        const size_t frame_count = helix_mp3_get_pcm_frames_decoded(&mp3);
        const uint32_t sample_rate = helix_mp3_get_sample_rate(&mp3);
        const uint32_t bitrate = helix_mp3_get_bitrate(&mp3);
        printf("Done! Decoded %u frames, last frame sample rate: %u Hz, "
               "bitrate: %u kbps\n",
               frame_count, sample_rate, bitrate / 1000);

    } while (0);

    if (verify_result(out_32k_raw, output_result, 0, 0) != EXIT_SUCCESS) {
        printf("FAIL\r\n");
        return EXIT_FAILURE;
    }

    /* Cleanup */
    if (out_fd != NULL) {
        memfclose(&out_fd);
    }
    helix_mp3_deinit(&mp3);

    printf("PASS\r\n");
    return EXIT_SUCCESS;
}
