#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <sys/types.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
enum {
  FLAG = 0, WIDTH = 1, PRECISION = 2, LENGTH = 3, CONVERSION = 4
};
enum {
  SHORTSHORT,  SHORT,  LONG,  LONGLONG,  SIZE,  LONGDOUBLE,
  POINTERSUB,  MAXIMUMINT,  DEFAULT,
};

typedef struct {
  unsigned int justify : 1;
  unsigned int sign : 1;
  unsigned int space : 1;
  unsigned int prefix : 2;
  unsigned int zero : 1;
  unsigned int length : 4;
  int width;
  int precision;
  int conversion;
} FormatOptions;

/* helper function */
static void reverse(char *str, int l, int j);
static int itos(int64_t num, char *str);
static int utos(uint64_t num, char *str);
static int utohs(uint64_t num, char *str);
static int utoos(uint64_t num, char *str);
static void insert_ch(char *str, int length, int count, char fillch);
static int keep_precision(char *str, int length, FormatOptions *format);
static int keep_width(char *str, int length, FormatOptions *format, char fillch);

/* parser funciton */
static int flag_parser(FormatOptions *format, const char **fmt, va_list *args);
static int width_parser(FormatOptions *format, const char **fmt, va_list *args);
static int precision_parser(FormatOptions *format, const char **fmt, va_list *args);
static int length_parser(FormatOptions *format, const char **fmt, va_list *args);
static int conversion_parser(FormatOptions *format, const char **fmt, va_list *args);

/* format funciton */
static int format_char(char *out, FormatOptions *format, va_list *args);
static int format_integer(char *out, FormatOptions *format, va_list *args);
static int format_float(char *out, FormatOptions *format, va_list *args);
static int format_pointer(char *out, FormatOptions *format, va_list *args);

static struct {
  int stage;
  int (*handle)(FormatOptions *format, const char **fmt, va_list *args);
} handle_table[] = {
  { FLAG, flag_parser },
  { WIDTH, width_parser },
  { PRECISION, precision_parser },
  { LENGTH, length_parser },
  { CONVERSION, conversion_parser },
};
#define TABLELEN (sizeof handle_table / sizeof handle_table[0])

static int flag_parser(FormatOptions *format, const char **fmt, va_list *args) {
  int doing = 1;
  while (**fmt && doing) {
    switch (**fmt) {
    case '-': format->justify = 1;  break;
    case '+': format->sign = 1;     break;
    case ' ': format->space = 1;    break;
    case '#': format->prefix = 1;   break;
    case '0': format->zero = 1;     break;
    default: doing = 0; (*fmt)--;   break;
    }
    (*fmt)++;
  }
  /* handle conflict */
  if (format->sign) { format->space = 0; }
  if (format->justify) { format->zero = 0; }
  return **fmt;
}

