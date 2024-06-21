/* ------------------------------------------------------------------
 * Copyright (C) 2009 Martin Storsjo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */

#include "wavreader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

MemoryFile* memfopen(void* arr, size_t size) {
    MemoryFile* mf = (MemoryFile*)malloc(sizeof(MemoryFile));
    if(mf != NULL) {
        mf->ptr = arr;
        mf->size = size;
        mf->position = 0;
    }
    return mf;
}

size_t memfwrite(const void* buffer, size_t itemSize, size_t count, MemoryFile* mf) {
    size_t bytesToWrite = itemSize * count;
    size_t remaining = mf->size - mf->position;
    
    if (bytesToWrite > remaining) {
        bytesToWrite = remaining; // 防止越界
    }

    if (bytesToWrite > 0) {
        memcpy((char*)mf->ptr + mf->position, buffer, bytesToWrite);
        mf->position += bytesToWrite;
    }
    
    return bytesToWrite; // 返回实际写入的字节数
}

size_t memfread(void* buffer, size_t itemSize, size_t count, MemoryFile* mf) {
    if (mf == NULL || buffer == NULL) {
        return 0; // 参数无效，返回0
    }

    size_t bytesToRead = itemSize * count;
    size_t remaining = mf->size - mf->position;

    if (bytesToRead > remaining) {
        bytesToRead = remaining; // 防止越界读取
    }

    if (bytesToRead > 0) {
        memcpy(buffer, (const char*)mf->ptr + mf->position, bytesToRead); // 从当前位置读取到缓冲区
        mf->position += bytesToRead; // 更新读取位置
    }

    return bytesToRead; // 返回实际读取的字节数
}

int memfseek(MemoryFile* mf, long offset, int whence) {
    switch (whence) {
        case SEEK_SET:
            mf->position = offset;
            break;
        case SEEK_CUR:
            mf->position += offset;
            break;
        case SEEK_END:
            mf->position = mf->size + offset;
            break;
        default:
            return -1; // 错误的origin
    }
    
    // 检查是否越界
    if (mf->position < 0 || mf->position > mf->size) {
        return -1; // 越界错误
    }
    
    return 0; // 成功
}

int memfgetc(MemoryFile* mf) {
    if (mf == NULL || mf->position >= mf->size) {
        // 如果指针为空或已到达数组末尾，返回EOF
        return EOF;
    }
    
    unsigned char ch = ((unsigned char*)mf->ptr)[mf->position]; // 安全类型转换并读取字符
    mf->position++; // 更新读取位置
    
    return ch; // 返回读取的字符
}

long memftell(MemoryFile* mf) {
    return mf->position;
}

int memfeof(MemoryFile* mf) {
    if (mf == NULL) {
        return 1; // 如果指针为空，视为EOF
    }
    return mf->position >= mf->size; // 如果位置大于等于数组大小，表示已到末尾
}

void memfclose(MemoryFile* mf) {
    free(mf);
}

#define TAG(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

struct wav_reader {
	MemoryFile* wav;
	uint32_t data_length;

	int format;
	int sample_rate;
	int bits_per_sample;
	int channels;
	int byte_rate;
	int block_align;
};

static uint32_t read_tag(struct wav_reader* wr) {
	uint32_t tag = 0;
	tag = (tag << 8) | memfgetc(wr->wav);
	tag = (tag << 8) | memfgetc(wr->wav);
	tag = (tag << 8) | memfgetc(wr->wav);
	tag = (tag << 8) | memfgetc(wr->wav);
	return tag;
}

static uint32_t read_int32(struct wav_reader* wr) {
	uint32_t value = 0;
	value |= memfgetc(wr->wav) <<  0;
	value |= memfgetc(wr->wav) <<  8;
	value |= memfgetc(wr->wav) << 16;
	value |= memfgetc(wr->wav) << 24;
	return value;
}

static uint16_t read_int16(struct wav_reader* wr) {
	uint16_t value = 0;
	value |= memfgetc(wr->wav) << 0;
	value |= memfgetc(wr->wav) << 8;
	return value;
}

void* wav_read_open(void* arr, size_t size) {
	struct wav_reader* wr = (struct wav_reader*) malloc(sizeof(*wr));
	long data_pos = 0;
	memset(wr, 0, sizeof(*wr));

	wr->wav = memfopen(arr, size);
	if (wr->wav == NULL) {
		free(wr);
		return NULL;
	}

	while (1) {
		uint32_t tag, tag2, length;
		tag = read_tag(wr);
		if (memfeof(wr->wav))
			break;
		length = read_int32(wr);
		if (tag != TAG('R', 'I', 'F', 'F') || length < 4) {
			memfseek(wr->wav, length, SEEK_CUR);
			continue;
		}
		tag2 = read_tag(wr);
		length -= 4;
    
		if (tag2 != TAG('W', 'A', 'V', 'E')) {
			memfseek(wr->wav, length, SEEK_CUR);
			continue;
		}
		// RIFF chunk found, iterate through it
		while (length >= 8) {
			uint32_t subtag, sublength;
			subtag = read_tag(wr);
			if (memfeof(wr->wav))
				break;
			sublength = read_int32(wr);
			length -= 8;
			if (length < sublength)
				break;
			if (subtag == TAG('f', 'm', 't', ' ')) {
				if (sublength < 16) {
					// Insufficient data for 'fmt '
					break;
				}
				wr->format          = read_int16(wr);
				wr->channels        = read_int16(wr);
				wr->sample_rate     = read_int32(wr);
				wr->byte_rate       = read_int32(wr);
				wr->block_align     = read_int16(wr);
				wr->bits_per_sample = read_int16(wr);
				memfseek(wr->wav, sublength - 16, SEEK_CUR);
			} else if (subtag == TAG('d', 'a', 't', 'a')) {
				data_pos = memftell(wr->wav);
				wr->data_length = sublength;
				memfseek(wr->wav, sublength, SEEK_CUR);
			} else {
				memfseek(wr->wav, sublength, SEEK_CUR);
			}
			length -= sublength;
		}
		if (length > 0) {
			// Bad chunk?
			memfseek(wr->wav, length, SEEK_CUR);
		}
	}
	memfseek(wr->wav, data_pos, SEEK_SET);
	return wr;
}

void wav_read_close(void* obj) {
	struct wav_reader* wr = (struct wav_reader*) obj;
	memfclose(wr->wav);
	free(wr);
}

int wav_get_header(void* obj, int* format, int* channels, int* sample_rate, int* bits_per_sample, unsigned int* data_length) {
	struct wav_reader* wr = (struct wav_reader*) obj;
	if (format)
		*format = wr->format;
	if (channels)
		*channels = wr->channels;
	if (sample_rate)
		*sample_rate = wr->sample_rate;
	if (bits_per_sample)
		*bits_per_sample = wr->bits_per_sample;
	if (data_length)
		*data_length = wr->data_length;
	return wr->format && wr->sample_rate;
}

int wav_read_data(void* obj, unsigned char* data, unsigned int length) {
	struct wav_reader* wr = (struct wav_reader*) obj;
	int n;
	if (wr->wav == NULL)
		return -1;
	if (length > wr->data_length)
		length = wr->data_length;
	n = memfread(data, 1, length, wr->wav);
	wr->data_length -= length;
	return n;
}

