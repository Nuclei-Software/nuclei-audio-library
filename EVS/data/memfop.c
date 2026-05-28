#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memfop.h"

MemoryFile *memfopen(void *arr, size_t size) {
    MemoryFile *mf = (MemoryFile *)malloc(sizeof(MemoryFile));
    if (mf != NULL) {
        mf->ptr = arr;
        mf->size = size;
        mf->position = 0;
    }
    return mf;
}

size_t memfwrite(const void *buffer, size_t itemSize, size_t count,
                 MemoryFile *mf) {
    size_t bytesToWrite = itemSize * count;
    size_t remaining = mf->size - mf->position;

    if (bytesToWrite > remaining) {
        bytesToWrite = remaining;
    }

    if (bytesToWrite > 0) {
        memcpy((char *)mf->ptr + mf->position, buffer, bytesToWrite);
        mf->position += bytesToWrite;
    }

    return bytesToWrite / itemSize;
}

size_t memfread(void *buffer, size_t itemSize, size_t count, MemoryFile *mf) {
    if (mf == NULL || buffer == NULL) {
        return 0;
    }

    size_t bytesToRead = itemSize * count;
    size_t remaining = mf->size - mf->position;

    if (bytesToRead > remaining) {
        bytesToRead = remaining;
    }

    if (bytesToRead > 0) {
        memcpy(buffer, (const char *)mf->ptr + mf->position, bytesToRead);
        mf->position += bytesToRead;
    }

    return bytesToRead / itemSize;
}

int memfseek(MemoryFile *mf, long offset, int whence) {
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
        return -1;
    }

    if (mf->position < 0 || mf->position > mf->size) {
        return -1;
    }

    return 0;
}

int memfputc(int c, MemoryFile *mf) {
    if (mf == NULL || mf->position >= mf->size) {
        return EOF;
    } else {
        ((unsigned char *)mf->ptr)[mf->position] = c;
        mf->position++;
    }
    return c;
}

int memfgetc(MemoryFile *mf) {
    if (mf == NULL || mf->position >= mf->size) {
        return EOF;
    }

    unsigned char ch = ((unsigned char *)mf->ptr)[mf->position];
    mf->position++;

    return ch;
}

char *memfgets(char *dst, int len, MemoryFile *src) {
    if (src == NULL || src->position >= src->size) {
        return NULL;
    }

    if (len > src->size - src->position) {
        return NULL;
    }

    memcpy(dst, (char *)src->ptr + src->position, len * sizeof(char));
    src->position += len;
    return dst;
}

long memftell(MemoryFile *mf) { return mf->position; }

int memfeof(MemoryFile *mf) {
    if (mf == NULL) {
        return 1;
    }
    return mf->position >= mf->size;
}

void memfclose(MemoryFile *mf) { free(mf); }

void memrewind(MemoryFile *mf) { mf->position = 0; }
int memfprintf(MemoryFile *mf, const char *format, ...) { return 0; }
