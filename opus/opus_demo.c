#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "in_1s.h"
#include "memfop.h"
#include "opus.h"
#include "nmsis_bench.h"

#define CHANNELS (1)
#define SAMPLING_RATE (16000)
#define FRAME_SIZE (SAMPLING_RATE / 50)
#define MAX_PACKET 1500
#define ENC_RESULT_BUFFER (3000)
#define DEC_RESULT_BUFFER (32768)

// #define DUMP_DEC_RESULT

int16_t frame[FRAME_SIZE * CHANNELS] = {0}; // data is processed by frame
uint8_t data[MAX_PACKET] = {0};             // temporary data for encoding output
uint8_t enc_result[ENC_RESULT_BUFFER] = {0};
uint8_t dec_result[DEC_RESULT_BUFFER] = {0};

BENCH_DECLARE_VAR();

static void int_to_char(opus_uint32 i, unsigned char ch[4])
{
    ch[0] = i >> 24;
    ch[1] = (i >> 16) & 0xFF;
    ch[2] = (i >> 8) & 0xFF;
    ch[3] = i & 0xFF;
}

static opus_uint32 char_to_int(unsigned char ch[4])
{
    return ((opus_uint32)ch[0] << 24) | ((opus_uint32)ch[1] << 16) | ((opus_uint32)ch[2] << 8) | (opus_uint32)ch[3];
}

int encode(opus_int32 sampling_rate, int channels, int application)
{
    // create encoder
    OpusEncoder *enc = NULL;
    int err;
    enc = opus_encoder_create(sampling_rate, channels, application, &err);
    if (err != OPUS_OK) {
        printf("Cannot create encoder: %s\r\n", opus_strerror(err));
        return -1;
    }

    // encoder configuration
    opus_encoder_ctl(enc, OPUS_SET_BITRATE(OPUS_AUTO));
    opus_encoder_ctl(enc, OPUS_SET_BANDWIDTH(OPUS_AUTO));
    opus_encoder_ctl(enc, OPUS_SET_VBR(1));
    opus_encoder_ctl(enc, OPUS_SET_VBR_CONSTRAINT(0));
    opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(10));
    opus_encoder_ctl(enc, OPUS_SET_INBAND_FEC(0));
    opus_encoder_ctl(enc, OPUS_SET_FORCE_CHANNELS(OPUS_AUTO));
    opus_encoder_ctl(enc, OPUS_SET_DTX(0));
    opus_encoder_ctl(enc, OPUS_SET_PACKET_LOSS_PERC(0));
    opus_encoder_ctl(enc, OPUS_SET_LSB_DEPTH(16)); // input data width
    opus_encoder_ctl(enc, OPUS_SET_EXPERT_FRAME_DURATION(OPUS_FRAMESIZE_ARG));

    // open input and output file
    MemoryFile *fin = NULL;
    MemoryFile *fenc = NULL;
    fin = memfopen((void *)in_1s_raw, in_1s_raw_len * sizeof(uint8_t));
    fenc = memfopen((void *)enc_result, ENC_RESULT_BUFFER * sizeof(uint8_t));

    int stop = 0;
    size_t num_read;
    int remaining = 0;
    while (!stop) {
        // read data from input, concat with previous remaining data
        int samples_to_read = FRAME_SIZE - remaining;
        num_read = memfread(frame + remaining * channels, sizeof(int16_t) * channels, samples_to_read, fin);
        if (num_read == 0) {
            // there is no more data to read
            break;
        }

        if (num_read < samples_to_read) {
            // if data not enough, fill with zero
            memset(frame + (num_read + remaining) * channels, 0, (samples_to_read - num_read) * channels * sizeof(int16_t));
            stop = 1;
        }

        // encode
        BENCH_START(opus_encode)
        int len = opus_encode(enc, frame, FRAME_SIZE, data, MAX_PACKET);
        BENCH_END(opus_encode)
        if (len < 0) {
            printf("Error encoding.\r\n");
            return -1;
        }

        // calculate samples in data and updata remaining samples
        int nb_encoded = opus_packet_get_samples_per_frame(data, sampling_rate) * opus_packet_get_nb_frames(data, len);
        remaining = FRAME_SIZE - nb_encoded;
        for (int i = 0; i < remaining * channels; i++) {
            frame[i] = frame[nb_encoded * channels + i];
        }

        // store data in format: [len, encoder state, payload]
        unsigned char int_field[4];
        int_to_char(len, int_field);
        if (memfwrite(int_field, 1, 4, fenc) != 4) {
            printf("Error writing.\r\n");
            return -1;
        }
        opus_uint32 enc_final_range;
        opus_encoder_ctl(enc, OPUS_GET_FINAL_RANGE(&enc_final_range));
        int_to_char(enc_final_range, int_field);
        if (memfwrite(int_field, 1, 4, fenc) != 4) {
            printf("Error writing.\r\n");
            return -1;
        }
        if (memfwrite(data, 1, len, fenc) != (unsigned)len) {
            printf("Error writing.\r\n");
            return -1;
        }
    }

    int encoded_length = memftell(fenc);
    printf("Encoded buffer length: %d bytes\r\n", encoded_length);

    opus_encoder_destroy(enc);
    if (fin)
        memfclose(&fin);
    if (fenc)
        memfclose(&fenc);

    return encoded_length;
}

