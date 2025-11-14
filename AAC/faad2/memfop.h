#ifndef MEMFOP_H
#define MEMFOP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct {
    void *ptr;       // 指向数组的指针
    size_t size;     // 数组大小
    size_t position; // 当前读写位置
    int _flags;
} MemoryFile;

MemoryFile *memfopen(void *arr, size_t size);
size_t memfwrite(const void *buffer, size_t itemSize, size_t count,
                 MemoryFile *mf);
size_t memfread(void *buffer, size_t itemSize, size_t count, MemoryFile *mf);
int memfseek(MemoryFile *mf, long offset, int whence);
int memfputc(int c, MemoryFile *mf);
int memfgetc(MemoryFile *mf);
char *memfgets(char *dst, int, MemoryFile *src);
long memftell(MemoryFile *mf);
int memfeof(MemoryFile *mf);
int memfclose(MemoryFile **mf);
void memrewind(MemoryFile *mf);
static inline int memferror(MemoryFile *mf) { return 0; }

#ifdef __cplusplus
}
#endif

#endif