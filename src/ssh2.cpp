///////////////////////////////////////////////////////////////////////////
//libssh2
#include "libssh2_config.h"
#include <libssh2.h>
 
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/types.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include <iostream>

#include "ssh2.h"

using namespace std;
 
static int waitsocket(int socket_fd, LIBSSH2_SESSION *session) {
    struct timeval timeout;
    int rc;
    fd_set fd;
    fd_set *writefd = NULL;
    fd_set *readfd = NULL;
    int dir;
 
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
 
    FD_ZERO(&fd);
 
    FD_SET(socket_fd, &fd);
 
    /* now make sure we wait in the correct direction */ 
    dir = libssh2_session_block_directions(session);

 
    if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        readfd = &fd;
 
    if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        writefd = &fd;
 
    rc = select(socket_fd + 1, readfd, writefd, NULL, &timeout);
 
    return rc;
}


int libssh2_main(char *_hostname, int _port, char *_username, char *_password,
		 char *_cmd,  char *_out, int *_size) {
    const char *hostname = "127.0.0.1";
    const char *commandline = "uptime";
    const char *username    = "user";
    const char *password    = "password";
    
    unsigned long hostaddr;
    int sock;
    struct sockaddr_in sin;
    const char *fingerprint;
    LIBSSH2_SESSION *session;
    LIBSSH2_CHANNEL *channel;
    int rc;
    int exitcode;
    char *exitsignal=(char *)"none";
    int bytecount = 0;
    size_t len;
    LIBSSH2_KNOWNHOSTS *nh;
    int type;
    int ret = -1;
 
#ifdef WIN32
    WSADATA wsadata;
    int err;
    err = WSAStartup(MAKEWORD(2,0), &wsadata);
    if (err != 0) {
      cout << "WSAStartup failed with error: " << err << endl;
      ret = -1;
      goto failed_wsa_init;
    }
#endif
    
    hostname = _hostname;
    username = _username;
    password = _password;
    commandline = _cmd;

    /* init */
    rc = libssh2_init (0);
    if (rc != 0) {
      cout << "libssh2 initialization failed" << rc << endl;
      ret = -2;
      goto failed_libssh2_init;
    }

    /* socket */
    hostaddr = inet_addr(hostname);
    sock = socket(AF_INET, SOCK_STREAM, 0);

    /* connect */
    sin.sin_family = AF_INET;
    sin.sin_port = htons(_port);
    sin.sin_addr.s_addr = hostaddr;
    if (connect(sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in)) != 0) {
      cout << "failed to connect!" << endl;
      ret = -3;
      goto failed_connect;
    }

    /* session */
    session = libssh2_session_init();
    if (!session) {
      cout << "libssh2 session init failed" << endl;
      ret = -4;
      goto failed_session_init;
    }
 
    /* tell libssh2 we want it all done non-blocking */ 
    libssh2_session_set_blocking(session, 0);

    /* handshake 
     * ... start it up. This will trade welcome banners, exchange keys,
     * and setup crypto, compression, and MAC layers
     */ 
    while ((rc = libssh2_session_handshake(session, sock)) == LIBSSH2_ERROR_EAGAIN);
    if (rc)  {
      cout << "Failure establishing SSH session: " << rc  << endl;
      ret = -5;
      goto failed_handshake;
    }
 
    nh = libssh2_knownhost_init(session);
    if(!nh) {
        /* eeek, do cleanup here */ 
      ret =  -6;
      goto failed_knownhost_init;
    }
 
    /* read all hosts from here */ 
    libssh2_knownhost_readfile(nh, "known_hosts",LIBSSH2_KNOWNHOST_FILE_OPENSSH);
     /* store all known hosts to here */ 
    libssh2_knownhost_writefile(nh, "dumpfile", LIBSSH2_KNOWNHOST_FILE_OPENSSH);
 
    fingerprint = libssh2_session_hostkey(session, &len, &type);
    if(fingerprint) {
      struct libssh2_knownhost *host;
#if LIBSSH2_VERSION_NUM >= 0x010206
      /* introduced in 1.2.6 */ 
      int check = libssh2_knownhost_checkp(nh, hostname, 22,
					   fingerprint, len,
					   LIBSSH2_KNOWNHOST_TYPE_PLAIN|
					   LIBSSH2_KNOWNHOST_KEYENC_RAW,
					   &host);
#else
      /* 1.2.5 or older */ 
      int check = libssh2_knownhost_check(nh, hostname,
					  fingerprint, len,
					  LIBSSH2_KNOWNHOST_TYPE_PLAIN|
					  LIBSSH2_KNOWNHOST_KEYENC_RAW,
					  &host);
#endif
      //cout << "Host check: " << check << ", key: ";
      //cout << ((check <= LIBSSH2_KNOWNHOST_CHECK_MISMATCH) ? host->key:"<none>") << endl;

    /*****
     * At this point, we could verify that 'check' tells us the key is
     * fine or bail out.
     *****/ 
    } else {
        /* eeek, do cleanup here */ 
      ret = -7;
      goto failed_fingerprint;
    }
    libssh2_knownhost_free(nh);
 
    if ( strlen(password) != 0 ) {
      /* We could authenticate via password */ 
      while ((rc = libssh2_userauth_password(session, username, password)) == LIBSSH2_ERROR_EAGAIN);
      if (rc) {
	cout << "Authentication by password failed." << endl;
	ret = -8;
	goto shutdown;
      }
    } else {
      /* Or by public key */ 
      while ((rc = libssh2_userauth_publickey_fromfile(session, username,
						       "/home/user/"
						       ".ssh/id_rsa.pub",
						       "/home/user/"
						       ".ssh/id_rsa",
						       password)) == LIBSSH2_ERROR_EAGAIN);
      if (rc) {
	cout << "Authentication by public key failed" << endl;
	ret = -9;
	goto shutdown;
      }
    }
 
