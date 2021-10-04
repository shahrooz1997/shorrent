//
// Created by shahrooz on 10/3/21.
//

#include "Server.h"

int Server::registerFiles(const std::string& address, const std::vector<File> &files) {
  for(auto file: files) {
    this->files.emplace_back(file.filename, file.size, address);
  }
  return 0;
}
int Server::fileList(std::vector<std::string> &files) {
  return 0;
}
int Server::getFileInfo(const std::string &filename) {
  return 0;
}
int Server::registerChunk(const std::string &address, const std::string &filename, uint32_t id) {
  return 0;
}
