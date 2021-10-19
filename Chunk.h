//
// Created by shahrooz on 10/3/21.
//

#ifndef SHORRENT__CHUNK_H_
#define SHORRENT__CHUNK_H_

#include "Util.h"
#include <string>
#include <vector>

enum ChunkState {
  inTransfer,
  done
};

class Chunk {
 public:
  uint32_t id;
  std::string filename;
  std::string path;
  uint32_t size;
  ChunkState state;

  std::vector<std::string> peers; // string = IP:PORT

  Chunk(uint32_t id, const std::string& filename, const std::string& path, uint32_t size, ChunkState state);
  int add_peer(const std::string& address);
};

#endif //SHORRENT__CHUNK_H_
