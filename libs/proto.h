#ifndef __PROTO_H_
#define __PROTO_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 #ifdef __cplusplus
extern "C" {
#endif

/* FE CMD1 CMD2 LEN DATA CHK */
  
enum {
  S_WAIT_HEAD,
  S_WAIT_CMD1,
  S_WAIT_CMD2,
  S_WAIT_LEN1,
  S_WAIT_LEN2,
  S_WAIT_DATA,
  S_WAIT_CHECK,
};
  
#define MAX_FRAME_LEN 1024

#ifndef u8
#define u8 unsigned char
#endif

#define proto_printf printf

  /* return 1, get a complete frame */
  int proto_frame_get(u8 * _fm);

  /* get a byte from uart or uart buffer 
   * return 0, no data
   * return 1, has data and b is the byte received 
   */
  int proto_uart_get(u8 *b);

  /* construct a frame to buffer 
   * return the frame len 
   */
  int proto_frame_set(u8 *_fm, u8 cmd1, u8 cm2, int len, u8 *data);


  /* view a buffer */
  void proto_buffer_view(u8 *_fm, int len);

  /* util */
  u8 proto_frame_get_cmd1(u8 *_fm);
  u8 proto_frame_get_cmd2(u8 *_fm);
  int proto_frame_get_len(u8 *_fm);
  u8* proto_frame_get_data(u8 *_fm);
  int proto_frame_get_cmd(u8 *_fm);
#ifdef __cplusplus
}
#endif
#endif
