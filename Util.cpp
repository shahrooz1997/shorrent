//
// Created by shahrooz on 10/3/21.
//

#include "Util.h"
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

int recvData(int sock, void *buf, size_t size) {
  char *iter = static_cast<char *>(buf);
  ssize_t bytes_read = 0;
  while (size > 0) {
    if ((bytes_read = recv(sock, iter, size, 0)) < 1) {
      if (bytes_read == -1) {
        printf("Error: host socket, errno is %d\n", errno);
        return -1;
      } else if (bytes_read == 0) {
        printf("Warn: remote socket closed\n");
      }
      return 0;
    }
    iter += bytes_read;
    size -= bytes_read;
  }
  return 0;
}

int recvData(int sock, std::string &data) {
  uint32_t size = 0;
  int result;
  data.clear();
  result = recvData(sock, &size, sizeof(size));
  if (result == 0) {
    size = ntohl(size);
    if (size > 0) {
      char *buffer = new char[size];
      result = recvData(sock, buffer, size);
      if (result == 0) {
        data = std::string(buffer, size);
        delete[] buffer;
      } else {
        delete[] buffer;
        return result;
      }
    }
  }
  if (data.empty()) {
    return -2;
  }
  return 0;
}

int sendData(int sock, const void *data, size_t size) {
  const char *iter = static_cast<const char *>(data);
  ssize_t bytes_sent = 0;
  while (size > 0) {
    if ((bytes_sent = send(sock, iter, size, 0)) < 1) {
      if (bytes_sent == -1) {
        printf("sendData Error: host socket, errno is %d\n", errno);
        return -1;
      } else if (bytes_sent == 0) {
        printf("sendData Warn: remote socket closed\n");
      }
      return 0;
    }
    iter += bytes_sent;
    size -= bytes_sent;
  }
  return 0;
}

int sendData(int sock, const std::string &data) {
  uint32_t size = htonl(static_cast<uint32_t>(data.size()));
  int result = sendData(sock, &size, sizeof(size));
  if (result == 0) {
    result = sendData(sock, data.c_str(), data.size());
  }
  return result;
}

void listDir(const std::string &path, std::vector<std::string> &files) {
  struct dirent *entry;
  DIR *dir = opendir(path.c_str());

  if (dir == NULL) {
    return;
  }
  while ((entry = readdir(dir)) != NULL) {
    files.push_back(entry->d_name);
    if (files.back() == "." || files.back() == ".." || files.back().find("!!!") == std::string::npos) {
      files.pop_back();
    }
  }
  closedir(dir);
}

void listDir2(const std::string &path, std::vector<std::string> &files) {
  struct dirent *entry;
  DIR *dir = opendir(path.c_str());

  if (dir == NULL) {
    return;
  }
  while ((entry = readdir(dir)) != NULL) {
    files.push_back(entry->d_name);
    if (files.back() == "." || files.back() == ".." || files.back() == ".gitignore") {
      files.pop_back();
    }
  }
  closedir(dir);
}

uint32_t fileSize(const std::string &path) {
  FILE *pFile;
  pFile = fopen(path.c_str(), "rb");
  fseek(pFile, 0L, SEEK_END);
  uint32_t ret = static_cast<uint32_t>(ftell(pFile));
  fclose(pFile);
  return ret;
}
bool fileExist(const std::string &path) {
  return access(path.c_str(), F_OK) == 0;
}

bool isDir(const std::string &path) {
//  std::string standard_path = path;
//  DPRINTF(true, "AAAA %s, %lu, %lu\n", standard_path.c_str(), standard_path.rfind('/'), standard_path.size());
//  if (standard_path.rfind('/') == standard_path.size() - 1) {
//    DPRINTF(true, "SSSSS\n");
//    DPRINTF(true, "AAA %s\n", standard_path.c_str());
//    standard_path = standard_path.substr(0, standard_path.size() -1);
//    DPRINTF(true, "AAA2 %s\n", standard_path.c_str());
//  }
  struct stat stat_buf;
  if (stat(path.c_str(), &stat_buf) == 0) {
    return S_ISDIR(stat_buf.st_mode) != 0;
  } else {
    DPRINTF(true, "Error in stat file %s. Errno is %s\n", path.c_str(), strerror(errno));
  }
  return false;
}

int copyDir(const std::string &src, const std::string &dest) {
  if (!isDir(dest)) {
    DPRINTF(true, "dest \"%s\" is not a directory.\n", dest.c_str());
    return -1;
  }
  std::string command = "cp -r ";
  command += src + "/*";
  command += " ";
  command += dest;
  system(command.c_str());
  return 0;
}

int copyFile(const std::string &src, const std::string &dest) {
  if (!isDir(dest)) {
    DPRINTF(true, "dest \"%s\" is not a directory.\n", dest.c_str());
    return -1;
  }
  std::string command = "cp ";
  command += src;
  command += " ";
  command += dest;
  system(command.c_str());
  return 0;
}
