#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  yield();
  for (size_t i = 0; i < len; i++) {
    putch(((char*)buf)[i]);
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  yield();
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if(ev.keycode == AM_KEY_NONE) { return 0; }
  char ev_str[64];
  int ev_len = sprintf(ev_str, "%s %s\n", ev.keydown ? "kd" : "ku", keyname[ev.keycode]);
  strncpy(buf, ev_str, len);
  return (ev_len < len) ? ev_len : len;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T gpu_config = io_read(AM_GPU_CONFIG);
  char gpuinfo_str[64];
  int gpuinfo_len = sprintf(gpuinfo_str, "WIDTH:%d\nHEIGHT:%d\n", gpu_config.width, gpu_config.height);
  strncpy(buf, gpuinfo_str, len);
  return (gpuinfo_len < len) ? gpuinfo_len : len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  yield();
  AM_GPU_CONFIG_T gpu_config = io_read(AM_GPU_CONFIG);
  offset /= 4;
  int x = offset % gpu_config.width;
  int y = offset / gpu_config.width;
  io_write(AM_GPU_FBDRAW, x, y, (void*)buf, len / 4, 1, true);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