int decode(opus_int32 sampling_rate, int channels, size_t encoded_length)
{
    OpusDecoder *dec = NULL;
    int err;
    dec = opus_decoder_create(sampling_rate, channels, &err);
    if (err != OPUS_OK) {
        printf("Cannot create decoder: %s\r\n", opus_strerror(err));
        return EXIT_FAILURE;
    }
    opus_decoder_ctl(dec, OPUS_SET_COMPLEXITY(0));

    MemoryFile *fenc = NULL;
    MemoryFile *fdec = NULL;
    fenc = memfopen((void *)enc_result, encoded_length * sizeof(uint8_t));
    fdec = memfopen((void *)dec_result, DEC_RESULT_BUFFER * sizeof(uint8_t));

    unsigned char ch[4];
    size_t num_read;
    for (;;) {
        num_read = memfread(ch, 1, 4, fenc);
        if (num_read != 4) {
            // no more new data
            break;
        }
        int len = char_to_int(ch);
        if (len > MAX_PACKET || len <= 0) {
            printf("Invalid payload length: %d\r\n", len);
            return EXIT_FAILURE;
        }

        num_read = memfread(ch, 1, 4, fenc);
        if (num_read != 4) {
            printf("Error reading.\r\n");
            return EXIT_FAILURE;
        }
        opus_uint32 enc_final_range = char_to_int(ch);
        num_read = memfread(data, 1, len, fenc);
        if (num_read != (size_t)len) {
            printf("Ran out of input, expecting %d bytes got %d\n", len, num_read);
            return EXIT_FAILURE;
        }

        opus_int32 output_samples;
        BENCH_START(opus_decode)
        output_samples = opus_decode(dec, data, len, frame, FRAME_SIZE, 0);
        BENCH_END(opus_decode)

        if (output_samples > 0) {
            if (memfwrite(frame, sizeof(int16_t) * channels, output_samples, fdec) != output_samples) {
                printf("Error writing.\r\n");
                return EXIT_FAILURE;
            }
        } else {
            printf("Error decoding.\r\n");
            return EXIT_FAILURE;
        }
    }

    int decoded_length = memftell(fdec);
    printf("Decoded buffer length: %d bytes\r\n", decoded_length);

#ifdef DUMP_DEC_RESULT
    printf("\r\n[");
    int cnt = 0;
    while (cnt + 50 <= decoded_length) {
        printf("\r\n");
        for (int i = 0; i < 50; ++i) {
            printf("%02x ", dec_result[cnt++]);
        }
    }
    if (cnt < decoded_length) {
        printf("\r\n");
        for (int i = 0; i < decoded_length - cnt; ++i) {
            printf("%02x ", dec_result[cnt++]);
        }
    }
    printf("\r\n]\r\n");
#endif

    opus_decoder_destroy(dec);
    if (fenc)
        memfclose(&fenc);
    if (fdec)
        memfclose(&fdec);
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    printf("Input data length: %d bytes\r\n", in_1s_raw_len);

    int len;
    printf("Start encoding...\r\n");
    len = encode(16000, 1, OPUS_APPLICATION_AUDIO);
    if (len <= 0) {
        return EXIT_FAILURE;
    }

    printf("Start decoding...\r\n");
    int status = decode(16000, 1, len);
    if (status != EXIT_SUCCESS) {
        return status;
    }

    return EXIT_SUCCESS;
}