static int width_parser(FormatOptions *format, const char **fmt, va_list *args) {
  if (**fmt == '*') {
    format->width = va_arg(*args, int);
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

static int precision_parser(FormatOptions *format, const char **fmt, va_list *args) {
  if (**fmt == '.') {
    (*fmt)++;
    if (**fmt == '*') {
      int prec = va_arg(*args, int);
      format->precision = prec > 0 ? prec : format->precision;
      (*fmt)++;
    }
    else if (**fmt >= '0' && **fmt <= '9') {
      format->precision = 0;
      while (**fmt >= '0' && **fmt <= '9') {
        format->precision = format->precision * 10 + **fmt - '0';
        (*fmt)++;
      }
    }
    else {
      format->precision = 0;
    }
    /* handle conflict */
    format->zero = 0;
  }
  return **fmt;
}

static int length_parser(FormatOptions *format, const char **fmt, va_list *args) {
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
  case 'j':
    format->length = MAXIMUMINT; (*fmt)++;    break;
  case 'z':
    format->length = SIZE; (*fmt)++;          break;
  case 't':
    format->length = POINTERSUB; (*fmt)++;    break;
  case 'L':
    format->length = LONGDOUBLE; (*fmt)++;    break;
  default:
    break;
  }
  return **fmt;
}

static int conversion_parser(FormatOptions *format, const char **fmt, va_list *args)
{
  const char valid[] = "%csdioxXufFeEaAgGnp";
  int invalid_flag = 1;
  for (const char *p = valid; *p && invalid_flag; p++) {
    if (**fmt == *p) { invalid_flag = 0; }
  }
  if (invalid_flag) { return 0; }
  format->conversion = **fmt;
  /* handle default precision */
  if (format->precision == -1) {
    switch (**fmt) {
    case 'd': case 'i': case 'o': case 'x':
    case 'X': case 'u': case 'p':
      format->precision = 1;
      break;
    case 'f': case 'F': case 'e': case 'E':
    case 'g': case 'G':
      format->precision = 6;
      break;
    case 'a': case 'A':
      format->precision = 15;
      break;
    }
  }
  /* handle prefix */
  if (format->prefix == 1) {
    switch (**fmt) {
    case 'x': format->prefix = 2; break;
    case 'X': format->prefix = 3; break;
    }
  }
  (*fmt)++;
  return 1;
}

static FormatOptions format_parser(const char **fmt, int *success, va_list *args) {
  FormatOptions format = {
      .justify = 0,
      .sign = 0,
      .space = 0,
      .prefix = 0,
      .zero = 0,
      .width = 0,
      .precision = -1, /* -1 means default */
      .length = DEFAULT,
      .conversion = 0,
  };
  *success = 1;
  for (int stage = 0; stage < TABLELEN && *success; stage++) {
    *success = handle_table[stage].handle(&format, fmt, args);
  }
  return format;
}

static int format_char(char *out, FormatOptions *format, va_list *args) {
  int count = 0;
  if (format->zero || format->prefix) { return -1; }
  switch (format->conversion) {
  case 'c': {
    unsigned char ch = va_arg(*args, int);
    if (format->justify) {
      out[count++] = ch;
      for (int i = 0; i < format->width - 1; i++) {
        out[count++] = ' ';
      }
    }
    else {
      for (int i = 0; i < format->width - 1; i++) {
        out[count++] = ' ';
      }
      out[count++] = ch;
    }
    break;
  }
  case 's': {
    char *str = va_arg(*args, char *);
    while (*str) {
      *(out++) = *(str++);
      count++;
    }
    break;
  }
  default: return -1;
  }
  count += keep_width(out, count, format, ' ');
  return count;
}

static int format_integer(char *out, FormatOptions *format, va_list *args) {
  int count = 0;
  switch (format->conversion) {
  case 'd': case 'i': {
    if (format->prefix) { return -1; }
    int64_t num = 0;
    switch (format->length) {
    case SHORTSHORT: num= (signed char)va_arg(*args, int);  break;
    case SHORT: num = (short)va_arg(*args, int);            break;
    case DEFAULT: num = va_arg(*args, int);                 break;
    case LONG: num = va_arg(*args, long);                   break;
    case LONGLONG: num = va_arg(*args, long long);          break;
    case MAXIMUMINT: num = va_arg(*args, intmax_t);         break;
    case SIZE: num = va_arg(*args, ssize_t);                break;
    case POINTERSUB: num = va_arg(*args, ptrdiff_t);        break;
    default: return -1;
    }
    if (format->precision == 0 && num == 0) { return 0; }
    if (num > 0) {
      if (format->sign) {
        format->width--;
      }
      else if (format->space) {
        format->width--;
      }
    }
    count += itos(num, out);
    count += keep_precision(out, count, format);
    count += keep_width(out, count, format, format->zero ? '0' : ' ');
    if (num > 0) {
      if (format->sign) {
        insert_ch(out, count++, 1, '+');
      }
      else if (format->space) {
        insert_ch(out, count++, 1, ' ');
      }
    }
    break;
  }
  case 'o': case 'x': case 'X': case 'u': {
    uint64_t num = 0;
    switch (format->length) {
    case SHORTSHORT: num= (unsigned char)va_arg(*args, int);  break;
    case SHORT: num = (unsigned short)va_arg(*args, int);     break;
    case DEFAULT: num = va_arg(*args, unsigned int);          break;
    case LONG: num = va_arg(*args, unsigned long);            break;
    case LONGLONG: num = va_arg(*args, unsigned long long);   break;
    case MAXIMUMINT: num = va_arg(*args, uintmax_t);          break;
    case SIZE: num = va_arg(*args, size_t);                   break;
    case POINTERSUB: num = va_arg(*args, size_t);             break;
    default: return -1;
    }
    if (format->precision == 0 && num == 0) { return 0; }
    switch (format->conversion) {
    case 'u': {
      if (format->prefix) { return -1; }
      count += utos(num, out);
      count += keep_precision(out, count, format);
      count += keep_width(out, count, format, format->zero ? '0' : ' ');
      break;
    }
    case 'o': {
      if (format->prefix) {
        format->width--;
      }
      count += utoos(num, out);
      count += keep_precision(out, count, format);
      count += keep_width(out, count, format, format->zero ? '0' : ' ');
      if (format->prefix) {
        insert_ch(out, count++, 1, '0');
      }
      break;
    }
    case 'x': case 'X': {
      if (format->prefix) {
        format->width -= 2;
      }
      count += utohs(num, out);
      count += keep_precision(out, count, format);
      count += keep_width(out, count, format, format->zero ? '0' : ' ');
      if (format->prefix) {
        insert_ch(out, count++, 1, '0');
        insert_ch(out, count++, 1, format->prefix == 2 ? 'x' : 'X');
      }
      break;
    }
    }
    break;
  }
  default: return -1;
  }
  return count;
}

static int format_float(char *out, FormatOptions *format, va_list *args) {
  panic("Not implemented");
}

static int format_pointer(char *out, FormatOptions *format, va_list *args) {
  void *ptr = va_arg(*args, void*);
  return utohs((size_t)ptr, out);
}

int printf(const char *fmt, ...) {
  char buf[4096];
  va_list args;
  va_start(args, fmt);
  int ret = vsprintf(buf, fmt, args);
  va_end(args);
  putstr(buf);
  return ret;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  int count = 0;
  va_list args;
  va_copy(args, ap);
  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      int success;
      FormatOptions format = format_parser(&fmt, &success, &args);
      if (!success) { return -1; }
      else {
        int width;
        switch (format.conversion) {
        case '%':
          *out = '%';
          width = 1;
          break;
        case 'd': case 'i': case 'o': case 'u':
        case 'x': case 'X':
          width = format_integer(out, &format, &args);
          break;
        case 'c': case 's':
          width = format_char(out, &format, &args);
          break;
        case 'f': case 'F': case 'e': case 'E':
        case 'a': case 'A': case 'g': case 'G':
          width = format_float(out, &format, &args);
          break;
        case 'p':
          width = format_pointer(out, &format, &args);
          break;
        case 'n':
          width = itos(count, out);
          break;
        default:
          return -count;
        }
        if (width < 0) { return -count; }
        out += width;
        count += width;
      }
    }
    else {
      *(out++) = *(fmt++);
      count++;
    }
  }
  *out = '\0';
  va_end(args);
  return count;
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

