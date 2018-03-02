#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serial.h"

#include "windows.h"
#include "PCOMM.H"

static int serial_dev2com(const char *dev) {
  if (!strncmp(dev, "COM", 3) == 0) {
    return -1;
  }
  char *s = (char *)dev+3;
  char *e = s;
  while (*e != '\0' && isdigit(*e)) ++e;
  int l = e - s;
  if (l <= 0 || l > 2) {
    return -2;
  }
  char com = 0;
  char buf[16] = {0};
  memcpy(buf, s, l);
  buf[l] = 0;
  if (sscanf(buf, "%d", &com) != 1) {
    return -3;
  }
  if (com < 0) {
    return -4;
  }
  return com;
}
    
int serial_init(const char *dev, int buad) {
  int ret;

  int comNumb = serial_dev2com(dev);
  if (comNumb < 0) {
    return -1;
  }

  ret = sio_open (comNumb);
  if (ret != SIO_OK) {
    return -2;
  }

  if (buad == B9600) {
    sio_ioctl (comNumb, B9600, P_NONE | BIT_8 | STOP_1 );
  } else if (buad == B115200) {
    sio_ioctl (comNumb, B115200, P_NONE | BIT_8 | STOP_1 );
  }

  return comNumb;
}
int serial_close(int fd) {
  int ret = 0;

  ret = sio_close(fd);
  
  return ret;
}
int serial_read(int fd, char *buf, int size, int s, int m) {
  int ret = 0;
  
  ret = sio_SetReadTimeouts(fd, s, m);
  
  ret = sio_read(fd, buf, size);
	
  return ret;
}
int serial_write(int fd, char *buf, int size, int s, int m) {
  int ret = 0;
  
  ret = sio_write (fd, buf, size);
	
  if (ret != size) {
    return -1;
  }

  return ret;
}

int serial_flush(int fd) {
  return 0;
}




