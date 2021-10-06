//
// Created by shahrooz on 10/4/21.
//

#include "Server.h"
#include "gbuffer.pb.h"

int Server::sock;
FileHandler Server::fh;

void Server::start() {
  struct sockaddr_in address;
//  int opt = 1;
//  int addrlen = sizeof(address);

  // Creating socket file descriptor
  if ((Server::sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
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

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = SERVER_IP;
  address.sin_port = htons(SERVER_PORT);
  // Forcefully attaching socket to the port 8080
  if (bind(Server::sock, (struct sockaddr *)&address,
           sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  if (listen(Server::sock, BACKLOG_SIZE) < 0)
  {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  DPRINTF(true, "Server is running on port %hu", SERVER_PORT);

  while(1){
    int new_sock = accept(Server::sock, NULL, NULL);
    if(new_sock < 0){
      DPRINTF(true, "Error: accept: %d, errno is %d\n", new_sock, errno);
    }
    else{
      DPRINTF(true, "New connection created on fd %d\n", new_sock);
      std::thread ([new_sock](){ Server::message_handle(new_sock); }).detach();
    }
  }
}

void Server::message_handle(int sock) {
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
    case shorrent::Operation_Type::Operation_Type_regFile: {
      shorrent::RegFile regFile;
      regFile.ParseFromString(operation.data());
      std::vector<File> files;
      for (int i = 0; i < regFile.files_size(); i++) {
        files.emplace_back(regFile.files(i).filename(), regFile.files(i).size());
      }
      Server::fh.registerFiles(regFile.address(), files);

      // Send back success.
      shorrent::Operation replyOperation;
      replyOperation.set_op(shorrent::Operation_Type::Operation_Type_ok);
      std::string tempStr;
      replyOperation.SerializeToString(&tempStr);
      sendData(sock, tempStr);
      break;
    }
    case shorrent::Operation_Type::Operation_Type_fileList: {
      std::vector<File> files;
      Server::fh.fileList(files);
      shorrent::FileList fileList;
      for (auto f: files) {
        shorrent::File* file_p = fileList.add_files();
        file_p->set_filename(f.filename);
        file_p->set_size(f.size);
      }
      std::string tempStr;
      fileList.SerializeToString(&tempStr);
      sendData(sock, tempStr);
      break;
    }
    case shorrent::Operation_Type::Operation_Type_getFileInfo: {
      shorrent::File file;
      file.ParseFromString(operation.data());
      File fileInfo;
      Server::fh.getFileInfo(file.filename(), fileInfo);

      // Send back data.
      shorrent::File replyFile;
      replyFile.set_filename(fileInfo.filename);
      replyFile.set_size(fileInfo.size);
      for (auto ch: fileInfo.chunks) {
        shorrent::Chunk* chunk_p = replyFile.add_chunks();
        chunk_p->set_id(ch.id);
        chunk_p->set_filename(ch.filename);
        chunk_p->set_path(ch.path);
        chunk_p->set_size(ch.size);
        chunk_p->set_state(static_cast<shorrent::Chunk_ChunkState>(ch.state));
        for (auto p: ch.peers) {
          chunk_p->add_peers(p);
        }
      }
      std::string tempStr;
      replyFile.SerializeToString(&tempStr);
      sendData(sock, tempStr);
      break;
    }
    case shorrent::Operation_Type::Operation_Type_regChunk: {
      shorrent::RegChunk regChunk;
      regChunk.ParseFromString(operation.data());
      Server::fh.registerChunk(regChunk.address(), regChunk.filename(), regChunk.id());

      // Send back success.
      shorrent::Operation replyOperation;
      replyOperation.set_op(shorrent::Operation_Type::Operation_Type_ok);
      std::string tempStr;
      replyOperation.SerializeToString(&tempStr);
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
}

