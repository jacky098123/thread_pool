#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "commu_socket.hpp"
using namespace std;

#define closesocket close
typedef struct sockaddr_in SOCKADDR_IN;

#ifndef INADDR_NONE
#define INADDR_NONE             -1
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR            -1
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET          -1
#endif

#define SOCKET_CLOSED           -2
#define SOCKET_TIMEOUT          -3

#define LISEN_COUNT             20


#define DEFAULT_TIMEOUT         10      /* timeout in seconds */
#define DEFAULT_NONB_TIMEOUT    100000  /* non-block timeout */
#define ONE_SEC                 1000000 //���΢��֮���ת��

const char*						http_head_terminal = "\r\n\r\n";
#define HTTP_HEADER_LENGTH      1024
#define JUMP_URL                "/"

/***************************************
 * To create a TCP server socket listening
 * on the given port
 * Parameter:
 *  port: the port to listen at
 * Return:
 *  If the server socket is created
 * successfully, the socket handle
 * will be returned.
 *  INVALID_SOCKET otherwise.
 ***************************************/
int SocketServer(int port)
{
	SOCKADDR_IN sin;
	int socket_handle;
	int err;
	int opt = 1;/* 1: Enable,  0: Disable */
	
	socket_handle = socket(AF_INET,SOCK_STREAM,0);
	if(socket_handle == INVALID_SOCKET)
		return INVALID_SOCKET;

	setsockopt(socket_handle, SOL_SOCKET, SO_REUSEADDR, (const void*)&opt, sizeof(opt));

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons((short)port);
	sin.sin_addr.s_addr = INADDR_ANY;
	
	err = bind(socket_handle, (struct sockaddr*)&sin, sizeof(sin));
	if(err==SOCKET_ERROR)
	{
		closesocket(socket_handle);
		return INVALID_SOCKET;
	}

	err = listen(socket_handle, LISEN_COUNT);
	if(err==SOCKET_ERROR)
	{
		closesocket(socket_handle);
		return INVALID_SOCKET;
	}

	return socket_handle;
}


int Accept(int serversock)
{
	struct sockaddr addr={0};
	socklen_t addrlen={0};
	int i=sizeof(sockaddr);
	addrlen = (socklen_t)i;
	return accept(serversock, &addr, &addrlen);
}

static unsigned long GetIPAddr(const char* host)
{
	unsigned long addr;
	
	if (host == NULL)
		return 0;
		
	addr = inet_addr(host);
	if (addr == INADDR_NONE){
		struct hostent* pHostent = gethostbyname(host);
		if (pHostent == NULL)
			return 0;
		else
			addr = *(unsigned long*)pHostent->h_addr_list[0];
	}

	return addr;
}


