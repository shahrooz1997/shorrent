//
// Created by shahrooz on 10/5/21.
//

#include "Peer.h"
#include "gbuffer.pb.h"
#include <thread>
#include <future>
#include <chrono>
#include <algorithm>
#include <random>

FileHandler Peer::fh(true);

Peer::Peer(const std::string &address) : address(address) {}

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
  shorrent::Operation operation;
  operation.ParseFromString(recvd);
  switch (operation.op()) {
    case shorrent::Operation_Type::Operation_Type_getChunk: {
      shorrent::RegChunk regChunk;
      regChunk.ParseFromString(operation.data());
      shorrent::Data data;
      Peer::fh.getChunk(regChunk.filename(), regChunk.id(), *(data.mutable_data()));

      // Send back data.
      std::string tempStr;
      data.SerializeToString(&tempStr);
      sendData(sock, tempStr);
      break;
    }
    case shorrent::Operation_Type::Operation_Type_ok: {
      DPRINTF(true, "OK received.\n");
      break;
    }
    default: {
      DPRINTF(true, "Operation not found: %d\n", operation.op());
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
  int sock = this->connectToServer();
  if (sock < 0) {
    return -1;
  }
  shorrent::RegFile regFile;
  regFile.set_address(this->address);
  shorrent::File* file_p = regFile.add_files();
  file_p->set_filename(filename);
  file_p->set_size(fileSize);
  std::string data;
  regFile.SerializeToString(&data);

  shorrent::Operation operation;
  operation.set_op(shorrent::Operation_Type::Operation_Type_regFile);
  operation.set_data(data);

  std::string tempStr;
  operation.SerializeToString(&tempStr);
  sendData(sock, tempStr);
  recvData(sock, tempStr);
  close(sock);
  shorrent::Operation operation2;
  operation2.ParseFromString(tempStr);
  if (operation2.op() != shorrent::Operation_Type::Operation_Type_ok) {
    return -1;
  }
  return 0;
}

int Peer::fileList(std::vector<File>& files) {
  int sock = this->connectToServer();
  if (sock < 0) {
    return -1;
  }
  shorrent::Operation operation;
  operation.set_op(shorrent::Operation_Type::Operation_Type_fileList);
  std::string tempStr;
  operation.SerializeToString(&tempStr);
  sendData(sock, tempStr);
  recvData(sock, tempStr);
  close(sock);
  shorrent::FileList fileList;
  fileList.ParseFromString(tempStr);
  for (int i = 0; i < fileList.files_size(); i++) {
    files.emplace_back(fileList.files(i).filename(), fileList.files(i).size());
  }
  return 0;
}

int Peer::downloadChunk(const std::string address, const std::string filename, uint32_t id) {
  // Check if the peer has already the chunk.
  if (fileExist(std::string(CHUNKS_PATH) + filename + "!!!" + std::to_string(id))) {
    return 0;
  }

  int ret = this->getChunk(address, filename, id);
  if (ret != 0) {
    return ret;
  }
  ret = this->registerChunk(filename, id);
  return ret;
}

void showProgressBar(float progress) {
  int barWidth = 70;
  std::cout << "[";
  int pos = static_cast<int>(static_cast<float>(barWidth) * progress);
  for (int i = 0; i < barWidth; ++i) {
    if (i < pos) std::cout << "=";
    else if (i == pos) std::cout << ">";
    else std::cout << " ";
  }
  std::cout << "] " << int(progress * 100.0) << " %\r";
  std::cout.flush();
}

int Peer::downloadFile(File &file) {
  // Download chunks.
  std::vector<std::future<int>> threads;
  int numDownloadedChunks = 0;
  for (auto& chunk: file.chunks) {
    threads.emplace_back(std::async(std::launch::async, &Peer::downloadChunk, this, chunk.peers[0],
                                    chunk.filename, chunk.id));
    if (threads.size() >= MAX_NUM_OF_THREADS) {
      bool block = true;
      while(block) {
        for (size_t i = 0; i < threads.size(); i++) {
          if (threads[i].wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) {
            if (threads[i].get() != 0) {
              DPRINTF(true, "Chunk download error");
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
    }
    numDownloadedChunks++;
    showProgressBar(static_cast<float>(numDownloadedChunks) / static_cast<float>(file.chunks.size()));
  }
  std::cout << std::endl;

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
  int sock = this->connectToServer();
  if (sock < 0) {
    return -1;
  }
  shorrent::File file;
  file.set_filename(filename);
  std::string dataTmp;
  file.SerializeToString(&dataTmp);

  shorrent::Operation operation;
  operation.set_op(shorrent::Operation_Type::Operation_Type_getFileInfo);
  operation.set_data(dataTmp);
  std::string tempStr;
  operation.SerializeToString(&tempStr);
  sendData(sock, tempStr);
  recvData(sock, tempStr);
  close(sock);
  shorrent::File replyFile;
  replyFile.ParseFromString(tempStr);
  fileInfo.filename = replyFile.filename();
  fileInfo.size = replyFile.size();
  for (int i = 0; i < replyFile.chunks_size(); i++) {
    fileInfo.chunks.emplace_back(replyFile.chunks(i).id(), replyFile.chunks(i).filename(), replyFile.chunks(i).path(),
                                 replyFile.chunks(i).size(), static_cast<ChunkState>(replyFile.chunks(i).state()));
    for (int j = 0; j < replyFile.chunks(i).peers_size(); j++) {
      fileInfo.chunks.back().add_peer(replyFile.chunks(i).peers(j));
    }
  }
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
  int sock = this->connectToServer();
  if (sock < 0) {
    return -1;
  }
  shorrent::RegChunk regChunk;
  regChunk.set_filename(filename);
  regChunk.set_id(id);
  regChunk.set_address(this->address);
  std::string dataTmp;
  regChunk.SerializeToString(&dataTmp);

  shorrent::Operation operation;
  operation.set_op(shorrent::Operation_Type::Operation_Type_regChunk);
  operation.set_data(dataTmp);
  std::string tempStr;
  operation.SerializeToString(&tempStr);
  sendData(sock, tempStr);
  recvData(sock, tempStr);
  close(sock);
  shorrent::Operation operation2;
  operation2.ParseFromString(tempStr);
  if (operation2.op() != shorrent::Operation_Type::Operation_Type_ok) {
    return -1;
  }
  return 0;
}

int Peer::getChunk(const std::string& address, const std::string &filename, uint32_t id) {
  int sock = this->connectTo(address);
  if (sock < 0) {
    return -1;
  }
  shorrent::RegChunk regChunk;
  regChunk.set_filename(filename);
  regChunk.set_id(id);
  std::string dataTmp;
  regChunk.SerializeToString(&dataTmp);

  shorrent::Operation operation;
  operation.set_op(shorrent::Operation_Type::Operation_Type_getChunk);
  operation.set_data(dataTmp);
  std::string tempStr;
  operation.SerializeToString(&tempStr);
  sendData(sock, tempStr);
  recvData(sock, tempStr);
  close(sock);
  shorrent::Data data;
  data.ParseFromString(tempStr);

  std::string chunkFilename = filename + "!!!" + std::to_string(id);
  std::ofstream outFile(std::string(CHUNKS_PATH) + chunkFilename);
  outFile.write(data.data().c_str(), static_cast<std::streamsize>(data.data().size()));
  outFile.close();
  return 0;
}

int Peer::connectToServer() {
  struct sockaddr_in serv_addr;
  serv_addr.sin_addr.s_addr = SERVER_IP;

  std::string address(inet_ntoa(serv_addr.sin_addr));
  address += ":";
  address += std::to_string(SERVER_PORT);

  return this->connectTo(address);
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
