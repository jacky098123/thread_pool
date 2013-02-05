#ifndef __COMMU_SOCKET_H__
#define __COMMU_SOCKET_H__

extern const char*						http_head_terminal;

enum HTTP_METHOD {
    METHOD_NONE,
    METHOD_GET,
    METHOD_POST,
    METHOD_OTHER
};

int SocketServer(int port);

int Accept(int serversock);

int Socket(const char* server, int port);

int Send(int sock, const char *buffer, int total, int timeout);

int Receive(int sock, char *buffer, int size, int timeout);

int HttpReceive(int sock, char **buffer, int *buffer_size, int timeout);

int HttpSend(int sock, int http_code, const char*buffer, int total, int timeout);

void CloseSocket(int sock);

#endif // __COMMU_SOCKET_H__
