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
#include <arpa/inet.h>

#define CHUNK_DEFAULT_SIZE (1024 * 1024)
#define SERVER_IP INADDR_ANY
#define SERVER_PORT 8080
#define BACKLOG_SIZE 1024
#define FILES_PATH "files/"
#define CHUNKS_PATH "chunks/"
#define MAX_NUM_OF_THREADS 8

#define DPRINTF(flag, fmt, ...) \
    do{ \
        if(flag) \
            fprintf(stdout, "Time %10li - Thread: %lu : [%s][%s]%d: " fmt, time(nullptr), pthread_self(), __FILE__, __func__, __LINE__, ##__VA_ARGS__); \
        fflush(stdout); \
    } while(0)

int recvData(int sock, std::string& data);
int sendData(int sock, const std::string& data);

#endif //SHORRENT__UTIL_H_
