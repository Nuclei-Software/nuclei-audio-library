#include <assert.h>
#include <stdio.h>

#include "defines.h"
#include "lc3.h"
#include "memfop.h"
#include "tinywavein_c.h"
#include "tinywaveout_c.h"

#include "dec.h"
#include "enc.h"
#include "input.h"

#define TARGET_BITRATE (20000)
#define ARG_LFE (0)
#define ARG_EPMODE (0)
#define ARG_HRMODE (0)
#ifndef FIXED_POINT
#define FRAME_TIME (10)
#endif

int sample_buf[LC3PLUS_MAX_CHANNELS * LC3PLUS_MAX_SAMPLES];
int16_t buf_16[LC3PLUS_MAX_CHANNELS * LC3PLUS_MAX_SAMPLES];
uint8_t bytes[LC3PLUS_MAX_BYTES];
int32_t sample_buf_int[LC3PLUS_MAX_CHANNELS * LC3PLUS_MAX_SAMPLES] = {0};
int16_t *sample_buf_short = (int16_t *)(void *)sample_buf_int;

uint8_t enc_result[ENC_BIN_LEN];
uint8_t dec_result[DEC_WAV_LEN];

static MemoryFile *open_bitstream_writer(void *buffer, size_t data,
                                         uint32_t samplerate, int bitrate,
                                         short channels, uint32_t signal_len,
                                         float frame_ms, int epmode,
                                         int32_t hrmode);
static void write_bitstream_frame(MemoryFile *bitstream_file, uint8_t *bytes,
                                  int size);

static MemoryFile *
open_bitstream_reader(void *buffer, size_t data, unsigned int *samplerate,
                      int *bitrate, short *channels, unsigned int *signal_len,
                      float *frame_ms, int *epmode, int *hrmode);
static int read_bitstream_frame(MemoryFile *bitstream_file, uint8_t *bytes,
                                int size);

static int encode(WAVEFILEIN *fin, MemoryFile *fenc, uint32_t sampleRate,
                  int16_t nChannels, int bitrate)
{
    LC3PLUS_Error err = LC3PLUS_OK;
    /* Setup Encoder */
    int encoder_size = lc3plus_enc_get_size(sampleRate, nChannels);
    LC3PLUS_Enc *encoder = malloc(encoder_size);
    err = lc3plus_enc_init(encoder, sampleRate, nChannels
#ifdef ENABLE_HR_MODE
                           ,
                           arg.hrmode
#endif
    );
    if (err != LC3PLUS_OK) {
        printf("Error initializing encoder.\r\n");
        return EXIT_FAILURE;
    }

#ifdef FIXED_POINT
    lc3plus_enc_set_frame_dms(encoder, (int)(FRAME_TIME * 10));
#else
    lc3plus_enc_set_frame_ms(encoder, (int)(FRAME_TIME));
#endif
    lc3plus_enc_set_ep_mode(encoder, (LC3PLUS_EpMode)ARG_EPMODE);
    lc3plus_enc_set_lfe(encoder, ARG_LFE);
    lc3plus_enc_set_bitrate(encoder, bitrate);

    uint32_t nSamples = lc3plus_enc_get_input_samples(encoder);
    int real_bitrate = lc3plus_enc_get_real_bitrate(encoder);
    (void)real_bitrate;

#ifdef FIXED_POINT
    int scratch_size = lc3plus_enc_get_scratch_size(encoder);
    void *scratch = malloc(scratch_size);
    if (scratch == NULL) {
        printf("Failed to allocate scratch memory!\r\n");
        return EXIT_FAILURE;
    }
#endif

    while (1) {
        /* read audio data */
        unsigned int nSamplesRead = 0;
        ReadWavInt(fin, sample_buf, nSamples * nChannels, &nSamplesRead);
        /* zero out rest of last frame */
        memset(sample_buf + nSamplesRead, 0,
               (nSamples * nChannels - nSamplesRead) * sizeof(sample_buf[0]));

        if (nSamplesRead == 0) {
            break;
        }

        /* deinterleave channels */
        int16_t *input16[] = {buf_16, buf_16 + nSamples};
        for (int ch = 0; ch < nChannels; ch++) {
            for (int i = 0; i < (int)nSamples; i++) {
                input16[ch][i] = sample_buf[i * nChannels + ch];
            }
        }

        int nBytes = 0;
        err = lc3plus_enc16(encoder, input16, bytes, &nBytes
#ifdef FIXED_POINT
                            ,
                            scratch
#endif
        );
        if (err != LC3PLUS_OK) {
            printf("Error encoding!\r\n");
            return EXIT_FAILURE;
        }

        write_bitstream_frame(fenc, bytes, nBytes);
    }

    free(encoder);
#ifdef FIXED_POINT
    free(scratch);
#endif

    return EXIT_SUCCESS;
}