int Socket(const char *server, int port)
{
	unsigned long ip = GetIPAddr(server);
	SOCKADDR_IN sin;
	int sock;
	
	if(ip == 0) {
		return INVALID_SOCKET;
	}

	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock == INVALID_SOCKET)
		return INVALID_SOCKET;

	/* connect to server */
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = ip;
	sin.sin_port = htons((u_short)port);
	if(connect(sock, (struct sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR){
		closesocket(sock);
		return INVALID_SOCKET;
	}

	return sock;
}

static int CheckSocketSendTimeOut(int hSocket,int time)
{
	fd_set set;
	struct timeval tv;
	int result;
	
	FD_ZERO(&set);
	FD_SET((unsigned)hSocket,&set);

	tv.tv_sec = time;
	tv.tv_usec = 0;

	result = select(hSocket+1,NULL,&set,NULL,&tv);
	return result;
}

static int CheckSocketRecvTimeOut(int hSocket,int time)
{
	fd_set set;
	struct timeval tv;
	int result;
	
	FD_ZERO(&set);
	FD_SET((unsigned)hSocket,&set);

	tv.tv_sec = time;
	tv.tv_usec = 0;

	result = select(hSocket+1,&set,NULL,NULL,&tv);
	return result;
}

int Send(int sock, const char *buffer, int total, int timeout)
{
	int retval;
	int sentSize = 0;
	
	if (buffer == NULL)
		return 0;
		
	if (timeout <= 0) {
		timeout = DEFAULT_TIMEOUT;
	}
	while(sentSize < total) {
		int res = CheckSocketSendTimeOut(sock, timeout);
		if(res < 0) {
			return SOCKET_ERROR;
		} else if (res == 0) {
			if (sentSize == 0) {
				return SOCKET_TIMEOUT;
			} else {
				return sentSize;
			}
		}
	
		retval = send(sock, buffer+sentSize, total-sentSize, 0);
		if (retval == SOCKET_ERROR) {
			return SOCKET_ERROR;
		}
		
		sentSize += retval;
	}
	
	return total;
}

int Receive(int sock, char *buffer, int size, int timeout)
{
	int receiveResult;
	int recvSize = 0;
	
	if (buffer == NULL)
		return 0;
	
	if (timeout <= 0) {
		timeout = DEFAULT_TIMEOUT;
	}

	while (recvSize < size)	{
		receiveResult = CheckSocketRecvTimeOut(sock, timeout);
		if(receiveResult < 0) {
			if (recvSize <= 0) {
				return SOCKET_ERROR;
			} else {
				return recvSize;
			}
		} else if (receiveResult == 0) {
			if (recvSize <= 0) {
				return SOCKET_TIMEOUT;
			} else {
				return recvSize;
			}
		}
		receiveResult = recv(sock, buffer+recvSize, size-recvSize, MSG_DONTWAIT);
	
		if (receiveResult == 0) {
			if (recvSize <= 0) {
				return SOCKET_CLOSED;
			} else {
				return recvSize;
			}
		} else if (receiveResult < 0) {
			if (EAGAIN == errno) {
				continue;
			}
			if (recvSize <= 0) {
				return SOCKET_ERROR;
			} else  {
				return recvSize;
			}
		}
		
		recvSize += receiveResult;
	}
	return size;
}

// only handle
int HttpReceive(int sock, char **buffer, int *buffer_size, int timeout) {
	int receive_result;
	int recv_size = 0;
    int body_length = -1;

    enum HTTP_METHOD http_method = METHOD_NONE;

	if (buffer == NULL || *buffer == NULL)
		return -1;

	if (timeout <= 0) {
		timeout = DEFAULT_TIMEOUT;
	}

	while (1) {
		receive_result = CheckSocketRecvTimeOut(sock, timeout);
		if(receive_result < 0) {
            return SOCKET_ERROR;
		} else if (receive_result == 0) {
            return SOCKET_TIMEOUT;
		}

		receive_result = recv(sock, *buffer+recv_size, *buffer_size-recv_size, MSG_DONTWAIT);
		if (receive_result == 0) {
            return SOCKET_CLOSED;
		} else if (receive_result < 0) {
			if (EAGAIN == errno) {
				continue;
			}
            return SOCKET_ERROR;
		}

		recv_size += receive_result;

        if (recv_size >= *buffer_size) {
            size_t new_size = *buffer_size * 1.5;
            char* new_buffer = (char*)realloc(*buffer, new_size);
            if (new_buffer == NULL) {
                return -1000;
            }
            *buffer_size = new_size;
            *buffer = new_buffer;
        }

		// check the http head terminal
		char *p = strstr(*buffer, http_head_terminal);
        if (p == NULL) {
            continue;
        }

        if (http_method == METHOD_NONE) {
            if (strncmp(*buffer, "GET", 3) == 0)
                http_method = METHOD_GET;
            else if (strncmp(*buffer, "POST", 4) == 0)
                http_method = METHOD_POST;
            else
                http_method = METHOD_OTHER;
        }

		if (http_method == METHOD_GET || http_method == METHOD_OTHER) {
			return recv_size;
		} else if (http_method == METHOD_POST) {
            if (body_length < 0) {
                const char *CONTENT_LENGTH = "Content-Length:";
                char *content_length = strstr(*buffer, CONTENT_LENGTH);
                if (content_length == NULL) {
                    continue;
                }
                content_length += strlen(CONTENT_LENGTH);
                while (*content_length == ' ')
                    content_length++;

                body_length = atoi(content_length);
            }

            if (recv_size >= *buffer_size || recv_size >= body_length + 4 + (p-*buffer)) {
                return recv_size;
            }
        } else {
			return recv_size;
        }
	}
    return recv_size;
}

static size_t add_http_200(char** header_buffer, int content_len)
{
    char   now_buf[256];
	time_t tNow = time(NULL);
	struct tm tmNow = *localtime(&tNow);
	strftime(now_buf,sizeof(now_buf),"%a, %d %b %Y %H:%M:%S GMT",&tmNow);
    *header_buffer = (char*)malloc(HTTP_HEADER_LENGTH);
	
    const char* fmt = "HTTP/1.1 200 OK\r\n\
Connection: close\r\n\
Content-Type: text/plain; charset=utf-8\r\n\
Expires: Mon, 26 Jul 1997 05:00:00 GMT\r\n\
Cache-Control: no-cache must-revalidate\r\n\
Last-Modified:  %s\r\n\
Pragma: no-cache\r\n\
Content-Length: %zu\r\n\
\r\n";

     return snprintf(*header_buffer, HTTP_HEADER_LENGTH, fmt, now_buf, content_len);   
}

static size_t add_http_301(char** header_buffer) {
    const char* fmt = "HTTP/1.1 301 Moved temporarily\r\n\
Connection: keep-alive\r\n\
Location: %s\r\n\
Content-length: %zu\r\n\
\r\n%s";
    const char* body = "301 Moved temporarily";

    *header_buffer = (char*)malloc(HTTP_HEADER_LENGTH);
    size_t size = snprintf(*header_buffer, HTTP_HEADER_LENGTH, fmt, JUMP_URL, strlen(body), body);
    return size;
}

static size_t add_http_404(char** header_buffer) {
    const char* fmt = "HTTP/1.1 404 Object Not Found\r\n\
Connection: close\r\n\
Content-Type: text/plain; charset=utf-8\r\n\
Content-length: %zu\r\n\
\r\n%s";
    const char* body = "404 Object Not Found";

    *header_buffer = (char*)malloc(HTTP_HEADER_LENGTH);
    size_t size = snprintf(*header_buffer, HTTP_HEADER_LENGTH, fmt, strlen(body), body);

    return size;
}

static size_t add_http_501(char** header_buffer) {
    const char* fmt = "HTTP/1.1 501 Not Implemented\r\n\
Connection: close\r\n\
Content-Type: text/plain; charset=utf-8\r\n\
Content-length: %zu\r\n\
\r\n%s";
    const char* body = "501 Not Implemented";

    *header_buffer = (char*)malloc(HTTP_HEADER_LENGTH);
    size_t size = snprintf(*header_buffer, HTTP_HEADER_LENGTH, fmt, strlen(body), body);
    return size;
}


int HttpSend(int sock, int http_code, const char*buffer, int total, int timeout) {
    size_t  ret;
    int     error_code = 0;
    char*   header_buffer = NULL;

    switch(http_code) {
        case 200:
            if ((ret = add_http_200(&header_buffer, total)) < 0) {
                error_code = -201;
                break;
            }
            if ((ret = Send(sock, header_buffer, strlen(header_buffer), timeout)) < 0) {
                error_code = -202;
                break;
            }
            if ((ret = Send(sock, buffer, total, timeout)) < 0) {
                error_code = -202;
                break;
            }
            break;
        case 301:
            if ((ret = add_http_301(&header_buffer)) < 0) {
                error_code = -301;
                break;
            }
            if ((ret = Send(sock, header_buffer, strlen(header_buffer), timeout)) < 0) {
                error_code = -302;
                break;
            }
            break;
        case 404:
            if ((ret = add_http_404(&header_buffer)) < 0) {
                error_code = -401;
                break;
            }
            if ((ret = Send(sock, header_buffer, strlen(header_buffer), timeout)) < 0) {
                error_code = -402;
                break;
            }
            break;
        default:
            ret = add_http_501(&header_buffer);
            if ((ret = Send(sock, header_buffer, strlen(header_buffer), timeout)) < 0) {
                error_code = -502;
                break;
            }
    }
    if (header_buffer != NULL) {
        free(header_buffer);
    }
    return error_code;
}

void CloseSocket(int sock)
{
	closesocket(sock);
	return;
}
