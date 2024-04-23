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

typedef void (*CWriter)(char);
static char **dest = NULL;
static char buffer[128];

/* helper function */
static void set_dest(char **out);
static void write_ch(char ch);
static void reverse(char *str, int l, int j);
static int itos(int64_t num, char *str);
static int utos(uint64_t num, char *str);
static int utohs(uint64_t num, char *str, int upper_case);
static int utoos(uint64_t num, char *str);
static int keep_width_writer(CWriter writer, char *str, int write_len, FormatOptions *format, char fillch);
static int num_writer(CWriter writer, const char *num, int num_len, FormatOptions *format);

/* parser funciton */
static int flag_parser(FormatOptions *format, const char **fmt, va_list *args);
static int width_parser(FormatOptions *format, const char **fmt, va_list *args);
static int precision_parser(FormatOptions *format, const char **fmt, va_list *args);
static int length_parser(FormatOptions *format, const char **fmt, va_list *args);
static int conversion_parser(FormatOptions *format, const char **fmt, va_list *args);

/* format funciton */
static int format_char(CWriter writer, FormatOptions *format, va_list *args);
static int format_integer(CWriter writer, FormatOptions *format, va_list *args);
static int format_float(CWriter writer, FormatOptions *format, va_list *args);
static int format_pointer(CWriter writer, FormatOptions *format, va_list *args);

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
      format->precision = (prec > 0) ? prec : format->precision;
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

static int format_char(CWriter writer, FormatOptions *format, va_list *args) {
  int count = 0;
  if (format->zero || format->prefix) { return -1; }
  switch (format->conversion) {
  case 'c': {
    unsigned char ch = va_arg(*args, int);
    buffer[0] = ch;
    count += keep_width_writer(writer, buffer, 1, format, ' ');
    break;
  }
  case 's': {
    char *str = va_arg(*args, char *);
    size_t length = strlen(str);
    if (format->precision < 0) {
      count += keep_width_writer(writer, str, length, format, ' ');
    }
    else {
      int write_len = (length > format->precision) ? format->precision : length;
      count += keep_width_writer(writer, str, write_len, format, ' ');
    }
    break;
  }
  default: return -1;
  }
  return count;
}

static int format_integer(CWriter writer, FormatOptions *format, va_list *args) {
  int num_len = 0;
  switch (format->conversion) {
  case 'd': case 'i': {
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
    if (format->prefix) { return -1; }
    if (format->precision == 0 && num == 0) { return 0; }
    num_len = itos(num, buffer);
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
    if (format->space || format->sign) { return -1; }
    switch (format->conversion) {
    case 'u':
      if (format->prefix) { return -1; }
      if (format->precision == 0 && num == 0) { return 0; }
      num_len = utos(num, buffer);
      break;
    case 'o':
      num_len = utoos(num, buffer);
      if (format->precision == 0 && num == 0 && !format->prefix) { return 0; }
      break;
    case 'x': case 'X':
      num_len = utohs(num, buffer, format->conversion == 'x' ? 0 : 1);
      if (format->precision == 0 && num == 0) { return 0; }
      if (num == 0) { format->prefix = 0; }
      break;
    }
    break;
  }
  default: return -1;
  }
  return num_writer(writer, buffer, num_len, format);
}

static int format_float(CWriter writer, FormatOptions *format, va_list *args) {
  panic("Not implemented");
}

static int format_pointer(CWriter writer, FormatOptions *format, va_list *args) {
  void *ptr = va_arg(*args, void*);
  int count = utohs((size_t)ptr, buffer, 0);
  writer('0');
  writer('x');
  for (int i = 0; i < count; i++) {
    writer(buffer[i]);
  }
  return count+2;
}

