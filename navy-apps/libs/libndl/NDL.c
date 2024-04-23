#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <assert.h>

int _gettimeofday(struct timeval *tv, struct timezone *tz);
int _open(const char *path, int flags, mode_t mode);
int _write(int fd, void *buf, size_t count);
int _read(int fd, void *buf, size_t count);
int _close(int fd);
off_t _lseek(int fd, off_t offset, int whence);

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int system_w = 0, system_h = 0;

uint32_t NDL_GetTicks() {
  struct timeval now;
  _gettimeofday(&now, NULL);
  return now.tv_sec * 10000000 + now.tv_usec;
}
int NDL_PollEvent(char *buf, int len) {
  int valid = _read(evtdev, buf, len);
  return valid ? 1 : 0;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
  else {
    assert(*w <= system_w);
    assert(*h <= system_h);
    screen_w = *w ? *w : system_w;
    screen_h = *h ? *h : system_h;
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  assert(x + w <= screen_w);
  assert(y + h <= screen_h);
  uint32_t *pixels_ptr = pixels;
  const int w_padding = (system_w - screen_w) / 2;
  const int h_padding = (system_h - screen_h) / 2;
  for (int i = 0; i < h; i++) {
    _lseek(fbdev, x+w_padding+(y+h_padding+i)*system_w, SEEK_SET);
    _write(fbdev, pixels_ptr, w);
    pixels_ptr += w;
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  else {
    evtdev = _open("/dev/events", 0, 0);
    fbdev = _open("/dev/fb", 0, 0);
    int fd = _open("/proc/disinfo", 0, 0);
    char buf[64];
    _read(fd, buf, 64);
    sscanf(buf, "WIDTH : %d\nHEIGHT : %d", &system_w, &system_h);
    _close(fd);
  }
  return 0;
}

void NDL_Quit() {
  _close(evtdev);
}
