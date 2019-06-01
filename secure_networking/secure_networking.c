//Extern file to store function and procedures of the secure networking header.

#include "secure_header.h"

//------Definitions-------

//------ Connections - Server ------
int secure_socket(int __domain, int __type, int __protocol) {
    int __retfd;
    if((__retfd = socket(__domain, __type, __protocol)) == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    return __retfd;
}

void secure_bind(int __sfd, const struct sockaddr* __saddr, socklen_t __addrlen) {
    if(bind(__sfd, __saddr, __addrlen) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
}

void secure_listen(int __sfd, int __backlog) {
    if(listen(__sfd, __backlog) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

int secure_accept(int __sfd, struct sockaddr* __caddr, socklen_t* __addrlen) {
    int __retfd;
    if((__retfd = accept(__sfd, __caddr, __addrlen)) == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    return __retfd;

}

//------ Connection - Client ------
void secure_connect(int __sfd, const struct sockaddr* __saddr, socklen_t __addrlen) {
    if(connect(__sfd, __saddr, __addrlen) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
}

void secure_server_init(connectioninfo info, serverinfo* s_info) {
    //socket creation
    s_info->__serverfd = secure_socket(AF_INET, SOCK_STREAM, 0);

    //assigning properties to server info
    s_info->__serveraddr.sin_family = AF_INET;
    s_info->__serveraddr.sin_port = htons(info.__port);
    secure_pton(AF_INET, info.__ip, &s_info->__serveraddr.sin_addr.s_addr);

    //defining the server address lenght
    s_info->__serverlen = sizeof(s_info->__serveraddr);
}



//------ Secure translation ------
void secure_pton(int __af, const char* __src, void* __dst) {
    if(inet_pton(__af, __src, __dst) == -1) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }


}

void secure_ntop(int __af, const void* __src, char* __dst, socklen_t __addrlen) {
    if(inet_ntop(__af, __src, __dst, __addrlen) == NULL) {
        perror("inet_ntop");
        exit(EXIT_FAILURE);
    }
}

struct hostent* secure_gethostbyaddr(const void* __addr, socklen_t __len, int __type) {
    if (gethostbyaddr(__addr, __len, __type) == NULL) {
        perror("gethostbyaddr");
        exit(EXIT_FAILURE);
    }
}


//------ Sending message ------
int full_send(int __sfd, void* __buf, size_t __buflen, int __flags, int __mode, int __type, int __filefd) {
    int retval, bytes_sent = 0;
    while (bytes_sent < __buflen) { //while still to send
        //if sending type is a file
        if(__type == FILE) {
            retval = sendfile(__sfd, __filefd, NULL, __buflen);
        } 
        else if(__type == NONFILE) {
            retval = send(__sfd, __buf, __buflen, __flags);
        }
        //check for errors
        if (retval == -1) {
            //if something interrupted the read
            if(errno == EINTR) {
                //resume from the last byte received
                retval = 0;
            } else {
                find_errno(__sfd, __mode);
            }
        } 
        else if (retval == 0) { //EOF reached or disconnecting
            find_errno(__sfd, __mode);
        }

        //if here, data was sent
        //updating the buf position and the remaining buf lenght
        if(__type != FILE) {
            __buf += bytes_sent;
        }
        __buflen -= bytes_sent;

        //increasing the number of bytes sent
        bytes_sent += retval;
    }

    return bytes_sent;

}

int full_recv(int __sfd, void* __buf, size_t __buflen, int __flags, int __mode, int  __type, int __filefd) {
    int retval, bytes_received = 0;
    while(bytes_received < __buflen){ //while still to receive
        //if sending type is a file
        retval = recv(__sfd, __buf, __buflen, 0);
        //check for errors
        if (retval == -1) {
            //if something interrupted the read
            if(errno == EINTR) {
                //resume from the last byte received
                retval = 0;
            } else {
                find_errno(__sfd, __mode);
            }
        } 
        else if (retval == 0) { //EOF reached or disconnecting
            find_errno(__sfd, __mode);
        }
        
        //if here, data was received
        //updating the buf position and the remaining buf lenght
        __buf += bytes_received;
        __buflen -= bytes_received;

        //increasing the number of bytes received
        bytes_received += retval;

        //check if file type or not
        if(__type == FILE) {
            if(write(__filefd, __buf, retval) <= 0) {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }
        
    }

    return bytes_received;

}

int secure_recv(int __sfd, void* __buf, size_t __buflen, int __flags, int __mode, int __type, int __filefd) {
    
    //receiving data
    int bytes_recv = full_recv(__sfd, __buf, __buflen, __flags, __mode, __type, __filefd);

    //telling the sender that the data was successful read
    full_send(__sfd, ACKNOWLEDGE, ACKSIZE, MSG_NOSIGNAL, __mode, NONFILE, 0);

    //return the byte recvd
    return bytes_recv;
}

int secure_send(int __sfd, void* __buf, size_t __buflen, int __flags, int __mode, int __type, int __filefd) {
    
    //declaring ack buffer and cleaning it
    char ACKNOWLEDGE_RECV[ACKSIZE];
    memset(ACKNOWLEDGE_RECV, 0, ACKSIZE);
    
    //sending data
    int bytes_sent = full_send(__sfd, __buf, __buflen, __flags, __mode, __type, __filefd);

    //waiting for the acknowedge of the data sent from the receiver
    full_recv(__sfd, ACKNOWLEDGE_RECV, ACKSIZE, 0, __mode, __type, __filefd);

    //return byte sent
    return bytes_sent;
}


void find_errno(int __fd, int __mode) {

    switch(errno) {    
        case ECONNRESET:
            //if client resetted con
            close(__fd);
            if(__mode == SERVER) {
                perror("Thread exiting:");
                pthread_exit(NULL);
            } 
            else {
                fprintf(stderr, "Server reset connection, exiting..\n");
                exit(EXIT_FAILURE);
            }
            break;
        
        case ECONNREFUSED:
            close(__fd);
            if(__mode == SERVER) {
                perror("Thread exiting:");
                pthread_exit(NULL);
            } else {
                fprintf(stderr, "Server refused connection, exiting..\n");
                exit(EXIT_FAILURE);
            }
            break;

        case EPIPE:
            //if client resetted con
            close(__fd);
            if(__mode == SERVER) {
                perror("Thread exiting:");
                pthread_exit(NULL);
            } 
            else {
                //if server close con, return
                fprintf(stderr, "Could not send/receive data to/from server, exiting..\n");
                exit(EXIT_FAILURE);
            }
            break;

        default:
            //close con
            close(__fd);
            if(__mode == SERVER) {
                perror("Thread exiting:");
                pthread_exit(NULL);
            }
            else {
                fprintf(stderr, "Could not reach the server, exiting.\n");
                exit(EXIT_FAILURE);
            }
        break;

    }

}


//------ Thread Managing -------
void secure_pthread_create(pthread_t* __tid, const pthread_attr_t* __attr, void* (*__start_rtn)(void*), void* __arg) {
    int errnum;
    if((errnum = pthread_create(__tid, __attr, __start_rtn, __arg)) != 0) {
        fprintf(stderr, "pthread_create: %d", errnum);
        exit(EXIT_FAILURE);
    }
}
