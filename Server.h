//
// Created by shahrooz on 10/4/21.
//

#ifndef SHORRENT__SERVER_H_
#define SHORRENT__SERVER_H_

#include "FileHandler.h"
#include <thread>

class Server {
 public:
  static FileHandler fh;
  static int sock;

  static void start();
  static void message_handle(int sock);

};

#endif //SHORRENT__SERVER_H_