static int vprintf(CWriter writer, const char *fmt, va_list *args) {
  int count = 0;
  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      int success;
      FormatOptions format = format_parser(&fmt, &success, args);
      if (!success) { return -1; }
      else {
        int width;
        switch (format.conversion) {
        case '%':
          writer('%');
          width = 1;
          break;
        case 'd': case 'i': case 'o': case 'u':
        case 'x': case 'X':
          width = format_integer(writer, &format, args);
          break;
        case 'c': case 's':
          width = format_char(writer, &format, args);
          break;
        case 'f': case 'F': case 'e': case 'E':
        case 'a': case 'A': case 'g': case 'G':
          width = format_float(writer, &format, args);
          break;
        case 'p':
          width = format_pointer(writer, &format, args);
          break;
        case 'n':
          panic("TODO");
          break;
        default:
          return -count;
        }
        if (width < 0) { return -count; }
        count += width;
      }
    }
    else {
      writer(*(fmt)++);
      count++;
    }
  }
  writer('\0');
  return count;
}

int printf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int count = vprintf(putch, fmt, &args);
  va_end(args);
  return count;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  va_list args;
  va_copy(args, ap);
  set_dest(&out);
  int count = vprintf(write_ch, fmt, &args);
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
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

/* helper funciont */
static void set_dest(char **out) {
  dest = out;
}

static void write_ch(char ch) {
  if (dest) { *((*dest)++) = ch; }
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

static int utohs(uint64_t num, char *str, int upper_case) {
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
      str[count++] = (temp - 10) + (upper_case ? 'A' : 'a');
    }
    num = num / 16;
  }
  reverse(str, 0, count-1);
  return count;
}

 static int keep_width_writer(CWriter writer, char *str, int write_len, FormatOptions *format, char fillch) {
  int count = 0;
  if (write_len > format->width) {
    count += write_len;
    while (write_len--) {
      writer(*(str++));
    }
  }
  else {
    if (format->justify) {
      count += write_len;
      while (write_len--) {
        writer(*(str++));
      }
      for (; count < format->width; count++) {
        writer(fillch);
      }
    }
    else {
      for (; count < format->width - write_len; count++) {
        writer(fillch);
      }
      count += write_len;
      while(*str) {
        writer(*(str++));
      }
    }
  }
  return count;
}

static int num_writer(CWriter writer, const char *num, int num_len, FormatOptions *format) {
  char sign = format->sign ? '+' : ' ';
  int sign_len = (num > 0 && (format->space || format->sign)) ? 1 : 0;
  if (*num == '-') {
    sign = '-';
    sign_len = 1;
    num++;
    num_len -= 1;
  }

  int precision_len = (format->precision > num_len) ? format->precision - num_len : 0;
  int space_len = 0;

  const char *prefix = NULL;
  int prefix_len = 0;
  switch (format->prefix) {
  case 1:
    precision_len = (precision_len < 1 && *num != '0') ? 1 : precision_len;
    break;
  case 2:
    prefix = "0x";
    prefix_len = 2;
    break;
  case 3:
    prefix = "0X";
    prefix_len = 2;
    break;
  default: break;
  }

  if (format->zero) {
    if (num_len + precision_len + sign_len + prefix_len < format->width) {
      precision_len = format->width - num_len - sign_len - prefix_len;
    }
  }
  else {
    if (num_len + precision_len + sign_len + prefix_len < format->width) {
      space_len = format->width - num_len - precision_len - sign_len - prefix_len;
    }
  }
  /* fill space according to format->justify */
  if (!format->justify) {
    for (int i = 0; i < space_len; i++) { writer(' '); }
  }
  /* write sign */
  if (sign_len) { writer(sign); }
  /* write prefix */
  for (int i = 0; i < prefix_len; i++) {
    writer(prefix[i]);
  }
  /* write precision-zero */
  for (int i = 0; i < precision_len; i++) {
    writer('0');
  }
  /* write num */
  for (int i = 0; i < num_len; i++) {
    writer(num[i]);
  }
  /* fill space according to format->justify */
  if (format->justify) {
    for (int i = 0; i < space_len; i++) { writer(' '); }
  }
  return sign_len + prefix_len + precision_len + num_len + space_len;
}
#endif
