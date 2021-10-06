//
// Created by shahrooz on 10/6/21.
//

#include "Peer.h"
#include <iostream>

int main(int argc, char* argv[]) {
//  if (argc < 2) {
//    return -1;
//  }
  std::string address(argv[1]); // IP:Port
//  std::string address("127.0.0.1:8081"); // IP:Port
  Peer peer(address);
  // Start the peer server.
  std::thread(&Peer::start, peer).detach();
  while(true) {
    std::string command("fileList");
    std::cin >> command;
    if (command == "regFile") {
      std::string filename("sample");
      std::cin >> filename;
      if (peer.registerFile(filename) != 0) {
        DPRINTF(true, "Error running registerFile\n");
      }
    } else if (command == "fileList") {
      std::vector<File> files;
      if (peer.fileList(files) != 0) {
        DPRINTF(true, "Error running fileList\n");
      }
      std::cout << "Files:" << std::endl;
      int counter = 0;
      for (auto& f: files) {
        counter++;
        std::cout << counter << ": " << f.filename << std::endl;
      }
    } else if (command == "download") {
      std::string filename;
      std::cin >> filename;
      File fileInfo;
      if (peer.getFileInfo(filename, fileInfo) != 0) {
        DPRINTF(true, "Error running getFileInfo\n");
      }
      if (peer.downloadFile(fileInfo) != 0) {
        DPRINTF(true, "Error running downloadFile\n");
      }
    } else {
      DPRINTF(true, "Wrong command\n");
    }
  }
  return 0;
}