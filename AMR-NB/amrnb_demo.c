#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dec_wav.h"
#include "enc_amr.h"
#include "in_1s_8k.h"

#include "memfop.h"

#include "wavreader.h"
#include "wavwriter.h"
#include "wrapper.h"

#include "nmsis_bench.h"

uint8_t enc_result[ENC_AMR_LEN] = {0};
uint8_t dec_result[DEC_WAV_LEN] = {0};

BENCH_DECLARE_VAR();

/* From WmfDecBytesPerFrame in dec_input_format_tab.cpp */
const int sizes[] = {12, 13, 15, 17, 19, 20, 26, 31, 5, 6, 5, 5, 0, 0, 0, 0};

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
    enum Mode mode = MR122;
    int ch, dtx = 0;
    const char *infile, *outfile;
    MemoryFile *out;
    void *wav, *amr;
    int format, sampleRate, channels, bitsPerSample;
    int inputSize;
    uint8_t *inputBuf;

    wav = wav_read_open(in_1s_8k_wav, in_1s_8k_wav_len);
    if (!wav) {
        printf("Unable to open wav file %s\r\n", infile);
        return EXIT_FAILURE;
    }
    if (!wav_get_header(wav, &format, &channels, &sampleRate, &bitsPerSample,
                        NULL)) {
        printf("Bad wav file %s\r\n", infile);
        return EXIT_FAILURE;
    }
    if (format != 1) {
        printf("Unsupported WAV format %d\r\n", format);
        return EXIT_FAILURE;
    }
    if (bitsPerSample != 16) {
        printf("Unsupported WAV sample depth %d\r\n", bitsPerSample);
        return EXIT_FAILURE;
    }
    if (channels != 1)
        printf("Warning, only compressing one audio channel\r\n");
    if (sampleRate != 8000)
        printf(
            "Warning, AMR-NB uses 8000 Hz sample rate (WAV file has %d Hz)\r\n",
            sampleRate);
    inputSize = channels * 2 * 160;
    inputBuf = (uint8_t *)malloc(inputSize);

    amr = Encoder_Interface_init(dtx);
    out = memfopen(enc_result, ENC_AMR_LEN);
    if (!out) {
        perror(outfile);
        return EXIT_FAILURE;
    }

    memfwrite("#!AMR\n", 1, 6, out);
    while (1) {
        short buf[160];
        uint8_t outbuf[500];
        int read, i, n;
        read = wav_read_data(wav, inputBuf, inputSize);
        read /= channels;
        read /= 2;
        if (read < 160)
            break;
        for (i = 0; i < 160; i++) {
            const uint8_t *in = &inputBuf[2 * channels * i];
            buf[i] = in[0] | (in[1] << 8);
        }
        n = Encoder_Interface_Encode(amr, mode, buf, outbuf, 0);
        memfwrite(outbuf, 1, n, out);
    }
    free(inputBuf);
    memfclose(&out);
    Encoder_Interface_exit(amr);
    wav_read_close(wav);
    return EXIT_SUCCESS;
}

int decode() {
    MemoryFile *in;
    char header[6];
    int n;
    void *wav, *amr;

    in = memfopen((void *)enc_amr, ENC_AMR_LEN);
    if (!in) {
        printf("Unable to open amr file\r\n");
        return EXIT_FAILURE;
    }
    n = memfread(header, 1, 6, in);
    if (n != 6 || memcmp(header, "#!AMR\n", 6)) {
        printf("Bad header\n");
        return EXIT_FAILURE;
    }

    wav = wav_write_open(dec_result, DEC_WAV_LEN, 8000, 16, 1);
    if (!wav) {
        printf("Unable to open dec_result\r\n");
        return EXIT_FAILURE;
    }

    amr = Decoder_Interface_init();
    while (1) {
        uint8_t buffer[500], littleendian[320], *ptr;
        int size, i;
        int16_t outbuffer[160];
        /* Read the mode byte */
        n = memfread(buffer, 1, 1, in);
        if (n <= 0)
            break;
        /* Find the packet size */
        size = sizes[(buffer[0] >> 3) & 0x0f];
        n = memfread(buffer + 1, 1, size, in);
        if (n != size)
            break;

        /* Decode the packet */
        Decoder_Interface_Decode(amr, buffer, outbuffer, 0);

        /* Convert to little endian and write to wav */
        ptr = littleendian;
        for (i = 0; i < 160; i++) {
            *ptr++ = (outbuffer[i] >> 0) & 0xff;
            *ptr++ = (outbuffer[i] >> 8) & 0xff;
        }
        wav_write_data(wav, littleendian, 320);
    }
    memfclose(&in);
    Decoder_Interface_exit(amr);
    wav_write_close(wav);
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    printf("Start Encoding...\r\n");
    if (encode() != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (verify_result(enc_amr, enc_result, ENC_AMR_LEN, 0) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    printf("Start Decoding...\r\n");
    if (decode() != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (verify_result(dec_wav, dec_result, DEC_WAV_LEN, 0) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
}