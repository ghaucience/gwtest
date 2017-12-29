#ifndef __SHEET_H_
#define __SHEET_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libxl/libxl.h"

 #ifdef __cplusplus
extern "C" {
#endif




  BookHandle sheet_open(const wchar_t *path, const wchar_t *sheet, const wchar_t *cols[], int colcnt);
  void sheet_append(BookHandle bh, const wchar_t *cols[], int colcnt);
  void sheet_append_header(BookHandle bh, const wchar_t *cols[], int colcnt);
  void sheet_save(BookHandle bh, const wchar_t *path, const wchar_t *sheet);
  void sheet_close(BookHandle bh);
  int sheet_get_total_colnum(BookHandle bh);

#define sheet_printf(...) printf(##__VA_ARGS__)

  


















#ifdef __cplusplus
}
#endif

#endif
