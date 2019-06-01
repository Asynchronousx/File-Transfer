#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/sendfile.h>

//------ Global Define ------
#define BACKLOG 32
#define CLIENT 0
#define OFFLINE 0
#define SERVER 1
#define ONLINE 1

//defines codes
#define TERM 40
#define FAIL 41
#define SUCCESS 42
#define DISCONNECTED 43

//define types
#define FILE 44
#define NONFILE 45

//define sizes
#define ACKSIZE 2
#define MINBUFSIZE 8
#define MAXIDSIZE 16
#define MAXDEPSIZE 32
#define BUFSIZE 32
#define MAXBUFSIZE 64

//define colors
#define GREEN "\033[32m"
#define RED "\033[31m"
#define RESET "\033[0m"

//define global var
static char* ACKNOWLEDGE = "OK";

//------ Struct Define ------
struct s_info {
    struct sockaddr_in __serveraddr;
    int __serverfd;
    socklen_t __serverlen;
};

struct c_info {
    struct sockaddr_in __clientaddr;
    int __clientfd;
    socklen_t __clientlen;
};

struct con_info {
    int __port;
    char __ip[INET_ADDRSTRLEN];
};


//------ Typedefs ------
typedef struct s_info serverinfo;
typedef struct c_info clientinfo;
typedef struct con_info connectioninfo;


//------ Prototipes ------
//Connection - Server
int secure_socket(int __domain, int __type, int __protocol);
void secure_bind(int __sfd, const struct sockaddr* __saddr, socklen_t __addrlen);
void secure_listen(int __sfd, int __backlog);
int secure_accept(int __sfd, struct sockaddr* __caddr, socklen_t* __addrlen);

//Connection - Client
void secure_connect(int __sfd, const struct sockaddr* __saddr, socklen_t __addrlen);
void secure_server_init(connectioninfo __coninfo, serverinfo* __servinfo);

//Translation 
void secure_pton(int __af, const char* __src, void* __dst);
void secure_ntop(int __af, const void* __src, char* __dst, socklen_t __addrlen);
struct hostent* secure_gethostbyaddr(const void* __addr, socklen_t __len, int __type);

//Message Passing
int full_recv(int __sfd, void* __buf, size_t __buflen, int __flags, int __mode, int __type, int __filefd);
int secure_recv(int __sfd, void* __buf, size_t __buflen, int __flags, int __mode, int __type, int __filefd);
int full_send(int __sfd, void* __buf, size_t __buflen, int __flags, int __mode, int __type, int __filefd);
int secure_send(int __sfd, void* __buf, size_t __buflen, int __flags, int __mode, int __type, int __filefd);
void find_errno(int __fd, int __mode);

//Pthread_safe
void secure_pthread_create(pthread_t* __tid, const pthread_attr_t* __attr, void* (*__start_rtn)(void*), void* __arg);
