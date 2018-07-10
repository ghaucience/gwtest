#include "socket.h"

#pragma comment(lib,"ws2_32.lib")

int socket_init() {
  WORD sockVersion = MAKEWORD(2,2);
    WSADATA wsaData;
    if(WSAStartup(sockVersion, &wsaData)!=0) {
      socket_printf("init error\n");
      return -1;
    }
    return 0;
}

void socket_uninit() {
    WSACleanup();
}

SOCKET socket_server_open(int port) {
    SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(slisten == INVALID_SOCKET)  {
        socket_printf("socket error !");
        return NULL;
    }

    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.S_un.S_addr = INADDR_ANY; 
    if(bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR) {
      socket_printf("bind error !");
      return NULL;
    }

    if(listen(slisten, 5) == SOCKET_ERROR) {
      socket_printf("listen error !");
      return NULL;
    }
    
    return slisten;
}
SOCKET socket_server_accept(SOCKET sock) {
  /* no need to do */
  SOCKET sClient;
  sockaddr_in remoteAddr;
  int nAddrlen = sizeof(remoteAddr);

  fd_set readSet;
  FD_ZERO(&readSet);   
  FD_SET(sock, &readSet); // 将socketListen加入套接字集合中
      
  int result = select(0, &readSet, NULL, NULL, NULL);  
  if (result == SOCKET_ERROR)   {  
    socket_printf("select() error\n");
    return NULL;
  }
  
  if (!FD_ISSET(sock, &readSet)) {
    socket_printf("unknown error !");
    return NULL;
  }
  
  sClient = accept(sock, (SOCKADDR *)&remoteAddr, &nAddrlen);
  if(sClient == INVALID_SOCKET) {
    socket_printf("accept error !");
    return NULL;
  }    

  return sClient;
}
  
SOCKET socket_client_open(const char *svr, int port) {

  SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sclient == INVALID_SOCKET) {
    socket_printf("invalid socket !\n");
    return NULL;
  }

  sockaddr_in serAddr;
  serAddr.sin_family = AF_INET;
  serAddr.sin_port = htons(port);
  serAddr.sin_addr.S_un.S_addr = inet_addr(svr);

#if 1
  unsigned long ul = 1;
  ioctlsocket(sclient, FIONBIO, &ul);
  
  int ret = connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr));
  if (!(ret == SOCKET_ERROR && (WSAGetLastError() == WSAEWOULDBLOCK))) {
    socket_printf("connect error 1\rn");
    closesocket(sclient);
    return NULL;
  }

  fd_set writefds,expectfds;
  struct timeval tv;
 
  tv.tv_sec = 2;//设置select()超时时间为2s
  tv.tv_usec = 0;
  FD_ZERO(&writefds);
  FD_ZERO(&expectfds);
  FD_SET(sclient,&writefds);
  FD_SET(sclient,&expectfds);
  int result = select(sclient + 1, NULL, &writefds, &expectfds, &tv);
  if (result == 0) {
    socket_printf("connect timeout\n");
    closesocket(sclient);
    return NULL;
  }
  if (result < 0) {
    socket_printf("select error: < 0\n");
    closesocket(sclient);
    WSACleanup();
    return NULL;
  }

  if(FD_ISSET(sclient,&expectfds)) {
    socket_printf("connect failed!\n");	
    int error, error_len;
    error_len = sizeof(error);
    getsockopt(sclient, SOL_SOCKET, SO_ERROR, (char *)&error, &error_len);//获得错误号
    socket_printf("error is : excepts!\n");
    WSACleanup();
    closesocket(sclient);
    return NULL;
  }
  
  if(!FD_ISSET(sclient,&writefds)) {
    socket_printf("connect Error!\n");
    closesocket(sclient);
    return NULL;
  }
  
  return sclient;
#else  
  if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR) {
    socket_printf("connect error !\n");
    closesocket(sclient);
    return NULL;
  }
#endif  
  return sclient;
}

int socket_close(SOCKET sock) {
  closesocket(sock);
  return 0;
}

int socket_recv(SOCKET sock, char *buffer, int len, int ms) {
  fd_set readSet;
  FD_ZERO(&readSet);   
  FD_SET(sock, &readSet); // 将socketListen加入套接字集合中

  struct timeval tv = { ms / 1000, ms % 1000 };
      
  int result = select(0, &readSet, NULL, NULL, &tv);  
  if (result == SOCKET_ERROR)   {  
    socket_printf("select() error\n");
    return 0;
  }
  
  if (!FD_ISSET(sock, &readSet)) {
    return 0;
  }
  
  int ret = recv(sock, buffer, len, 0);        
  if(ret > 0)  {
    buffer[ret] = 0x00;
    return ret;
  }
  
  return 0;
}

int socket_send(SOCKET sock, char *buffer, int len, int ms) {
  send(sock, buffer, len, 0);
  return 0;
}
