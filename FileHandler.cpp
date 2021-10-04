//
// Created by shahrooz on 10/3/21.
//

#include "FileHandler.h"

int FileHandler::registerFiles(const std::string &address, const std::vector<File> &files) {
  for (auto file: files) {
    this->files.emplace_back(file.filename, file.size, address);
  }
  return 0;
}

int FileHandler::fileList(std::vector<std::string> &files) {
  for (auto file: this->files) {
    files.emplace_back(file.filename);
  }
  return 0;
}

int FileHandler::getFileInfo(const std::string &filename, File &file) {
  for (auto f: this->files) {
    if (f.filename == filename) {
      file = f;
      return 0;
    }
  }
  return -1; // File not found.
}

int FileHandler::registerChunk(const std::string &address, const std::string &filename, uint32_t id) {
  for (auto f: this->files) {
    if (f.filename == filename) {
      for (auto chunk: f.chunks) {
        if (chunk.id == id) {
          chunk.add_peer(address);
        }
      }
      return -2; // Chunk not found.
    }
  }
  return -1; // File not found.
}
