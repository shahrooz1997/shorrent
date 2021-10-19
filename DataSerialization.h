//
// Created by shahrooz on 10/18/21.
//

#ifndef SHORRENT_DATASERIALIZATION_H
#define SHORRENT_DATASERIALIZATION_H

#include "Util.h"
#include "File.h"
#include "gbuffer.pb.h"

class DataSerialization {
public:
    static std::string serializeToChunk(const Chunk &chunk);

    static std::string serializeToFile(const File &file);

    static std::string serializeToRegFile(const std::string &address, const std::vector<File> &files);

    static std::string serializeToFileList(const std::vector<File> &files);

    static int serializeToFileList(std::ostream *output, const std::vector<File> &files);

    static std::string serializeToRegChunk(const std::string &filename, uint32_t id);

    static std::string serializeToRegChunk(const std::string &address, const std::string &filename, uint32_t id);

    static std::string serializeToData(const std::string &data);

    static std::string serializeToOperation(const shorrent::Operation::Type &op);

    static std::string serializeToOperation(const shorrent::Operation::Type &op, const std::string &data,
                                            const std::string &msg);

    static int deserializeChunk(const std::string &str_in, std::unique_ptr<Chunk> &chunk_p);

    static int deserializeFile(const std::string &str_in, std::unique_ptr<File> &file_p);

    static int deserializeRegFile(const std::string &str_in, std::unique_ptr<std::string> &address_p,
                                  std::unique_ptr<std::vector<File>> &files_p);

    static int deserializeFileList(const std::string &str_in, std::unique_ptr<std::vector<File>> &files_p);

    static int deserializeFileList(std::istream *input, std::unique_ptr<std::vector<File>> &files_p);

    static int deserializeRegChunk(const std::string &str_in, std::unique_ptr<std::string> &filename_p,
                                   std::unique_ptr<uint32_t> &id_p);

    static int deserializeRegChunk(const std::string &str_in, std::unique_ptr<std::string> &address_p,
                                   std::unique_ptr<std::string> &filename_p, std::unique_ptr<uint32_t> &id_p);

    static int deserializeData(const std::string &str_in, std::unique_ptr<std::string> &data_p);

    static int deserializeOperation(const std::string &str_in,
                                    std::unique_ptr<shorrent::Operation::Type> &op_p);

    static int deserializeOperation(const std::string &str_in,
                                    std::unique_ptr<shorrent::Operation::Type> &op_p,
                                    std::unique_ptr<std::string> &data_p, std::unique_ptr<std::string> &msg_p);


    static int serializeToChunk(const Chunk &chunk, shorrent::Chunk &s_chunk);

    static int serializeToFile(const File &file, shorrent::File &s_file);

    static int serializeToRegFile(const std::string &address, const std::vector<File> &files,
                                  shorrent::RegFile &regFile);

    static int serializeToFileList(const std::vector<File> &files, shorrent::FileList &fileList);

    static int serializeToRegChunk(const std::string &address, const std::string &filename, uint32_t id,
                                   shorrent::RegChunk &regChunk);

    static int serializeToData(const std::string &data, shorrent::Data &s_data);

    static int serializeToOperation(const shorrent::Operation::Type &op, const std::string &data,
                                    const std::string &msg, shorrent::Operation &operation);

    static int deserializeChunk(const shorrent::Chunk &s_chunk, std::unique_ptr<Chunk> &chunk_p);

    static int deserializeFile(const shorrent::File &s_file, std::unique_ptr<File> &file_p);

    static int deserializeRegFile(const shorrent::RegFile &regFile, std::unique_ptr<std::string> &address_p,
                                  std::unique_ptr<std::vector<File>> &files_p);

    static int deserializeFileList(const shorrent::FileList &fileList, std::unique_ptr<std::vector<File>> &files_p);

    static int deserializeRegChunk(const shorrent::RegChunk &regFile, std::unique_ptr<std::string> &address_p,
                                   std::unique_ptr<std::string> &filename_p, std::unique_ptr<uint32_t> &id_p);

    static int deserializeData(const shorrent::Data &s_data, std::unique_ptr<std::string> &data_p);

    static int deserializeOperation(const shorrent::Operation &operation,
                                    std::unique_ptr<shorrent::Operation::Type> &op_p,
                                    std::unique_ptr<std::string> &data_p, std::unique_ptr<std::string> &msg_p);
};


#endif //SHORRENT_DATASERIALIZATION_H
