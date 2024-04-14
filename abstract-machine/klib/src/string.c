#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t count = 0;
  while (*(s++)) { count++; }
  return count;
}

char *strcpy(char *dst, const char *src) {
  char *dst_cpy = dst;
  while (*src) {
    *(dst++) = *(src++);
  }
  *dst = '\0';
  return dst_cpy;
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *dst_cpy = dst;
  while (n-- && *src) {
    *(dst++) = *(src++);
  }
  while (n--) {
    *(dst++) = '\0';
  }
  return dst_cpy;
}

char *strcat(char *dst, const char *src) {
  char *dst_cpy = dst;
  while(*dst) { dst++; }
  while (*src) {
    *(dst++) = *(src++);
  }
  return dst_cpy;
}

int strcmp(const char *s1, const char *s2) {
  int ret = 0;
  while (*s1 && *s2) {
    if ((ret = *(s1++) - *(s2++))) { return ret; }
  }
  if (*s1) { ret = *s1; }
  else if (*s2) { ret = -*s2; }
  return ret;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  int ret = 0;
  while (*s1 && *s2 && n) {
    if ((ret = *(s1++) - *(s2++))) { return ret; }
    n--;
  }
  if (!n) { }
  else if (*s1) { ret = *s1; }
  else if (*s2) { ret = -*s2; }
  return ret;
}

void *memset(void *s, int c, size_t n) {
  unsigned char *p = s;
  while (n--) {
    *(p++) = (unsigned char)c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  if ((size_t)src > (size_t)dst && (size_t)dst + n > (size_t)src) {
    memcpy(dst, src, n);
  }
  else {
    unsigned char *dst_p = dst;
    const unsigned char *src_p = src ;
    while (n--) {
      dst_p[n] = src_p[n];
    }
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  unsigned char *out_p = out;
  const unsigned char *in_p = in;
  while (n--) {
    *(out_p++) = *(in_p++);
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *s1_p = s1, *s2_p = s2;
  int ret = 0;
  while (n--) {
    if ((ret = *(s1_p++) - *(s2_p++))) { return ret; }
  }
  return ret;
}

#endif