static void reverse(char *str, int l, int j) {
  while (l < j) {
    char temp = str[l];
    str[l] = str[j];
    str[j] = temp;
    l++; j--;
  }
}
static int itos(int64_t num, char *str) {
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
  reverse(str, 0, count-1);
  return count;
}

static int utos(uint64_t num, char *str) {
  int count = 0;
  if (num == 0) {
    str[count++] = '0';
    return count;
  }

  while (num != 0) {
    int temp = num % 10;
    str[count++] = temp + '0';
    num = num / 10;
  }
  reverse(str, 0, count-1);
  return count;
}

static int utoos(uint64_t num, char *str) {
  int count = 0;
  if (num == 0) {
    str[count++] = '0';
    return count;
  }

  while (num != 0) {
    int temp = num % 8;
    str[count++] = temp + '0';
    num = num / 8;
  }
  reverse(str, 0, count-1);
  return count;
}

static int utohs(uint64_t num, char *str) {
  int count = 0;
  if (num == 0) {
    str[count++] = '0';
    return count;
  }

  while (num != 0) {
    int temp = num % 16;
    if (temp < 10) {
      str[count++] = temp + '0';
    } else {
      str[count++] = (temp - 10) + 'A';
    }
    num = num / 16;
  }
  reverse(str, 0, count-1);
  return count;
}

static void insert_ch(char *str, int length, int count, char fillch) {
  for (int i = length - 1; i >= 0; i--) {
    str[count + i] = str[i];
  }
  for (int i = 0; i < count; i++) {
    str[i] = fillch;
  }
}

static int keep_width(char *str, int length, FormatOptions *format, char fillch) {
  int count = format->width - length;
  if (count <= 0) { return 0; }
  if (format->justify) {
    for (int i = 0; i < count; i++) {
      str[length + i] = fillch;
    }
  }
  else {
    insert_ch(str, length, count, fillch);
  }
  return count;
}

static int keep_precision(char *str, int length, FormatOptions *format) {
  int count = format->precision - length;
  if (count <= 0) { return 0; }
  insert_ch(str, length, count, '0');
  return count;
}
#endif
