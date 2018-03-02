#ifndef __LWEB_H_
#define __LWEB_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus 
extern "C" {
#endif


#ifdef ARCH_ARM
char *rindex(const char *str, int c);
#endif


#include "httpd.h"

#ifdef __cplusplus
}
#endif


#endif
