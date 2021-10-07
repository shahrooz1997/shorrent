//
// Created by shahrooz on 10/3/21.
//

#include "FileHandler.h"
#include "gbuffer.pb.h"

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
          chunk.add_peer(address);
          this->saveStateToFile();
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
  shorrent::FileList fileList;
  std::ifstream in("state.bin");
  if (!in.is_open()) {
    return -1;
  }
  fileList.ParseFromIstream(&in);
  in.close();

  for (int i = 0; i < fileList.files_size(); i++) {
    this->files.emplace_back(fileList.files(i).filename(), fileList.files(i).size());
    for (int j = 0; j < fileList.files(i).chunks_size(); j++) {
      this->files.back().chunks.emplace_back(fileList.files(i).chunks(j).id(), fileList.files(i).chunks(j).filename(),
                                             fileList.files(i).chunks(j).path(), fileList.files(i).chunks(j).size(),
                                             static_cast<ChunkState>(fileList.files(i).chunks(j).state()));
      for (int k = 0; k < fileList.files(i).chunks(j).peers_size(); k++) {
        this->files.back().chunks.back().peers.push_back(fileList.files(i).chunks(j).peers(k));
      }
    }
  }
  return 0;
}

int FileHandler::saveStateToFile() {
  shorrent::FileList fileList;
  for (auto& f: this->files) {
    shorrent::File* file_p = fileList.add_files();
    file_p->set_filename(f.filename);
    file_p->set_size(f.size);
    for (auto& ch: f.chunks) {
      shorrent::Chunk* chunk_p = file_p->add_chunks();
      chunk_p->set_id(ch.id);
      chunk_p->set_filename(ch.filename);
      chunk_p->set_path(ch.path);
      chunk_p->set_size(ch.size);
      chunk_p->set_state(static_cast<shorrent::Chunk_ChunkState>(ch.size));

      for(auto& p: ch.peers) {
        chunk_p->add_peers(p);
      }
    }
  }
  std::ofstream out("state.bin");
  fileList.SerializeToOstream(&out);
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