static int decode(MemoryFile *fenc, WAVEFILEOUT *fdec, uint32_t sampleRate,
                  int16_t nChannels, uint32_t nSamplesFile, float frame_ms,
                  int epmode)
{
    LC3PLUS_Error err = LC3PLUS_OK;

    /* Setup Decoder */
    int decoder_size = lc3plus_dec_get_size(sampleRate, nChannels
#ifdef FIXED_POINT
                                            ,
                                            LC3PLUS_PLC_ADVANCED
#endif
    );
    LC3PLUS_Dec *decoder = malloc(decoder_size);
    err = lc3plus_dec_init(decoder, sampleRate, nChannels, LC3PLUS_PLC_ADVANCED
#ifdef ENABLE_HR_MODE
                           ,
                           arg.hrmode
#endif
    );
    if (err != LC3PLUS_OK) {
        printf("Error initializing decoder.\r\n");
        return EXIT_FAILURE;
    }

#ifdef FIXED_POINT
    lc3plus_dec_set_frame_dms(decoder, (int)(frame_ms * 10));
#else
    lc3plus_dec_set_frame_ms(decoder, (int)(frame_ms));
#endif
    lc3plus_dec_set_ep_enabled(decoder, epmode != 0);

    int delay = lc3plus_dec_get_delay(decoder);
    int nSamples = lc3plus_dec_get_output_samples(decoder);

#ifdef FIXED_POINT
    int scratch_size = lc3plus_dec_get_scratch_size(decoder);
    void *scratch = malloc(scratch_size);
    if (scratch == NULL) {
        printf("Failed to allocate scratch memory!\r\n");
        return EXIT_FAILURE;
    }
#endif

    while (1) {
        /* Read bitstream */
        int nBytes = read_bitstream_frame(fenc, bytes, sizeof(bytes));
        if (nBytes < 0) {
            break;
        }

        int16_t *output16[LC3PLUS_MAX_CHANNELS];

        for (int i = 0; i < nChannels; i++) {
            output16[i] = buf_16 + i * nSamples;
        }

        /* Run Decoder */
        err = lc3plus_dec16(decoder, bytes, nBytes, output16
#ifdef FIXED_POINT
                            ,
                            scratch
#endif
                            ,
                            0);
        if (err != LC3PLUS_OK) {
            printf("Error decoding!\r\n");
            return EXIT_FAILURE;
        }

        uint32_t out_samples = MIN((uint32_t)nSamples - delay, nSamplesFile);

        // interleave_short
        for (int ch = 0; ch < nChannels; ch++) {
            for (int i = 0; i < nSamples; i++) {
                sample_buf_short[i * nChannels + ch] = output16[ch][i];
            }
        }
        WriteWavShort(fdec, sample_buf_short + delay * nChannels,
                      out_samples * nChannels);

        nSamplesFile -= out_samples;
        delay = 0;
    }

    free(decoder);
#ifdef FIXED_POINT
    free(scratch);
#endif

    return EXIT_SUCCESS;
}

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

