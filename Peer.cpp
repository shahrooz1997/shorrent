//
// Created by shahrooz on 10/5/21.
//

#include "Peer.h"
#include <thread>
#include <future>
#include <chrono>
#include <algorithm>
#include <random>
#include "DataSerialization.h"

FileHandler Peer::fh(true);

Peer::Peer(const std::string &address) : address(address), sock(-1) {}

void Peer::start() {
  struct sockaddr_in address;
//  int opt = 1;
//  int addrlen = sizeof(address);

  // Creating socket file descriptor
  if ((this->sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

//  // Forcefully attaching socket to the port 8080
//  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
//                 &opt, sizeof(opt)))
//  {
//    perror("setsockopt");
//    exit(EXIT_FAILURE);
//  }

  int portInt = std::stoi(this->address.substr(this->address.find(':') + 1));
  uint16_t port = 0;
  if (portInt <= static_cast<int>(UINT16_MAX) && portInt >=0) {
    port = static_cast<uint16_t>(portInt);
  } else {
    perror("Bad port number");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_port = htons(port);

  if(inet_pton(AF_INET, this->address.substr(0, this->address.find(':')).c_str(), &(address.sin_addr))<=0)
  {
    printf("\nInvalid address/ Address not supported \n");
    exit(EXIT_FAILURE);
  }

  // Forcefully attaching socket to the port 8080
  if (bind(this->sock, (struct sockaddr *)&address,
           sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  if (listen(this->sock, BACKLOG_SIZE) < 0)
  {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  DPRINTF(true, "Peer is running on port %hu\n", port);

  while(1){
    int new_sock = accept(this->sock, NULL, NULL);
    if(new_sock < 0){
      DPRINTF(true, "Error: accept: %d, errno is %d\n", new_sock, errno);
    }
    else{
      DPRINTF(true, "New connection created on fd %d\n", new_sock);
      std::thread ([new_sock, this](){ this->message_handle(new_sock); }).detach();
    }
  }
}

void Peer::message_handle(int sock) {
  std::string recvd;
  int ret = recvData(sock, recvd);
  if(ret != 0){
    close(sock);
    printf("one connection closed.\n");
    return;
  }
  std::unique_ptr<shorrent::Operation::Type> op_p;
  std::unique_ptr<std::string> data_p;
  std::unique_ptr<std::string> msg_p;
  DataSerialization::deserializeOperation(recvd, op_p, data_p, msg_p);
  switch (*op_p) {
    case shorrent::Operation_Type::Operation_Type_getChunk: {
      std::unique_ptr<std::string> filename_p;
      std::unique_ptr<uint32_t> id_p;
      DataSerialization::deserializeRegChunk(*data_p, filename_p, id_p);
      std::string data;
      Peer::fh.getChunk(*filename_p, *id_p, data);

      // Send back data.
      sendData(sock, DataSerialization::serializeToData(data));
      break;
    }
    case shorrent::Operation_Type::Operation_Type_ok: {
      DPRINTF(true, "OK received.\n");
      break;
    }
    default: {
      DPRINTF(true, "Operation not found: %d\n", *op_p);
    }
  }
  close(sock);
}

int Peer::registerFile(const std::string& filename) {
  // Divide file into chunks
  std::ifstream inFile;
  inFile.open(std::string(FILES_PATH) + filename);
  if (!inFile.is_open()) {
    return -1;
  }

  char buffer[CHUNK_DEFAULT_SIZE];
  uint32_t chunkCounter = 0;
  while(inFile.read(buffer, CHUNK_DEFAULT_SIZE)) {
    std::string chunkFilename = filename + "!!!" + std::to_string(chunkCounter);
    std::ofstream outFile(std::string(CHUNKS_PATH) + chunkFilename);
    if (!outFile.is_open()) {
      return -1;
    }
    outFile.write(buffer, CHUNK_DEFAULT_SIZE);
    chunkCounter++;
    outFile.close();
  }
  std::string chunkFilename = filename + "!!!" + std::to_string(chunkCounter);
  std::ofstream outFile(std::string(CHUNKS_PATH) + chunkFilename);
  std::streamsize lastReadSize = inFile.gcount();
  outFile.write(buffer, lastReadSize);
  outFile.close();
  inFile.close();
  uint32_t fileSize = chunkCounter * (CHUNK_DEFAULT_SIZE) + static_cast<uint32_t>(lastReadSize);

  // Send regFile operation to the server
  int sock = Peer::connectToServer();
  if (sock < 0) {
    return -1;
  }
  std::string operationData = DataSerialization::serializeToRegFile(this->address,
                                    std::vector<File>(1, File(filename, fileSize)));
  sendData(sock, DataSerialization::serializeToOperation(shorrent::Operation_Type::Operation_Type_regFile,
                                                         operationData, ""));
  std::string recvd;
  recvData(sock, recvd);
  close(sock);
  std::unique_ptr<shorrent::Operation::Type> op_p;
  DataSerialization::deserializeOperation(recvd, op_p);
  if (*op_p != shorrent::Operation_Type::Operation_Type_ok) {
    return -1;
  }
  return 0;
}

int Peer::fileList(std::vector<File>& files) {
  int sock = Peer::connectToServer();
  if (sock < 0) {
    return -1;
  }
  sendData(sock, DataSerialization::serializeToOperation(shorrent::Operation_Type::Operation_Type_fileList));
  std::string recvd;
  recvData(sock, recvd);
  close(sock);
  std::unique_ptr<std::vector<File>> files_p;
  DataSerialization::deserializeFileList(recvd, files_p);
  files = std::move(*files_p);
  return 0;
}

int Peer::downloadChunk(const std::vector<std::string> peers, const std::string filename, uint32_t id) {
  // Check if the peer has already the chunk.
  if (fileExist(std::string(CHUNKS_PATH) + filename + "!!!" + std::to_string(id))) {
    return 0;
  }

  int ret = -1;
  for (auto& peer: peers) {
    ret = this->getChunk(peer, filename, id);
    if (ret == 0) {
      break;
    }
  }
  if (ret != 0) {
    return ret;
  }
  ret = this->registerChunk(filename, id);
  return ret;
}

void showProgressBar(float progress) {
  int barWidth = PROGRESS_BAR_WIDTH;
  std::cout << "[";
  int pos = static_cast<int>(static_cast<float>(barWidth) * progress);
  for (int i = 0; i < barWidth; i++) {
    if (i < pos) {
      std::cout << "=";
    } else if (i == pos) {
       std::cout << ">";
    } else {
      std::cout << " ";
    }
  }
  std::cout << "] " << int(progress * 100.0) << " %\r";
  std::cout.flush();
}

int Peer::downloadFile(File &file) {
  // Download chunks.
  std::vector<std::future<int>> threads;
  int numDownloadedChunks = 0;
  bool missingChunk = false;
  for (auto& chunk: file.chunks) {
    threads.emplace_back(std::async(std::launch::async, &Peer::downloadChunk, this, chunk.peers,
                                    chunk.filename, chunk.id));
    if (threads.size() >= MAX_NUM_OF_THREADS) {
      bool block = true;
      while(block) {
        for (size_t i = 0; i < threads.size(); i++) {
          if (threads[i].wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) {
            if (threads[i].get() != 0) {
              DPRINTF(true, "Chunk download error");
              missingChunk = true;
            }
            threads.erase(threads.begin() + i);
            i--;
            numDownloadedChunks++;
            showProgressBar(static_cast<float>(numDownloadedChunks) / static_cast<float>(file.chunks.size()));
            block = false;
          }
        }
      }
    }
  }
  for (auto& t: threads) {
    if (t.get() != 0) {
      DPRINTF(true, "Chunk download error");
      missingChunk = true;
    }
    numDownloadedChunks++;
    showProgressBar(static_cast<float>(numDownloadedChunks) / static_cast<float>(file.chunks.size()));
  }
  std::cout << std::endl;

  if (missingChunk) {
    DPRINTF(true, "Cannot combine the chunks to create the file because at least one chunk is missing.\n");
    return -1;
  }

  // Combine chunks together.
  std::ofstream outFile(std::string(FILES_PATH) + file.filename);
  char buffer[CHUNK_DEFAULT_SIZE];
  uint32_t chunkCounter = 0;
  uint32_t readSize = 0;
  while(readSize < file.size) {
    std::string chunkFilename = file.filename + "!!!" + std::to_string(chunkCounter);
    std::ifstream inFile(std::string(CHUNKS_PATH) + chunkFilename);
    inFile.read(buffer, CHUNK_DEFAULT_SIZE);
    outFile.write(buffer, inFile.gcount());
    readSize += static_cast<uint32_t>(inFile.gcount());
    inFile.close();
    chunkCounter++;
  }
  outFile.close();
  return 0;
}

int Peer::getFileInfo(const std::string &filename, File &fileInfo) {
  int sock = Peer::connectToServer();
  if (sock < 0) {
    return -1;
  }
  std::string operationData = DataSerialization::serializeToFile(File(filename));
  sendData(sock, DataSerialization::serializeToOperation(shorrent::Operation_Type::Operation_Type_getFileInfo,
                                                           operationData, ""));
  std::string recvd;
  recvData(sock, recvd);
  close(sock);
  std::unique_ptr<File> file_p;
  DataSerialization::deserializeFile(recvd, file_p);
  fileInfo = std::move(*file_p);
  // Put the rarest chunks to the top.
  std::sort(fileInfo.chunks.begin(), fileInfo.chunks.end(),
            [](Chunk& ch1, Chunk& ch2){ return ch1.peers.size() < ch2.peers.size(); });
  // Shuffle available peers.
  DPRINTF(true, "AAA %lu\n", fileInfo.chunks[0].peers.size());
  std::srand(static_cast<unsigned int>((std::time(nullptr))));
  for (auto& ch: fileInfo.chunks) {
    random_shuffle(ch.peers.begin(), ch.peers.end());
  }
  return 0;
}

int Peer::registerChunk(const std::string &filename, uint32_t id) {
  int sock = Peer::connectToServer();
  if (sock < 0) {
    return -1;
  }
  std::string operationData = DataSerialization::serializeToRegChunk(this->address, filename, id);
  sendData(sock, DataSerialization::serializeToOperation(shorrent::Operation_Type::Operation_Type_regChunk,
                                                         operationData, ""));
  std::string recvd;
  recvData(sock, recvd);
  close(sock);
  std::unique_ptr<shorrent::Operation::Type> op_p;
  DataSerialization::deserializeOperation(recvd, op_p);
  if (*op_p != shorrent::Operation_Type::Operation_Type_ok) {
    return -1;
  }
  return 0;
}

int Peer::getChunk(const std::string& address, const std::string &filename, uint32_t id) {
  int sock = Peer::connectTo(address);
  if (sock < 0) {
    return -1;
  }
  std::string operationData = DataSerialization::serializeToRegChunk(filename, id);
  sendData(sock, DataSerialization::serializeToOperation(shorrent::Operation_Type::Operation_Type_getChunk,
                                                         operationData, ""));
  std::string recvd;
  recvData(sock, recvd);
  close(sock);
  std::unique_ptr<std::string> data_p;
  DataSerialization::deserializeData(recvd, data_p);
  std::string chunkFilename = filename + "!!!" + std::to_string(id);
  std::ofstream outFile(std::string(CHUNKS_PATH) + chunkFilename);
  outFile.write(data_p->c_str(), static_cast<std::streamsize>(data_p->size()));
  outFile.close();
  return 0;
}

int Peer::connectToServer() {
  struct sockaddr_in serv_addr;
  serv_addr.sin_addr.s_addr = SERVER_IP;

  std::string address(inet_ntoa(serv_addr.sin_addr));
  address += ":";
  address += std::to_string(SERVER_PORT);

  return Peer::connectTo(address);
}

int Peer::connectTo(const std::string &address) {
  int sock = 0;
  struct sockaddr_in serv_addr;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  int portInt = std::stoi(address.substr(address.find(':') + 1));
  uint16_t port = 0;
  if (portInt <= static_cast<int>(UINT16_MAX) && portInt >=0) {
    port = static_cast<uint16_t>(portInt);
  } else {
    perror("Bad port number");
    exit(EXIT_FAILURE);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  if(inet_pton(AF_INET, address.substr(0, address.find(':')).c_str(), &(serv_addr.sin_addr))<=0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }

  return sock;
}
