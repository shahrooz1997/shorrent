//
// Created by shahrooz on 10/3/21.
//

#include "File.h"
File::File(std::string filename, uint32_t size) {
  this->filename = filename;
  this->size = size;
}

File::File(std::string filename, uint32_t size, std::string address) : File(filename, size) {
  uint32_t sizeTemp = size;
  uint32_t idCounter = 0;
  while(sizeTemp > CHUNK_DEFAULT_SIZE_MB) {
    this->chunks.emplace_back(idCounter++, filename, "", CHUNK_DEFAULT_SIZE_MB, ChunkState::done);
    this->chunks.back().add_peer(address);
    sizeTemp -= CHUNK_DEFAULT_SIZE_MB;
  }
  this->chunks.emplace_back(idCounter++, filename, "", sizeTemp, ChunkState::done);
  this->chunks.back().add_peer(address);
}
