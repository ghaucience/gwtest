#ifndef __SSH_2_H_
#define __SSH_2_H_

#ifdef __cplusplus
extern "C" {
#endif

int libssh2_main(char *_hostname, int _port, char *_username, char *_password,
		 char *_cmd,  char *_out, int *_size);  
  

#ifdef __cplusplus
}
#endif

#endif
