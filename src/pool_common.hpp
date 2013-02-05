#ifndef __COMMON_H__
#define __COMMON_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

#include <log4cplus/logger.h>   
#include <log4cplus/configurator.h>

#include "request.hpp"

#define DECLARE_POOL_LOG    log4cplus::Logger pool_log = log4cplus::Logger::getInstance("pool")
#define POOL_LOG            pool_log

#define DEFAULT_READ_BUFFER_SIZE	1024
#define DEFAULT_WRITE_BUFFER_SIZE	2048

using namespace std;

typedef struct {
    int     http_code;
    char    *read_buffer;
    int     read_length;
    int		read_current;
    char    *write_buffer;
    int     write_length;
    int		write_current;

    string  client_ip;
    int     client_port;

    int	    listen_sequence;
    int		fd;
} FunctionContext_t;

typedef int (*BusinessFunction)(const Request &request, FunctionContext_t &context);

void business_hook();
int register_function(const string &command, BusinessFunction function);


#endif // __COMMON_H__
