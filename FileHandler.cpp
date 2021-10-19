//
// Created by shahrooz on 10/3/21.
//

#include "FileHandler.h"
#include <algorithm>
#include "DataSerialization.h"

FileHandler::FileHandler() {
  this->readStateFromFile();
}

FileHandler::FileHandler(bool isPeer) {
  if (isPeer) {
    std::vector<std::string> dirFiles;
    listDir(CHUNKS_PATH, dirFiles);
    for (auto& fname: dirFiles) {
      std::string filename = fname.substr(0, fname.find("!!!"));
      uint32_t chunkId = static_cast<uint32_t>(std::stoul(fname.substr(fname.find("!!!") + 3)));
      addChunkPeer(filename, chunkId, fileSize(std::string(CHUNKS_PATH) + fname));
    }
  } else {
    this->readStateFromFile();
  }
}

int FileHandler::registerFiles(const std::string &address, const std::vector<File> &files) {
  std::lock_guard<std::mutex> lck(this->mtx);
  bool existed;
  for (auto& file: files) {
    existed = false;
    for (auto& f: this->files) {
      if (f.filename == file.filename) {
        for (auto& chunk: f.chunks) {
          if (std::find(chunk.peers.begin(), chunk.peers.end(), address) == chunk.peers.end()) {
            chunk.add_peer(address);
          }
        }
        existed = true;
        break;
      }
    }
    if (!existed) {
      this->files.emplace_back(file.filename, file.size, address);
    }
  }
  this->saveStateToFile();
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
          if (std::find(chunk.peers.begin(), chunk.peers.end(), address) == chunk.peers.end()) {
            chunk.add_peer(address);
            this->saveStateToFile();
          }
          return 0;
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

int FileHandler::readStateFromFile() {
  std::unique_ptr<std::vector<File>> files_p;
  std::ifstream in("state.bin");
  if (!in.is_open()) {
    return -1;
  }
  DataSerialization::deserializeFileList(&in, files_p);
  in.close();
  this->files = std::move(*files_p);
  return 0;
}

int FileHandler::saveStateToFile() {
  std::ofstream out("state.bin");
  DataSerialization::serializeToFileList(&out, this->files);
  out.close();
  return 0;
}

int FileHandler::addChunkPeer(const std::string &filename, uint32_t id, uint32_t chunkSize) {
  for (auto& f: this->files) {
    if (f.filename == filename) {
      f.chunks.emplace_back(id, filename, "", chunkSize, ChunkState::done);
      return 0;
    }
  }
  this->files.emplace_back(filename, 0);
  this->files.back().chunks.emplace_back(id, filename, "", chunkSize, ChunkState::done);
  return 0;
}