int main(int argc, char *argv[])
{
    WAVEFILEIN *input_wav;
    WAVEFILEOUT *output_wav;
    unsigned int sampleRate, nSamplesFile = 0xffffffff;
    int16_t nChannels, bipsIn;

    (void)argc;
    (void)argv;

    printf("Start encoding...\r\n");

    //   Open input wav file
    input_wav = OpenWav((void *)in_1s_wav, in_1s_wav_len, &sampleRate,
                        &nChannels, &nSamplesFile, &bipsIn);
    if (input_wav == NULL) {
        printf("Error opening wav file!\r\n");
        return EXIT_FAILURE;
    }

    // Open output bin file
    MemoryFile *output_bitstream = open_bitstream_writer(
        enc_result, ENC_BIN_LEN, sampleRate, TARGET_BITRATE, nChannels,
        nSamplesFile, FRAME_TIME, ARG_EPMODE, ARG_HRMODE);
    if (output_bitstream == NULL) {
        printf("Error opening bitstream file!\r\n");
        return EXIT_FAILURE;
    }

    int status = encode(input_wav, output_bitstream, sampleRate, nChannels,
                        TARGET_BITRATE);
    if (status != EXIT_SUCCESS) {
        return status;
    }

    // Close input wav file and output bin file
    CloseWavIn(input_wav);
    memfclose(&output_bitstream);

    // verify result
    status = verify_result(enc_bin, enc_result, ENC_BIN_LEN, 0);
    if (status != EXIT_SUCCESS) {
        return status;
    }

    printf("Encoding done!\r\n");

    printf("Start decoding...\r\n");

    // Open input bin file
    int bitrate, epmode, hrmode;
    float frame_ms;
    MemoryFile *input_bitstream = open_bitstream_reader(
        enc_result, ENC_BIN_LEN, &sampleRate, &bitrate, &nChannels,
        &nSamplesFile, &frame_ms, &epmode, &hrmode);
    if (input_bitstream == NULL) {
        printf("Error opening bitstream file!\r\n");
        return EXIT_FAILURE;
    }

    // Open output wav file
    output_wav = CreateWav(dec_result, DEC_WAV_LEN, sampleRate, nChannels, 16);
    if (output_wav == NULL) {
        printf("Error creating wav file!\r\n");
        return EXIT_FAILURE;
    }

    status = decode(input_bitstream, output_wav, sampleRate, nChannels,
                    nSamplesFile, frame_ms, epmode);
    if (status != EXIT_SUCCESS) {
        return status;
    }

    memfclose(&input_bitstream);
    CloseWav(output_wav);

    // verify result
    status = verify_result(dec_wav, dec_result, DEC_WAV_LEN
#ifdef FIXED_POINT
                           ,
                           0
#else
                           ,
                           1
#endif
    );
    if (status != EXIT_SUCCESS) {
        return status;
    }

    printf("Decoding done!\r\n");

    return EXIT_SUCCESS;
}

static MemoryFile *open_bitstream_writer(void *buffer, size_t size,
                                         uint32_t samplerate, int bitrate,
                                         short channels, uint32_t signal_len,
                                         float frame_ms, int epmode,
                                         int32_t hrmode)
{
    MemoryFile *f = memfopen(buffer, size);
    MemoryFile *f_use = f;

    if (f_use) {
        uint16_t header[10] = {0xcc1c,
                               sizeof(header),
                               samplerate / 100,
                               bitrate / 100,
                               channels,
                               (uint16_t)(frame_ms * 100),
                               epmode > 0 ? 1 : 0,
                               signal_len,
                               signal_len >> 16,
                               hrmode};
        memfwrite(&header, sizeof(header), 1, f_use);
    }

    return f;
}

static void write_bitstream_frame(MemoryFile *bitstream_file, uint8_t *bytes,
                                  int size)
{

    int i = 0;
    uint16_t nbytes = size;
    memfwrite(&nbytes, sizeof(nbytes), 1, bitstream_file);
    for (i = 0; i < size; i++) {
        memfputc(bytes[i], bitstream_file);
    }
}

static MemoryFile *
open_bitstream_reader(void *buffer, size_t size, unsigned int *samplerate,
                      int *bitrate, short *channels, unsigned int *signal_len,
                      float *frame_ms, int *epmode, int *hrmode)
{
    MemoryFile *f = memfopen(buffer, size);
    MemoryFile *f_use = f;

    if (f_use) {
        uint16_t header[10] = {0};
        memfread(header, sizeof(header), 1, f_use);

        if (header[0] != 0xcc1c) {
            /* Old style bitstream header */
            *samplerate = header[0] * 100;
            *bitrate = header[1] * 100;
            *channels = header[2];
            memfseek(f_use, 6, SEEK_SET);
        } else {
            assert(header[1] >= 18);
            *samplerate = header[2] * 100;
            *bitrate = header[3] * 100;
            *channels = header[4];
            *frame_ms = (float)(header[5] / 100.0);
            *epmode = header[6];
            *signal_len = (uint32_t)header[7] | ((uint32_t)header[8] << 16);
            *hrmode = header[1] > 18 ? header[9] : 0;
            memfseek(f_use, header[1], SEEK_SET);
        }
    }

    return f;
}

static int read_bitstream_frame(MemoryFile *bitstream_file, uint8_t *bytes,
                                int size)
{
    int i = 0;
    uint16_t nbytes = 0;
    if (memfread(&nbytes, sizeof(nbytes), 1, bitstream_file) != 1) {
        return -1; /* End of file reached */
    }
    for (i = 0; i < nbytes && i < size; i++) {
        bytes[i] = memfgetc(bitstream_file);
    }
    return nbytes;
}
