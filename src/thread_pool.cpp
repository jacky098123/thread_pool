#include <errno.h>
#include <libgen.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <map>
#include <utility>
#include <iostream>

#include <log4cplus/logger.h>   
#include <log4cplus/configurator.h>

#include "config.hpp"
#include "pool_common.hpp"
#include "commu_socket.hpp"
#include "request.hpp"
//#include "connection_manager.hpp"

using namespace std;
using namespace log4cplus;

#define     DEFAULT_BACKLOG_NUMBER      10
#define     DEFAULT_THREAD_NUMBER       10
#define     DEFAULT_PORT                9001

#define     DEFAULT_IN_MESSAGE_LENGTH   1024
#define     DEFAULT_OUT_MESSAGE_LENGTH  2048

#define     SOCKET_READ_TIMEOUT         3


typedef struct {
    pthread_t   thread_no;
    int			seq_no;
    int         processed_count;
} ThreadInfo_t;

typedef struct {
	int		backlog;
	int		thread_number;
	string	ip;
	int		port;
} ThreadSetting_t;


ThreadInfo_t                    *g_thread_info;
int                             g_thread_number;

int                             g_threads_count = 0;
pthread_cond_t                  g_threads_cond;
pthread_mutex_t                 g_threads_mutex;

int                             g_listen_socket;
int                             g_listen_sequence = 0;
pthread_mutex_t                 g_listen_mutex;

map<string, BusinessFunction>   g_command_function_mapping;


int get_program_info(const char* arg0, string &path, string &program) {
    char    buf[PATH_MAX + 1];

    realpath(arg0, buf);
    path = dirname(buf);

    strcpy(buf, arg0);
    char *p = basename(buf);
    program = p;

    return 0;
}

void sig_terminate(int signal_num) {
    DECLARE_POOL_LOG;

    LOG4CPLUS_DEBUG(POOL_LOG, "sig_terminate called: " << signal_num);
    LOG4CPLUS_DEBUG(POOL_LOG, "sig_terminate thread_id: " << pthread_self());
    return ;
}

int initialize_function_context(FunctionContext_t &context) {
    context.read_length = DEFAULT_READ_BUFFER_SIZE;
    context.write_length = DEFAULT_WRITE_BUFFER_SIZE;

    context.read_buffer = (char*)malloc(context.read_length);
    context.write_buffer = (char*)malloc(context.write_length);

    if (context.read_buffer == NULL || context.write_buffer == NULL) {
        return -1;
    }
    return 0;
}

int default_handler(const Request &request, FunctionContext_t &context) {
    DECLARE_POOL_LOG;

    LOG4CPLUS_WARN(POOL_LOG, "default_handler is called.");

    context.http_code   = 200;
    strcpy(context.write_buffer, "invalid path");
    context.write_current = strlen(context.write_buffer);
    return context.http_code;
}

int register_function(const string &command, BusinessFunction function) {
    map<string, BusinessFunction>::iterator it = g_command_function_mapping.find(command);

    if (it == g_command_function_mapping.end()) {
        //g_command_function_mapping.insert(make_pair(tmp, function));
        g_command_function_mapping.insert(map<string, BusinessFunction>::value_type(command, function));
        return 0;
    } else {
        it->second = function;
        return 1;
    }
}


BusinessFunction find_function(const string &command) {
    map<string, BusinessFunction>::iterator it = g_command_function_mapping.find(command);

    if (it != g_command_function_mapping.end()) {
        return it->second;
    } else {
        return default_handler;
    }
}


