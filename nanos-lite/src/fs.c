#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
void init_ramdisk();

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t fs_ioe_read(void *buf, size_t offset, size_t len) { /* yield(); */ ioe_read(offset, buf); return len; }
size_t fs_ioe_write(const void *buf, size_t offset, size_t len) { /* yield(); */ ioe_write(offset, (void*)buf); return len;}
typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENTS, FD_DISINFO, FD_FB, FB_IO};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [FD_EVENTS] = {"/dev/events", 0, 0, events_read, invalid_write},
  [FD_DISINFO] = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
  [FD_FB] = {"/dev/fb", 0, 0, invalid_read, fb_write},
  [FB_IO] = {"/dev/ioe", 0, 0, fs_ioe_read, fs_ioe_write},
#include "files.h"
};
#define FILENUM (LENGTH(file_table) - 1)
static size_t open_offset[FILENUM];

char *fd_to_filename(int fd) {
  assert(fd >= 0 && fd < FILENUM);
  return file_table[fd].name;
}

int fs_open(const char *pathname, int flags, int mode) {
  for (int i = 0; i < FILENUM; i++) {
    if (!strcmp(pathname, file_table[i].name)) {
      open_offset[i] = 0;
      return i;
    }
  }
  // panic("File %s doesn't exist", pathname);
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len) {
  assert(fd >= 0 && fd < FILENUM);
  ReadFn rf = ramdisk_read;
  if (file_table[fd].read) { rf = file_table[fd].read; }
  else if (open_offset[fd] + len > file_table[fd].size) {
    len = file_table[fd].size - open_offset[fd];
  }
  size_t rcount = rf(buf, file_table[fd].disk_offset + open_offset[fd], len);
  open_offset[fd]+= rcount;
  return rcount;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  assert(fd >= 0 && fd < FILENUM);
  WriteFn wf = ramdisk_write;
  if (file_table[fd].write) { wf = file_table[fd].write; }
  else if (open_offset[fd] + len > file_table[fd].size) {
    len = file_table[fd].size - open_offset[fd];
  }
  size_t wcount = wf(buf, file_table[fd].disk_offset + open_offset[fd], len);
  open_offset[fd]+= wcount;
  return wcount;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  assert(fd >= 0 && fd < FILENUM);
  switch (whence) {
    case SEEK_SET: open_offset[fd] = offset; break;
    case SEEK_CUR: open_offset[fd] += offset; break;
    case SEEK_END: open_offset[fd] = file_table[fd].size + offset; break;
    default: return -1;
  }
  return open_offset[fd];
}

int fs_close(int fd) {
  return 0;
}

void init_fs() {
  // TODO: initialize the size of /dev/fb
  AM_GPU_CONFIG_T gpu_config = io_read(AM_GPU_CONFIG);
  file_table[FD_FB].size = gpu_config.vmemsz;
}
