#include "stdafx.h"
#include "proto.h"

static u8 frame[MAX_FRAME_LEN - 5];
static u8 sts = 0;
static int flen = 0;
static int len = 0;
static int rlen = 0;
static u8 sum = 0;

static void state_reset() {
  sts = S_WAIT_HEAD;
  len = rlen = flen = sum = 0;
}

static void my_memcpy(u8 *dst, u8 *src, u8 len) {
#if 0
  u8 i = 0;
  for (i = 0; i < len; i++) {
    dst[i] = src[i];
  }
#else
  memcpy(dst, src, len);
#endif
}

int proto_frame_get(u8 *_fm) {
  u8 b;
  while (proto_uart_get(&b)) {
    switch (sts) {
    case S_WAIT_HEAD:
      if (b == 0xfe) {
	sts = S_WAIT_CMD1;
	frame[flen++] = b;
      }
      break;

    case S_WAIT_CMD1:
      frame[flen++] = b;
      sum ^= b;
      sts = S_WAIT_CMD2;
      break;

    case S_WAIT_CMD2:
      frame[flen++] = b;
      sum ^= b;
      sts = S_WAIT_LEN1;
      break;

    case S_WAIT_LEN1:
      frame[flen++] = b;
      sum ^= b;
      sts = S_WAIT_LEN2;
      break;
      
    case S_WAIT_LEN2:
      frame[flen++] = b;
      sum ^= b;
      len = proto_frame_get_len(frame);
      if (len > MAX_FRAME_LEN) {
	state_reset();
      } else {
	if (len > 0) {
	  sts = S_WAIT_DATA;
	} else {
	  sts = S_WAIT_CHECK;
	}
      }
      break;
    case S_WAIT_DATA:
      frame[flen++] = b;
      sum ^= b;
      rlen++;
      if (rlen == len) {
	sts = S_WAIT_CHECK;
      }
      break;
    case S_WAIT_CHECK:
      frame[flen++] = b;
      if (sum == b) {
	my_memcpy(_fm, frame, flen);
	state_reset();
	return 1;
      }
      state_reset();
      break;
    default:
      state_reset();
      break;
    }
  }
  return 0;
}

void proto_buffer_view(u8 *_fm, int len) {
  int i = 0;
  for (i = 0; i < len; i++) {
    proto_printf("[%02X] ",_fm[i]&0xff);
    if ( (i + 1) % 20 == 0) {
      proto_printf("\n");
    }
  }
  proto_printf("\n");
}

int proto_frame_set(u8 *_fm, u8 cmd1, u8 cmd2, int _len, u8 *data) {
  u8 sum = 0;
  u8 x = 0;
  int i = 0;

  len = 0;
  
  x = 0xFE;
  //uart_send(&x, 1);
  _fm[len++] = x;
  
  x = cmd1;
  //uart_send(&x, 1);
  _fm[len++] = x;
  sum ^= x;
  
  
  x = cmd2;
  //uart_send(&x, 1);
  _fm[len++] = x;
  sum ^= x;
  
  x = (_len>>8)&0xff;
  //uart_send(&x, 1);
  _fm[len++] = x;
  sum ^= x;

  x = (_len>>0)&0xff;
  //uart_send(&x, 1);
  _fm[len++] = x;
  sum ^= x;
  
  for (i = 0; i < _len; i++) {
    x = data[i];
    //uart_send(&x, 1);
    _fm[len++] = x;
    sum ^= x;
  }
  
   x = sum;
   //uart_send(&x, 1);
   _fm[len++] = x;
   sum ^= x;

   proto_buffer_view(_fm, len);

   return len;
}

int proto_uart_get(u8 *b) {
  /* get b byte from uart serial buffer */
  /* TODO */
#if 0
  return socket_buffer_get_byte(b);
#else
  return 0;
#endif
}

u8 proto_frame_get_cmd1(u8 *_fm) {
	return _fm[1] & 0xff;
}

u8 proto_frame_get_cmd2(u8 *_fm) {
  return _fm[2]&0xff;
}

 short proto_frame_get_cmd(u8 *_fm) {
   return ((_fm[1]&0xff)<<8) | (_fm[2]&0xfff);
 }

int proto_frame_get_len(u8 *_fm) {
  return (_fm[3]&0xff << 8) | (_fm[4]&0xff);
}
u8* proto_frame_get_data(u8 *_fm) {
  return &_fm[5];
}
