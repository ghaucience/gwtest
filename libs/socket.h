#ifndef __SOCKET_H_
#define __SOCKET_H_


#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

 #ifdef __cplusplus
extern "C" {
#endif


  int socket_init();
  void socket_uninit();
  SOCKET socket_server_open(int port);
  SOCKET socket_server_accept(SOCKET sock);
  SOCKET socket_client_open(const char *svr, int port);

  int socket_close(SOCKET sock);
  int socket_recv(SOCKET sock, char *buffer, int len, int ms);
  int socket_send(SOCKET sock, char *buffer, int len, int ms);

  
  //#define socket_printf(...) printf(##__VA_ARGS__)
#define socket_printf(...) MessageBox(NULL, L##__VA_ARGS__, L"Error", MB_OK)


#ifdef __cplusplus
}
#endif
#endif
