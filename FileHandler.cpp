//
// Created by shahrooz on 10/3/21.
//

#include "FileHandler.h"

int FileHandler::registerFiles(const std::string &address, const std::vector<File> &files) {
  std::lock_guard<std::mutex> lck(this->mtx);
  bool existed;
  for (auto& file: files) {
    existed = false;
    for (auto& f: this->files) {
      if (f.filename == file.filename) {
        for (auto& chunk: f.chunks) {
          chunk.add_peer(address);
        }
        existed = true;
        break;
      }
    }
    if (!existed) {
      this->files.emplace_back(file.filename, file.size, address);
    }
  }
  return 0;
}

int FileHandler::fileList(std::vector<File> &files) {
  std::lock_guard<std::mutex> lck(this->mtx);
  for (auto& file: this->files) {
    files.emplace_back(file.filename, file.size);
  }
  return 0;
}

int FileHandler::getFileInfo(const std::string &filename, File &file) {
  std::lock_guard<std::mutex> lck(this->mtx);
  for (auto& f: this->files) {
    if (f.filename == filename) {
      file = f;
      return 0;
    }
  }
  return -1; // File not found.
}

int FileHandler::registerChunk(const std::string &address, const std::string &filename, uint32_t id) {
  std::lock_guard<std::mutex> lck(this->mtx);
  for (auto& f: this->files) {
    if (f.filename == filename) {
      for (auto& chunk: f.chunks) {
        if (chunk.id == id) {
          chunk.add_peer(address);
        }
      }
      return -2; // Chunk not found.
    }
  }
  return -1; // File not found.
}

int FileHandler::getChunk(const std::string &filename, uint32_t id, std::string &data) {
  std::ifstream inFile;
  inFile.open(std::string(CHUNKS_PATH) + filename + "!!!" + std::to_string(id));
  if (!inFile.is_open()) {
    return -1;
  }
  std::stringstream strStream;
  strStream << inFile.rdbuf();
  inFile.close();
  data = strStream.str();
  return 0;
}
