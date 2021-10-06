//
// Created by shahrooz on 10/5/21.
//

#ifndef SHORRENT__PEER_H_
#define SHORRENT__PEER_H_

#include "Util.h"
#include "FileHandler.h"
#include <thread>

class Peer {
 public:
  static FileHandler fh;
  std::string address; // IP:PORT
  int sock;

  Peer(const std::string &address);

  void start();
  void message_handle(int sock);

  int registerFile(const std::string& filename);
  int fileList(std::vector<File>& files);
  int downloadFile(File& file);

 private:
  int getFileInfo(const std::string &filename, File &fileInfo);
  int registerChunk(const std::string& filename, uint32_t id);
  int getChunk(const std::string& address, const std::string& filename, uint32_t id);
  int downloadChunk(const std::string address, const std::string filename, uint32_t id);
};

#endif //SHORRENT__PEER_H_
