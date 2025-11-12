#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "data/dec_raw.h"
#include "data/enc_evrc.h"
#include "data/input.h"

#define INPUT_LEN (sizeof(input))
#define ENC_EVRC_LEN (sizeof(enc_evrc))
#define DEC_RAW_LEN (sizeof(dec_raw))

uint8_t enc_result[ENC_EVRC_LEN] = {0};
uint8_t dec_result[DEC_RAW_LEN] = {0};

constexpr int sample_rate = 8000;
constexpr double total_sec = INPUT_LEN * 1.0 / sizeof(short) / sample_rate;

#include "evrcc.h"
#include "nmsis_bench.h"

BENCH_DECLARE_VAR();

using namespace std;

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

int encodefile() {
    size_t nFrame = INPUT_LEN / 320;
    int16_t *pcm = (int16_t *)input;

    void *ct = evrc_encoder_init(4, 4, 1);
    BENCH_START(evrc_encode);
    int r = evrc_encoder_encode_to_stream(ct, (short *)&(pcm[0]),
                                          INPUT_LEN / sizeof(short), enc_result,
                                          1024 * 100);
    BENCH_SAMPLE(evrc_encode);
    /* NOTE: There is an overflow risk */
    unsigned long used_cycle = BENCH_GET_USECYC();

    double mcps = used_cycle * 1.0 / 1000000 / total_sec;
    printf("encode %d frames for %d ms, use %lu cycles\r\n", nFrame,
           nFrame * 20, used_cycle);
    printf("CSV, evrc_encode, %.02f\r\n", mcps);
    evrc_encoder_uninit(ct);
    return 0;
}

int decodefile() {
    void *ct = evrc_decoder_init();
    const int words = evrc_decoder_stream_max_sample(enc_evrc, ENC_EVRC_LEN);
    short *pcm_buf = (short *)dec_result;
    BENCH_START(evrc_decode);
    int bytes = evrc_decoder_decode_from_stream(ct, enc_evrc, ENC_EVRC_LEN,
                                                pcm_buf, words);
    BENCH_SAMPLE(evrc_decode);
    unsigned long used_cycle = BENCH_GET_USECYC();
    if (bytes > 0) {
        double mcps = used_cycle * 1.0 / 1000000 / total_sec;
        printf("decode %d frames\r\n", bytes / 320);
        printf("CSV, evrc_decode, %.02f\r\n", mcps);
    }
    evrc_decoder_uninit(ct);
    return 0;
}

int main(int argc, char *argv[]) {
    encodefile();
    if (verify_result(enc_evrc, enc_result, ENC_EVRC_LEN, 0) != EXIT_SUCCESS) {
        printf("FAIL\r\n");
        return EXIT_FAILURE;
    }
    decodefile();
    if (verify_result(dec_raw, dec_result, DEC_RAW_LEN, 0) != EXIT_SUCCESS) {
        printf("FAIL\r\n");
        return EXIT_FAILURE;
    }
    printf("PASS\r\n");
    return EXIT_SUCCESS;
}
