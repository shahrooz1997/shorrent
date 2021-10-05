//
// Created by shahrooz on 10/3/21.
//

#include "Util.h"
#include <string>
#include <assert.h>

int recvData(int sock, void* buf, size_t size){
  char* iter = static_cast<char*>(buf);
  ssize_t bytes_read = 0;
  while(size > 0){
    if((bytes_read = recv(sock, iter, size, 0)) < 1){
      if(bytes_read == -1){
        printf("Error: host socket, errno is %d\n", errno);
        return -1;
      }
      else if(bytes_read == 0){
        printf("Warn: remote socket closed\n");
      }
      return 0;
    }
    iter += bytes_read;
    size -= bytes_read;
  }
  return 0;
}

int recvData(int sock, std::string &data) {
  uint32_t size = 0;
  int result;
  data.clear();
  result = recvData(sock, &size, sizeof(size));
  if(result == 0){
    size = ntohl(size);
    if(size > 0){
      char* buffer = new char[size];
      result = recvData(sock, buffer, size);
      if (result == 0) {
        data = std::string(buffer, size);
        delete[] buffer;
      } else {
        delete[] buffer;
        return result;
      }
    }
  }
  if(data.empty()){
    return -2;
  }
  return 0;
}

int sendData(int sock, const void* data, size_t size){
  const char* iter = static_cast<const char*>(data);
  ssize_t bytes_sent = 0;
  while(size > 0){
    if((bytes_sent = send(sock, iter, size, 0)) < 1){
      if(bytes_sent == -1){
        printf("sendData Error: host socket, errno is %d\n", errno);
        return -1;
      }
      else if(bytes_sent == 0){
        printf("sendData Warn: remote socket closed\n");
      }
      return 0;
    }
    iter += bytes_sent;
    size -= bytes_sent;
  }
  return 0;
}

int sendData(int sock, const std::string& data){
  assert(!data.empty());
  uint32_t size = htonl(static_cast<uint32_t>(data.size()));
  int result = sendData(sock, &size, sizeof(size));
  if(result == 0){
    result = sendData(sock, data.c_str(), data.size());
  }
  return result;
}