//
// Created by shahrooz on 10/3/21.
//

#ifndef SHORRENT__UTIL_H_
#define SHORRENT__UTIL_H_

#include <cstdint>
#include <unistd.h>
#include <cstdio>
#include <sys/socket.h>
#include <cstdlib>
#include <netinet/in.h>
#include <string>

#define CHUNK_DEFAULT_SIZE_MB 1024
#define SERVER_IP INADDR_ANY
#define SERVER_PORT 8080
#define BACKLOG_SIZE 1024

#define DPRINTF(flag, fmt, ...) \
    do{ \
        if(flag) \
            fprintf(stdout, "Time %10li - Thread: %lu : [%s][%s]%d: " fmt, time(nullptr), pthread_self(), __FILE__, __func__, __LINE__, ##__VA_ARGS__); \
        fflush(stdout); \
    } while(0)

int recvData(int sock, std::string& data);
int sendData(int sock, const std::string& data);

#endif //SHORRENT__UTIL_H_
