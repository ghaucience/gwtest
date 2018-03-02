#include "lweb.h"


#ifdef ARCH_ARM
char *rindex(const char *str, int c) {

	if (str == NULL || c <= 0 || c >= 256) {
		return NULL;
	}

	

	char *p = (char *)str;
	char *ret = NULL;
	while (*p != '\0') {
		if (((*p)&0xff) == (c&0xff)) {
			ret = p;
		}
		p++;
	}

	return ret;
}
#endif
