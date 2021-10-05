//
// Created by shahrooz on 10/3/21.
//

#ifndef SHORRENT__FILEHANDLER_H_
#define SHORRENT__FILEHANDLER_H_

#include "File.h"
#include <vector>
#include <mutex>
#include <thread>

class FileHandler {
 public:
  int registerFiles(const std::string& address, const std::vector<File>& files);
  int fileList(std::vector<std::string>& files);
  int getFileInfo(const std::string& filename, File &file);
  int registerChunk(const std::string& address, const std::string& filename, uint32_t id);

 private:
  std::vector<File> files;
  std::mutex mtx;

};

#endif //SHORRENT__FILEHANDLER_H_
