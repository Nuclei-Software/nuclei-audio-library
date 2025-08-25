/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details. */

#include "ima_adpcm.h"
#include "wav.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dec_wav.h"
#include "enc_adp.h"
#include "input.h"

#include "memfop.h"

/* バージョン文字列 */
#define IMAADPCMCUI_VERSION_STRING "1.1.1"

/* ブロックサイズ 今の所1024で固定 */
#define IMAADPCMCUI_BLOCK_SIZE 1024

uint8_t enc_result[ENC_ADP_LEN] = {0};
uint8_t dec_result[DEC_WAV_LEN] = {0};

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

/* デコード処理 */
static int decode() {
    struct IMAADPCMWAVDecoder *decoder;
    struct IMAADPCMWAVHeaderInfo header;
    struct WAVFile *wav;
    struct WAVFileFormat wavformat;
    int16_t *output[IMAADPCM_MAX_NUM_CHANNELS];
    uint32_t ch, smpl;
    IMAADPCMApiResult ret;

    const uint8_t *buffer = enc_adp;
    uint32_t buffer_size = ENC_ADP_LEN;

    /* デコーダ作成 */
    decoder = IMAADPCMWAVDecoder_Create(NULL, 0);

    /* ヘッダ読み取り */
    if ((ret = IMAADPCMWAVDecoder_DecodeHeader(buffer, buffer_size, &header)) !=
        IMAADPCM_APIRESULT_OK) {
        printf("Failed to read header. API result: %d\r\n", ret);
        return 1;
    }

    /* 出力バッファ領域確保 */
    for (ch = 0; ch < header.num_channels; ch++) {
        output[ch] = malloc(sizeof(int16_t) * header.num_samples);
    }

    /* 全データをデコード */
    if ((ret = IMAADPCMWAVDecoder_DecodeWhole(
             decoder, buffer, buffer_size, output, header.num_channels,
             header.num_samples)) != IMAADPCM_APIRESULT_OK) {
        printf("Failed to decode. API result: %d\r\n", ret);
        return 1;
    }

    /* 出力ファイルを作成 */
    wavformat.data_format = WAV_DATA_FORMAT_PCM;
    wavformat.num_channels = header.num_channels;
    wavformat.sampling_rate = header.sampling_rate;
    wavformat.bits_per_sample = 16;
    wavformat.num_samples = header.num_samples;
    wav = WAV_Create(&wavformat);

    /* PCM書き出し */
    for (ch = 0; ch < header.num_channels; ch++) {
        for (smpl = 0; smpl < header.num_samples; smpl++) {
            WAVFile_PCM(wav, smpl, ch) = (output[ch][smpl] << 16);
        }
    }

    WAV_WriteToFile(dec_result, DEC_WAV_LEN, wav);

    IMAADPCMWAVDecoder_Destroy(decoder);
    for (ch = 0; ch < header.num_channels; ch++) {
        free(output[ch]);
    }
    WAV_Destroy(wav);

    return 0;
}

/* エンコード処理 */
static int encode() {
    MemoryFile *fp;
    struct WAVFile *wavfile;
    int16_t *input[IMAADPCM_MAX_NUM_CHANNELS];
    uint32_t ch, smpl, buffer_size, output_size;
    uint32_t num_channels, num_samples;
    uint8_t *buffer;
    struct IMAADPCMWAVEncodeParameter enc_param;
    struct IMAADPCMWAVEncoder *encoder;
    IMAADPCMApiResult api_result;

    /* 入力wav取得 */
    wavfile = WAV_CreateFromFile((void *)in_1s_wav, in_1s_wav_len);
    if (wavfile == NULL) {
        printf("Failed to open input file.\r\n");
        return EXIT_FAILURE;
    }

    num_channels = wavfile->format.num_channels;
    num_samples = wavfile->format.num_samples;

    /* 出力データの領域割当て */
    for (ch = 0; ch < num_channels; ch++) {
        input[ch] = malloc(sizeof(int16_t) * num_samples);
    }
    /* 入力wavと同じサイズの出力領域を確保（増えることはないと期待） */
    buffer_size = (uint32_t)in_1s_wav_len;
    buffer = malloc(buffer_size);

    /* 16bit幅でデータ取得 */
    for (ch = 0; ch < num_channels; ch++) {
        for (smpl = 0; smpl < num_samples; smpl++) {
            input[ch][smpl] = (int16_t)(WAVFile_PCM(wavfile, smpl, ch) >> 16);
        }
    }

    /* ハンドル作成 */
    encoder = IMAADPCMWAVEncoder_Create(NULL, 0);

    /* エンコードパラメータをセット */
    enc_param.num_channels = (uint16_t)num_channels;
    enc_param.sampling_rate = wavfile->format.sampling_rate;
    enc_param.bits_per_sample = 4;
    enc_param.block_size = IMAADPCMCUI_BLOCK_SIZE;
    if ((api_result = IMAADPCMWAVEncoder_SetEncodeParameter(
             encoder, &enc_param)) != IMAADPCM_APIRESULT_OK) {
        printf("Failed to set encode parameter. API result:%d\r\n", api_result);
        return 1;
    }

    /* エンコード */
    if ((api_result = IMAADPCMWAVEncoder_EncodeWhole(
             encoder, (const int16_t *const *)input, num_samples, buffer,
             buffer_size, &output_size)) != IMAADPCM_APIRESULT_OK) {
        printf("Failed to encode. API result:%d\r\n", api_result);
        return 1;
    }

    /* ファイル書き出し */
    fp = memfopen(enc_result, ENC_ADP_LEN);
    if (fp == NULL) {
        printf("Failed to open encoded file.\r\n");
        return 1;
    }
    if (memfwrite(buffer, sizeof(uint8_t), output_size, fp) < output_size) {
        printf("Warning: failed to write encoded data\r\n");
        return 1;
    }
    memfclose(&fp);

    /* 領域開放 */
    IMAADPCMWAVEncoder_Destroy(encoder);
    free(buffer);
    for (ch = 0; ch < num_channels; ch++) {
        free(input[ch]);
    }
    WAV_Destroy(wavfile);

    return 0;
}

/* メインエントリ */
int main(int argc, char **argv) {

    printf("Start Encoding...\r\n");
    if (encode() != EXIT_SUCCESS) {
        printf("FAIL\r\n");
        return EXIT_FAILURE;
    }
    if (verify_result(enc_adp, enc_result, ENC_ADP_LEN, 0) != EXIT_SUCCESS) {
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
