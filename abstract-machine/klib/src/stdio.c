#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
enum {
  FLAG=0, WIDTH=1, PRECISION=2, LENGTH=3, CONVERSION=4
};
enum {
  SHORTSHORT, SHORT, LONG, LONGLONG, SIZE, LONGDOUBLE, POINTERSUB, MAXIMUMINT, DEFAULT
};

typedef struct {
  unsigned int justify    : 1;
  unsigned int sign       : 1;
  unsigned int space      : 1;
  unsigned int prefix     : 2;
  unsigned int zero       : 1;
  unsigned int length     : 4;
  unsigned int width;
  unsigned int precision;
  unsigned int conversion;
} FormatOptions;

static int flag_parser(FormatOptions *format, const char** fmt, va_list args);
static int width_parser(FormatOptions *format, const char** fmt, va_list args);
static int precision_parser(FormatOptions *format, const char** fmt, va_list args);
static int length_parser(FormatOptions *format, const char** fmt, va_list args);
static int conversion_parser(FormatOptions *format, const char** fmt, va_list args);

static int format_char(char *out, FormatOptions format, va_list args);
static int format_integer(char *out, FormatOptions format, va_list args);
static int format_float(char *out, FormatOptions format, va_list args);
static int format_pointer(char *out, FormatOptions format, va_list args);
/* helper function */
static int itos(int num, char* str);

static struct {
  int stage;
  int (*handle)(FormatOptions *format, const char** fmt, va_list args);
} handle_table[] = {
  { FLAG, flag_parser },
  { WIDTH, width_parser },
  { PRECISION, precision_parser },
  { LENGTH, length_parser },
  { CONVERSION, conversion_parser },
};
#define TABLELEN (sizeof handle_table / sizeof handle_table[0])

static int flag_parser(FormatOptions *format, const char** fmt, va_list args) {
  int doing = 1;
  while (**fmt && doing) {
    switch (**fmt) {
      case '-': format->justify = 1;break;
      case '+': format->sign = 1;   break;
      case ' ': format->space = 1;  break;
      case '#': format->prefix = 1; break;
      case '0': format->zero = 1;   break;
      default: doing = 0;           break;
    }
    (*fmt)++;
  }
  /* handle conflict */
  if (format->sign) { format->space = 0; }
  if (format->justify) { format->zero = 0; }
  return **fmt;
}

static int width_parser(FormatOptions *format, const char** fmt, va_list args) {
  if (**fmt == '*') {
    format->width = va_arg(args, int);
    if (format->width < 0) {
      format->width = -format->width;
      format->justify = 1;
      /* handle conflict */
      format->zero = 0;
    }
    (*fmt)++;
  }
  else if (**fmt >= '0' && **fmt <= '9') {
    while (**fmt >= '0' && **fmt <= '9') {
      format->width = format->width * 10 + **fmt - '0';
      (*fmt)++;
    }
  }
  return **fmt;
}

static int precision_parser(FormatOptions *format, const char** fmt, va_list args) {
  if (**fmt == '.') {
    (*fmt)++;
    if (**fmt == '*') {
      int prec = va_arg(args, int);
      format->precision = prec > 0 ? prec : format->precision;
      (*fmt)++;
    }
    else if (**fmt >= '0' && **fmt <= '9') {
      format->precision = 0;
      while (**fmt >= '0' && **fmt <= '9') {
        format->precision =  format->precision * 10 + **fmt - '0';
        (*fmt)++;
      }
    }
    else {
      format->precision = 0;
    }
  }
  return **fmt;
}

static int length_parser(FormatOptions *format, const char** fmt, va_list args) {
  switch (**fmt) {
  case 'h':
    if (*(*fmt + 1) == 'h') {
      format->length = SHORTSHORT;
      (*fmt) += 2;
    }
    else {
      format->length = SHORT;
      (*fmt)++;
    }
    break;
  case 'l':
    if (*(*fmt + 1) == 'l') {
      format->length = LONGLONG;
      (*fmt) += 2;
    }
    else {
      format->length = LONG;
      (*fmt)++;
    }
    break;
  case 'j': format->length = MAXIMUMINT; (*fmt)++; break;
  case 'z': format->length = SIZE;       (*fmt)++; break;
  case 't': format->length = POINTERSUB; (*fmt)++; break;
  case 'L': format->length = LONGDOUBLE; (*fmt)++; break;
  default: break;
  }
  return **fmt;
}

static int conversion_parser(FormatOptions *format, const char** fmt, va_list args) {
  format->conversion = **fmt;
  switch (**fmt) {
  case '%': break;
  case 'c':
    panic("Not implemented");
  case 's': break;
  case 'd':
  case 'i':
    if (format->precision == -1) { format->precision = 1; }
    break;
  case '0':
  case 'x':
  case 'X':
  case 'u':
  case 'f':
  case 'F':
  case 'e':
  case 'E':
  case 'a':
  case 'A':
  case 'g':
  case 'G':
  case 'n':
  case 'p':
  default: return **fmt;
  }
  (*fmt)++;
  return **fmt;
}

static FormatOptions format_parser(const char** fmt, va_list args) {
  FormatOptions format = {
    .justify = 0,
    .sign = 0,
    .space = 0,
    .prefix = 0,
    .zero = 0,
    .width = 0,
    .precision = -1,  /* -1 means default */
    .length = DEFAULT,
    .conversion = 0,
  };
  for (int stage = 4; stage < TABLELEN; stage++) {
    if (!handle_table[stage].handle(&format, fmt, args) ) { break; }
  }
  return format;
}

static int format_char(char *out, FormatOptions format, va_list args) {
  panic("Not implemented");
}

static int format_integer(char *out, FormatOptions format, va_list args) {
  panic("Not implemented");
}

static int format_float(char *out, FormatOptions format, va_list args) {
  panic("Not implemented");
}

static int format_pointer(char *out, FormatOptions format, va_list args) {
  panic("Not implemented");
}

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      FormatOptions format = format_parser(&fmt, ap);
      switch (format.conversion) {
      case 'd': {
        int num = va_arg(ap, int);
        int num_width = itos(num, out);
        out += num_width;
        break;
      }
      case 's': {
        char* print = va_arg(ap, char*);
        while (*print) {
          *(out++) = *(print++);
        }
        break;
      }
      default:
        panic("Not implemented");
      }
    }
    else {
      *(out++) = *(fmt++);
    }
  }
  *out = '\0';
  return strlen(out);
}

int sprintf(char *out, const char *fmt, ...) {
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

static int itos(int num, char* str) {
  int count = 0;
  if (str == NULL) {
      return 0;
  }
  if (num == 0) {
    *str = '0';
    return 1;
  }

  int isNegative = 0;
  if (num < 0) {
    isNegative = 1;
    num = -num;
  }
  while (num != 0) {
    int digit = num % 10;
    str[count++] = '0' + digit;
    num /= 10;
  }
  if (isNegative) {
    str[count++] = '-';
  }

  for (int i = 0; i < count / 2; i++) {
    char temp = str[i];
    str[i] = str[count - i - 1];
    str[count - i - 1] = temp;
  }
  return count;
}
#endif
