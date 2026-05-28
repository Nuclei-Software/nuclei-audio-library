#ifndef MEMFOP_H
#define MEMFOP_H

#include <stdio.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *ptr;
    size_t size;
    size_t position;
    int _flags;
} MemoryFile;

MemoryFile *memfopen(void *arr, size_t size);
static inline MemoryFile *fopen_notsupported(const char *pathname, const char *mode) {
    return NULL;
}
static inline int memfscanf(MemoryFile *mf, const char *format, ...) { return 0; }
size_t memfwrite(const void *buffer, size_t itemSize, size_t count,
                 MemoryFile *mf);
size_t memfread(void *buffer, size_t itemSize, size_t count, MemoryFile *mf);
int memfseek(MemoryFile *mf, long offset, int whence);
int memfputc(int c, MemoryFile *mf);
int memfgetc(MemoryFile *mf);
char *memfgets(char *dst, int, MemoryFile *src);
long memftell(MemoryFile *mf);
int memfeof(MemoryFile *mf);
void memfclose(MemoryFile *mf);
void memrewind(MemoryFile *mf);
static inline int memferror(MemoryFile *mf) { return 0; }
int memfprintf(MemoryFile *mf, const char *format, ...);

#define FILE MemoryFile
#define fwrite memfwrite
#define fscanf memfscanf
#define fread memfread
#define fseek memfseek
#define fputc memfputc
#define fgetc memfgetc
#define fgets memfgets
#define ftell memftell
#define fclose memfclose
#define rewind memrewind

#ifdef feof
#undef feof
#endif
#define feof memfeof
#ifdef ferror
#undef ferror
#endif
#define ferror memferror
// #define fprintf memfprintf

#ifdef __cplusplus
}
#endif

#endif

