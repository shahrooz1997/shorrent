//
// Created by shahrooz on 10/4/21.
//

#include "Server.h"
#include "DataSerialization.h"

int Server::sock = -1;
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
  address.sin_port = htons(SERVER_PORT);
  address.sin_addr.s_addr = SERVER_IP;

  // Forcefully attaching socket to the port
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

  DPRINTF(true, "Server is running on port %hu\n", SERVER_PORT);

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
  std::unique_ptr<shorrent::Operation::Type> op_p;
  std::unique_ptr<std::string> data_p;
  std::unique_ptr<std::string> msg_p;
  DataSerialization::deserializeOperation(recvd, op_p, data_p, msg_p);
  switch (*op_p) {
    case shorrent::Operation_Type::Operation_Type_regFile: {
      std::unique_ptr<std::string> address_p;
      std::unique_ptr<std::vector<File>> files_p;
      DataSerialization::deserializeRegFile(*data_p, address_p, files_p);
      if (Server::fh.registerFiles(*address_p, *files_p) != 0) {
        DPRINTF(true, "Server::fh.registerFiles error");
      }
      // Send back success.
      sendData(sock, DataSerialization::serializeToOperation(shorrent::Operation_Type::Operation_Type_ok));
      break;
    }
    case shorrent::Operation_Type::Operation_Type_fileList: {
      std::vector<File> files;
      if (Server::fh.fileList(files) != 0) {
        DPRINTF(true, "Server::fh.fileList error");
      }
      sendData(sock, DataSerialization::serializeToFileList(files));
      break;
    }
    case shorrent::Operation_Type::Operation_Type_getFileInfo: {
      std::unique_ptr<File> file_p;
      DataSerialization::deserializeFile(*data_p, file_p);
      if (Server::fh.getFileInfo(file_p->filename, *file_p) != 0) {
        DPRINTF(true, "Server::fh.getFileInfo error");
      }
      // Send back data.
      sendData(sock, DataSerialization::serializeToFile(*file_p));
      break;
    }
    case shorrent::Operation_Type::Operation_Type_regChunk: {
      std::unique_ptr<std::string> address_p;
      std::unique_ptr<std::string> filename_p;
      std::unique_ptr<uint32_t> id_p;
      DataSerialization::deserializeRegChunk(*data_p, address_p, filename_p, id_p);
      if (Server::fh.registerChunk(*address_p, *filename_p, *id_p) != 0) {
        DPRINTF(true, "Server::fh.registerChunk");
      }
      // Send back success.
      sendData(sock, DataSerialization::serializeToOperation(shorrent::Operation_Type::Operation_Type_ok));
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
