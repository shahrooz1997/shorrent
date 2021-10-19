//
// Created by shahrooz on 10/3/21.
//

#include "Chunk.h"

Chunk::Chunk(uint32_t id, const std::string& filename, const std::string& path, uint32_t size, const ChunkState state) {
  this->id = id;
  this->filename = filename;
  this->path = path;
  this->size = size;
  this->state = state;
}

int Chunk::add_peer(const std::string& address) {
  this->peers.emplace_back(address);
  return 0;
}
