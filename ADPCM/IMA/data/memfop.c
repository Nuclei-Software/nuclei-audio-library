#include "memfop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MemoryFile *memfopen(void *arr, size_t size)
{
    MemoryFile *mf = (MemoryFile *)malloc(sizeof(MemoryFile));
    if (mf != NULL) {
        mf->ptr = arr;
        mf->size = size;
        mf->position = 0;
    }
    return mf;
}

size_t memfwrite(const void *buffer, size_t itemSize, size_t count, MemoryFile *mf)
{
    size_t bytesToWrite = itemSize * count;
    size_t remaining = mf->size - mf->position;

    if (bytesToWrite > remaining) {
        bytesToWrite = remaining; // 防止越界
    }

    if (bytesToWrite > 0) {
        memcpy((char *)mf->ptr + mf->position, buffer, bytesToWrite);
        mf->position += bytesToWrite;
    }

    return bytesToWrite / itemSize; // 返回实际写入的数据个数
}

size_t memfread(void *buffer, size_t itemSize, size_t count, MemoryFile *mf)
{
    if (mf == NULL || buffer == NULL) {
        return 0; // 参数无效，返回0
    }

    size_t bytesToRead = itemSize * count;
    size_t remaining = mf->size - mf->position;

    if (bytesToRead > remaining) {
        bytesToRead = remaining; // 防止越界读取
    }

    if (bytesToRead > 0) {
        memcpy(buffer, (const char *)mf->ptr + mf->position,
               bytesToRead);         // 从当前位置读取到缓冲区
        mf->position += bytesToRead; // 更新读取位置
    }

    return bytesToRead / itemSize; // 返回实际读取的数据个数
}

int memfseek(MemoryFile *mf, long offset, int whence)
{
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

int memfputc(int c, MemoryFile *mf)
{
    if (mf == NULL || mf->position >= mf->size) {
        return EOF;
    } else {
        ((unsigned char *)mf->ptr)[mf->position] = c;
        mf->position++;
    }
    return c;
}

int memfgetc(MemoryFile *mf)
{
    if (mf == NULL || mf->position >= mf->size) {
        // 如果指针为空或已到达数组末尾，返回EOF
        return EOF;
    }

    unsigned char ch = ((unsigned char *)mf->ptr)[mf->position]; // 安全类型转换并读取字符
    mf->position++;                                              // 更新读取位置

    return ch; // 返回读取的字符
}

long memftell(MemoryFile *mf) { return mf->position; }

int memfeof(MemoryFile *mf)
{
    if (mf == NULL) {
        return 1; // 如果指针为空，视为EOF
    }
    return mf->position >= mf->size; // 如果位置大于等于数组大小，表示已到末尾
}

void memfclose(MemoryFile **mf)
{
    free(*mf);
    *mf = NULL;
}