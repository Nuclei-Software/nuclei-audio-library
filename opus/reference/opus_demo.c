#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opus.h"

#define CHANNELS (1)
#define SAMPLING_RATE (16000)
#define FRAME_SIZE (SAMPLING_RATE / 50)
#define MAX_PACKET 1500

int16_t frame[FRAME_SIZE * CHANNELS] = {0}; // data is processed by frame
uint8_t data[MAX_PACKET] = {0};             // temporary data for encoding output

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
        return EXIT_FAILURE;
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
    FILE *fin = NULL;
    FILE *fenc = NULL;
    fin = fopen("in_1s.raw", "rb");
    fenc = fopen("enc.raw", "wb");

    int stop = 0;
    size_t num_read;
    int remaining = 0;
    for (int k = 0; k < 50; ++k) {
        // read data from input, concat with previous remaining data
        int samples_to_read = FRAME_SIZE - remaining;
        num_read = fread(frame + remaining * channels, sizeof(int16_t) * channels, samples_to_read, fin);

        if (num_read < samples_to_read) {
            // if data not enough, fill with zero
            memset(frame + (num_read + remaining) * channels, 0, (samples_to_read - num_read) * channels * sizeof(int16_t));
            stop = 1;
        }

        // encode
        int len = opus_encode(enc, frame, FRAME_SIZE, data, MAX_PACKET);
        if (len < 0) {
            printf("Error encoding.\r\n");
            return EXIT_FAILURE;
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
        if (fwrite(int_field, 1, 4, fenc) != 4) {
            printf("Error writing.\r\n");
            return EXIT_FAILURE;
        }
        opus_uint32 enc_final_range;
        opus_encoder_ctl(enc, OPUS_GET_FINAL_RANGE(&enc_final_range));
        int_to_char(enc_final_range, int_field);
        if (fwrite(int_field, 1, 4, fenc) != 4) {
            printf("Error writing.\r\n");
            return EXIT_FAILURE;
        }
        if (fwrite(data, 1, len, fenc) != (unsigned)len) {
            printf("Error writing.\r\n");
            return EXIT_FAILURE;
        }
    }

    opus_encoder_destroy(enc);
    if (fin)
        fclose(fin);
    if (fenc)
        fclose(fenc);

    return EXIT_SUCCESS;
}

int decode(opus_int32 sampling_rate, int channels)
{
    OpusDecoder *dec = NULL;
    int err;
    dec = opus_decoder_create(sampling_rate, channels, &err);
    if (err != OPUS_OK) {
        printf("Cannot create decoder: %s\r\n", opus_strerror(err));
        return EXIT_FAILURE;
    }
    opus_decoder_ctl(dec, OPUS_SET_COMPLEXITY(0));

    FILE *fenc = NULL;
    FILE *fdec = NULL;
    fenc = fopen("enc.raw", "rb");
    fdec = fopen("dec.raw", "wb");

    unsigned char ch[4];
    size_t num_read;
    for (;;) {
        num_read = fread(ch, 1, 4, fenc);
        if (num_read != 4) {
            // no more new data
            break;
        }
        int len = char_to_int(ch);
        if (len > MAX_PACKET || len < 0) {
            printf("Invalid payload length: %d\r\n", len);
            return EXIT_FAILURE;
        }
        num_read = fread(ch, 1, 4, fenc);
        if (num_read != 4) {
            printf("Error reading.\r\n");
            return EXIT_FAILURE;
        }
        opus_uint32 enc_final_range = char_to_int(ch);
        num_read = fread(data, 1, len, fenc);
        if (num_read != (size_t)len) {
            printf("Ran out of input, expecting %d bytes got %d\n", len, (int)num_read);
            return EXIT_FAILURE;
        }

        opus_int32 output_samples;
        output_samples = opus_decode(dec, data, len, frame, FRAME_SIZE, 0);

        if (output_samples > 0) {
            if (fwrite(frame, sizeof(int16_t) * channels, output_samples, fdec) != output_samples) {
                printf("Error writing.\r\n");
                return EXIT_FAILURE;
            }
        } else {
            printf("Error decoding.\r\n");
            return EXIT_FAILURE;
        }
    }

    opus_decoder_destroy(dec);
    if (fenc)
        fclose(fenc);
    if (fdec)
        fclose(fdec);
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    int status;
    status = encode(16000, 1, OPUS_APPLICATION_AUDIO);
    if (status != EXIT_SUCCESS) {
        return status;
    }

    status = decode(16000, 1);
    if (status != EXIT_SUCCESS) {
        return status;
    }

    return EXIT_SUCCESS;
}
