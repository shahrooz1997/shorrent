//
// Created by shahrooz on 10/3/21.
//

#ifndef SHORRENT__SERVER_H_
#define SHORRENT__SERVER_H_

#include "File.h"
#include <vector>

class Server {
 public:
  std::vector<File> files;

  int registerFiles(const std::string address, const std::vector<File>& files);
  int fileList(std::vector<std::string>& files);
  int getFileInfo(const std::string& filename);
  int registerChunk(const std::string& address, const std::string& filename, uint32_t id);
};

#endif //SHORRENT__SERVER_H_
