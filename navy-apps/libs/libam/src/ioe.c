#include <am.h>
bool ioe_init() {
  return true;
}

void ioe_read(int reg, void *buf) {
  int fd = open("/dev/ioe", 0, 0);
  lseek(fd, reg, SEEK_SET);
  read(fd, buf, 0);
  close(fd);
}
void ioe_write(int reg, void *buf) {
  int fd = open("/dev/ioe", 0, 0);
  lseek(fd, reg, SEEK_SET);
  write(fd, buf, 0);
  close(fd);
}