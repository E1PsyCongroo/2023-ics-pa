#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
enum {
  FLAG=0, WIDTH=1, PRECISION=2, LENGTH=3, CONVERSION=4
};
enum {
  SHORTSHORT, SHORT, LONG, LONGLONG, SIZE, LONGDOUBLE, POINTSUB, MAXIMUMINT, DEFAULT
};
enum {
  LITERAL, SINGLECHAR, CHARSTRING, SIGNEDINT, OCTAL, HEXADECIMAL, UNSIGNEDINT,
  FLAOT, FLOATDECEXP, FLOATHEXEXP, FLAOTPRECISION, NUMWRITTEN, POINTER, NONE
};

typedef struct {
  unsigned int justify    : 1;
  unsigned int sign       : 1;
  unsigned int space      : 1;
  unsigned int prefix     : 2;
  unsigned int zero       : 1;
  unsigned int length     : 4;
  unsigned int conversion : 4;
  unsigned int width;
  unsigned int precision;
} Format;

static void flag_parser(Format *format, const char* fmt, va_list args);
static void width_parser(Format *format, const char* fmt, va_list args);
static void precision_parser(Format *format, const char* fmt, va_list args);
static void length_parser(Format *format, const char* fmt, va_list args);
static void conversion_parser(Format *format, const char* fmt, va_list args);

static struct {
  int stage;
  int (*handle)(Format *format, const char* fmt, va_list args);
} handle_table[] = {
  { FLAG, flag_parser },
  { WIDTH, width_parser },
  { PRECISION, precision_parser },
  { LENGTH, length_parser },
  { CONVERSION, conversion_parser },
};
#define TABLELEN (sizeof handle_table / sizeof handle_table[0])

static Format format_parser(const char* fmt, va_list args) {
  Format format = {
    .justify = 0,
    .sign = 0,
    .space = 0,
    .prefix = 0,
    .zero = 0,
    .width = 0,
    .precision = 1,
    .length = DEFAULT,
    .conversion = NONE,
  };
  int stage = FLAG;
  while (stage) {
    for (int i = 0; i < TABLELEN; i ++) {
      if (stage == handle_table[i].stage) {
        stage = handle_table[i].handle(&format, fmt, args);
        break;
      }
    }
  }
  return format;
}

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  Format format __attribute_maybe_unused__;
  int count = 0;
  while (*fmt) {
    if (*fmt != '%') {
      format = format_parser(fmt, ap);
    }
    else {
      *(out++) = *(fmt++);
      count++;
    }
  }
  return count;
}

int sprintf(char *out, const char *fmt, ...) {
  panic("Not implemented");
  va_list args;
  va_start(args, fmt);
  int ret = vsprintf(out, fmt, args);
  va_end(args);
  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
  va_list args;
  va_start(args, fmt);
  int ret = vsnprintf(out, n, fmt, args);
  va_end(args);
  return ret;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
