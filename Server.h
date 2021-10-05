//
// Created by shahrooz on 10/4/21.
//

#ifndef SHORRENT__SERVER_H_
#define SHORRENT__SERVER_H_

#include "FileHandler.h"
#include <unistd.h>
#include <cstdio>
#include <sys/socket.h>
#include <cstdlib>
#include <netinet/in.h>

class Server {
  FileHandler fh;

  static int message_handle();
  static int start();
};

#endif //SHORRENT__SERVER_H_
