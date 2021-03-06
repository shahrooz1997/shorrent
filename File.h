//
// Created by shahrooz on 10/3/21.
//

#ifndef SHORRENT__FILE_H_
#define SHORRENT__FILE_H_

#include "Chunk.h"
#include <string>

class File {
 public:
  std::string filename;
  uint32_t size;
  std::vector<Chunk> chunks;

  File() = default;
  explicit File(std::string filename, uint32_t size = 0);
  File(std::string filename, uint32_t size, std::string address);

};

#endif //SHORRENT__FILE_H_