int server_socket() {
    int         backlog_number;
    int         listen_port;
    string      ip_address;
    int         flags = 1;
    struct sockaddr_in     sin;

    DECLARE_POOL_LOG;

    Config::Instance()->GetConfStr("general", "ip_address", ip_address);
    Config::Instance()->GetConfInt("general", "port", &listen_port, DEFAULT_PORT);
    Config::Instance()->GetConfInt("general", "backlog_number", &backlog_number, DEFAULT_BACKLOG_NUMBER);

    memset((void*)&sin, 0x00, sizeof(sin));
    sin.sin_family              = AF_INET;
    sin.sin_addr.s_addr         = inet_addr(ip_address.c_str());
    sin.sin_port                = htons(listen_port);

    g_listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(g_listen_socket, SOL_SOCKET, SO_REUSEADDR, (void*)&flags, sizeof(flags));

    if (bind(g_listen_socket, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        LOG4CPLUS_ERROR(POOL_LOG, "bind port error: " << ip_address << ":" << listen_port);
        return -1;
    }

    if (listen(g_listen_socket, backlog_number) < 0) {
        LOG4CPLUS_ERROR(POOL_LOG, "listen error");
        return -2;
    }

    pthread_mutex_init(&g_listen_mutex, NULL);
    return 0;
}

int read_http_data(FunctionContext_t &context) {
    context.read_current    = 0;
    context.write_current   = 0;

    DECLARE_POOL_LOG;

    while (true) {
        if (context.read_current == context.read_length) {
            size_t new_size = context.read_length * 1.5;
            char* new_buffer = (char*)realloc(context.read_buffer, new_size);
            if (new_buffer == NULL) {
                return -1;
            }
            context.read_length = new_size;
            context.read_buffer = new_buffer;
        }

        int length = HttpReceive(context.fd, context.read_buffer + context.read_current, 
                                 context.read_length - context.read_current, SOCKET_READ_TIMEOUT);
        LOG4CPLUS_DEBUG(POOL_LOG, "HttpReceive length: " << length);
        if (length < 0) {
            LOG4CPLUS_ERROR(POOL_LOG, "HttpReceive error: " << length);
            return length;
        }
        context.read_current += length;
        
        if (strstr(context.read_buffer, http_head_terminal) != NULL) {
           return context.read_current;
        } 
    }
}

void* thread_main(void* arg) {
    pthread_mutex_lock(&g_threads_mutex);
    g_threads_count++;
    pthread_cond_signal(&g_threads_cond);
    pthread_mutex_unlock(&g_threads_mutex);

    DECLARE_POOL_LOG;

    ThreadInfo_t* thread_info = (ThreadInfo_t*)arg;
    LOG4CPLUS_DEBUG(POOL_LOG, "thread seq: " << thread_info->seq_no << ", thread no: " << thread_info->thread_no);

    FunctionContext_t   function_context;
    if (initialize_function_context(function_context) < 0) {
        LOG4CPLUS_ERROR(POOL_LOG, "initialize_function_context error for thread seq: " << thread_info->seq_no);
        return NULL;
    }

    struct sockaddr     client_socket;
    socklen_t           client_len;
    int                 ret;

    BusinessFunction    handler;
    int                 http_code;

    for (;;) {
        pthread_mutex_lock(&g_listen_mutex);
        int fd = accept(g_listen_socket, &client_socket, &client_len);
        if (fd < 0) {
            LOG4CPLUS_ERROR(POOL_LOG, "accept error: " << strerror(errno) << ", thread no: " << thread_info->seq_no);
            pthread_mutex_unlock(&g_listen_mutex);
            continue;
        }
        function_context.listen_sequence    = g_listen_sequence++;
        function_context.fd                 = fd;
        pthread_mutex_unlock(&g_listen_mutex);
        LOG4CPLUS_DEBUG(POOL_LOG, "thread seq no: " << thread_info->seq_no << ", listern seq: " << function_context.listen_sequence);

        // read http
        if ((ret = read_http_data(function_context)) < 0) {
            close(function_context.fd);
            LOG4CPLUS_ERROR(POOL_LOG, "read_http_data err: " << ret);
            continue;
        }

        // handler
        Request     request(function_context.read_buffer, function_context.read_current);
        LOG4CPLUS_DEBUG(POOL_LOG, "path: " << request.path);
        handler     = find_function(request.path);
        http_code   = handler(request, function_context);

        // send http
        if ((ret = HttpSend(function_context.fd, http_code, function_context.write_buffer, 
                            function_context.write_current, SOCKET_READ_TIMEOUT)) < 0) {
            LOG4CPLUS_ERROR(POOL_LOG, "send_data err: " << ret);
            goto FINISH;
        }

        FINISH:
        LOG4CPLUS_DEBUG(POOL_LOG, "FINISH thread seq no: " << thread_info->seq_no << ", listern seq: " << function_context.listen_sequence);
        if (function_context.fd > 0) {
            LOG4CPLUS_DEBUG(POOL_LOG, "close fd: " << function_context.fd);
            close(function_context.fd);
        }
    }

    return NULL;
}


int thread_pool_initialize() {
    DECLARE_POOL_LOG;

    Config::Instance()->GetConfInt("general", "thread_number", &g_thread_number, DEFAULT_THREAD_NUMBER);

    g_thread_info = (ThreadInfo_t*)malloc((g_thread_number) * sizeof(ThreadInfo_t));
    memset(g_thread_info, 0x00, g_thread_number*sizeof(ThreadInfo_t));

    pthread_mutex_init(&g_threads_mutex, NULL);
    pthread_cond_init(&g_threads_cond, NULL);
    g_threads_count = 0;

    LOG4CPLUS_DEBUG(POOL_LOG, "begin create threads.");
    for (int i=0; i<g_thread_number; i++) {
        g_thread_info[i].seq_no = i;
        pthread_create(&(g_thread_info[i].thread_no), NULL, thread_main, &(g_thread_info[i]));
    }

    LOG4CPLUS_DEBUG(POOL_LOG, "wait threads.");
    pthread_mutex_lock(&g_threads_mutex);
    while (g_threads_count <g_thread_number) {
        pthread_cond_wait(&g_threads_cond, &g_threads_mutex);
    }
    pthread_mutex_unlock(&g_threads_mutex);

    LOG4CPLUS_DEBUG(POOL_LOG, "finish create threads.");

    return 0;
}

int thread_pool_finish() {
    for (int i=0; i<g_thread_number; i++) {
        pthread_join(g_thread_info[i].thread_no, NULL);
    }
    return 0;
}

int main(int argc, char** argv) {
    int     ret;
    string  path;
    string  program_name;
    bool    daemonize = false;
    char    c;

    get_program_info(argv[0], path, program_name);

    while ((c = getopt(argc, argv, "d")) != -1) {
        switch (c) {
            case 'd':
                daemonize = true;
                break;
            default:
                cout << "invalid option: " << c << endl;
        }
    }

    if (daemonize) {
        if (daemon(1, 0)) {
            cout << "daemon error" << endl;
            return -100;
        }
    }

    signal(SIGPIPE, sig_terminate);

    PropertyConfigurator::doConfigure("../conf/" + program_name + ".properties");

    DECLARE_POOL_LOG;
    LOG4CPLUS_DEBUG(POOL_LOG, "");
    LOG4CPLUS_DEBUG(POOL_LOG, "");
    LOG4CPLUS_DEBUG(POOL_LOG, "");
    LOG4CPLUS_DEBUG(POOL_LOG, "       --RUNNING--        ");

    Config::Instance()->LoadFile(path + "/../conf/" + program_name + ".ini");
    Config::Instance()->DumpInfo();

    int     read_buffer_length;
    int     write_buffer_length;
    Config::Instance()->GetConfInt("general", "read_buffer_length", &read_buffer_length, DEFAULT_READ_BUFFER_SIZE);
    Config::Instance()->GetConfInt("general", "write_buffer_length", &write_buffer_length, DEFAULT_WRITE_BUFFER_SIZE);

    // server socket
    ret = server_socket();
    if (ret < 0) {
        LOG4CPLUS_ERROR(POOL_LOG, "server_socket error: " << ret);
        exit(ret);
    }

    // special project 
    business_hook();

    cout << "g_command_function_mapping size: " << g_command_function_mapping.size() << endl;

    // create thread
    ret = thread_pool_initialize();
    if (ret < 0) {
        LOG4CPLUS_ERROR(POOL_LOG, "thread_pool_initialize error: " << ret);
        exit(ret);
    }

    // attach thread
    thread_pool_finish();

    return 0;
}
