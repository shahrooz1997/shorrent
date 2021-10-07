//
// Created by shahrooz on 10/6/21.
//

#include "Peer.h"
#include <iostream>
#include <chrono>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " [IP:Port]" << std::endl;
    return -1;
  }
//  std::string address(argv[1]); // IP:Port
//  std::string address("127.0.0.1:8081"); // IP:Port
  Peer peer(argv[1]);
  // Start the peer server.
  std::thread(&Peer::start, peer).detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  while (true) {
    std::cout << "Enter your command: ";
    std::string command("fileList");
    std::cin >> command;
    if (command == "regFile") {
      std::string filename("sample");
      std::cout << "Enter the filename: ";
      std::cin >> filename;
      if (peer.registerFile(filename) != 0) {
        std::cout << "Error running registerFile" << std::endl;
      }
    } else if (command == "fileList") {
      std::vector<File> files;
      if (peer.fileList(files) != 0) {
        std::cout << "Error running fileList" << std::endl;
      } else {
        if (files.size() != 0) {
          std::cout << "Files:" << std::endl;
        } else {
          std::cout << "No files available" << std::endl;
        }
        int counter = 0;
        for (auto &f: files) {
          counter++;
          std::cout << counter << ": " << f.filename << "\t\t" << static_cast<double>(f.size) / 1024. << "KB"
                    << std::endl;
        }
      }
    } else if (command == "download") {
      std::string filename;
      std::cout << "Enter the filename: ";
      std::cin >> filename;
      File fileInfo;
      if (peer.getFileInfo(filename, fileInfo) != 0) {
        std::cout << "Error running getFileInfo" << std::endl;
      }
      if (peer.downloadFile(fileInfo) != 0) {
        std::cout << "Error running downloadFile" << std::endl;
      }
    } else if (command == "downloadChunk") {
      std::string filename;
      uint32_t chunkId;
      std::cout << "Enter the filename: ";
      std::cin >> filename;
      std::cout << "Enter the chunk id: ";
      std::cin >> chunkId;
      File fileInfo;
      if (peer.getFileInfo(filename, fileInfo) != 0) {
        std::cout << "Error running getFileInfo" << std::endl;
      }
      Chunk *chunk_p = nullptr;
      for (auto &ch: fileInfo.chunks) {
        if (ch.id == chunkId) {
          chunk_p = &ch;
          break;
        }
      }
      if (chunk_p != nullptr) {
        if (peer.downloadChunk(chunk_p->peers[0], fileInfo.filename, chunkId) != 0) {
          std::cout << "Error running downloadFile" << std::endl;
        }
      } else {
        std::cout << "Invalid chunk id" << std::endl;
      }
    } else if (command == "ls") {
      std::vector<std::string> files;
      listDir2(FILES_PATH, files);
      std::cout << "Available files:" << std::endl;
      for (auto &f: files) {
        std::cout << f << "\t\t" << static_cast<double>(fileSize(std::string(FILES_PATH) + f)) / 1024. << "KB"
                  << std::endl;
      }
      std::cout << std::endl;
    } else {
      std::cout << "Wrong command" << std::endl;
    }
  }
  return 0;
}