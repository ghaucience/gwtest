#ifndef __SHEET_H_
#define __SHEET_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libxl.h"

 #ifdef __cplusplus
extern "C" {
#endif




  BookHandle sheet_open(const char *path, const char *sheet, const char *cols[], int colcnt);
  void sheet_append(BookHandle bh, const char *cols[], int colcnt);
  void sheet_append_header(BookHandle bh, const char *cols[], int colcnt);
  void sheet_save(BookHandle bh, const char *path, const char *sheet);
  void sheet_close(BookHandle bh);
  int sheet_get_total_colnum(BookHandle bh);

#define sheet_printf(...) printf(##__VA_ARGS__)

  


















#ifdef __cplusplus
}
#endif

#endif