#if 0
    libssh2_trace(session, ~0 );
#endif
 
    /* Exec non-blocking on the remove host */ 
    while( (channel = libssh2_channel_open_session(session)) == NULL &&
	   libssh2_session_last_error(session,NULL,NULL,0) == LIBSSH2_ERROR_EAGAIN ) {
      waitsocket(sock, session);
    }
    if( channel == NULL ) {
      cout << "channel open error" << endl;
      ret = -10;
      goto shutdown;
    }
    
    while( (rc = libssh2_channel_exec(channel, commandline)) == LIBSSH2_ERROR_EAGAIN ) {
        waitsocket(sock, session);
    }
    if( rc != 0 ) {
      cout << "exec " << commandline << "error" << endl;
      return -11;
      goto shutdown;
    }

    *_size = 0;
    for( ;; ) {
      int rc;
      do  {
	char buffer[0x4000];
	rc = libssh2_channel_read( channel, buffer, sizeof(buffer) );
	if( rc > 0 )  {
	  memcpy(_out + *_size, buffer, rc);
	  *_size += rc;
	}  else {
	  if( rc != LIBSSH2_ERROR_EAGAIN ) {
	    /* no need to output this for the EAGAIN case */ 
	    //fprintf(stderr, "libssh2_channel_read returned %d\n", rc);
	  }
	}
      } while( rc > 0 );
 
      /* this is due to blocking that would occur otherwise so we loop on
	 this condition */ 
      if( rc == LIBSSH2_ERROR_EAGAIN ) {
	waitsocket(sock, session);
      } else {
	break;
      }
    }
    _out[*_size] = 0;
    
    exitcode = 127;
    while( (rc = libssh2_channel_close(channel)) == LIBSSH2_ERROR_EAGAIN ) {
        waitsocket(sock, session);
    }
    if( rc == 0 )   {
        exitcode = libssh2_channel_get_exit_status( channel );
        libssh2_channel_get_exit_signal(channel, &exitsignal, NULL, NULL, NULL, NULL, NULL);
    }

    /*
    if (exitsignal) {
      cout << "Got signal: " << exitsignal << endl;
    } else {
      cout << "EXIT: " << exitcode << "bytecount: " << bytecount << endl;
    }
    */
    
    libssh2_channel_free(channel);
    channel = NULL;

    ret = 0;
    /*   

 shutdown:
    libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
    libssh2_session_free(session);

 
#ifdef WIN32
    closesocket(sock);
#else
    close(sock);
#endif
    fprintf(stderr, "all done\n");
    libssh2_exit();
    */
    
 shutdown:
 failed_fingerprint:
 failed_knownhost_init:
    libssh2_session_disconnect(session, "Normal ShutDown, Thank you for playing");
 failed_handshake:
    libssh2_session_free(session);
 failed_session_init:
    libssh2_exit();
 failed_connect:
    closesocket(sock);
 failed_libssh2_init:
 failed_wsa_init:
    return ret;
}
