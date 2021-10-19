//
// Created by shahrooz on 10/18/21.
//

#include "DataSerialization.h"

std::string DataSerialization::serializeToChunk(const Chunk &chunk) {
    shorrent::Chunk s_chunk;
    s_chunk.set_id(chunk.id);
    s_chunk.set_filename(chunk.filename);
    s_chunk.set_path(chunk.path);
    s_chunk.set_size(chunk.size);
    s_chunk.set_state(static_cast<shorrent::Chunk_ChunkState>(chunk.state));
    for (auto& p: chunk.peers) {
        s_chunk.add_peers(p);
    }
    std::string str_out;
    s_chunk.SerializeToString(&str_out);
    return str_out;
}

std::string DataSerialization::serializeToFile(const File &file) {
    shorrent::File s_file;
    s_file.set_filename(file.filename);
    s_file.set_size(file.size);
    for (auto& ch: file.chunks) {
        shorrent::Chunk* chunk_p = s_file.add_chunks();
        DataSerialization::serializeToChunk(ch, *chunk_p);
    }
    std::string str_out;
    s_file.SerializeToString(&str_out);
    return str_out;
}

std::string DataSerialization::serializeToRegFile(const std::string &address, const std::vector<File> &files) {
    shorrent::RegFile regFile;
    regFile.set_address(address);
    for (auto& file: files) {
        shorrent::File* file_p = regFile.add_files();
        DataSerialization::serializeToFile(file, *file_p);
    }
    std::string str_out;
    regFile.SerializeToString(&str_out);
    return str_out;
}

std::string DataSerialization::serializeToFileList(const std::vector<File> &files) {
    shorrent::FileList fileList;
    for (auto& file: files) {
        shorrent::File* file_p = fileList.add_files();
        DataSerialization::serializeToFile(file, *file_p);
    }
    std::string str_out;
    fileList.SerializeToString(&str_out);
    return str_out;
}

std::string DataSerialization::serializeToRegChunk(const std::string &filename, uint32_t id) {
    return DataSerialization::serializeToRegChunk("", filename, id);
}

std::string DataSerialization::serializeToRegChunk(const std::string &address, const std::string &filename,
                                                   uint32_t id) {
    shorrent::RegChunk regChunk;
    regChunk.set_address(address);
    regChunk.set_filename(filename);
    regChunk.set_id(id);
    std::string str_out;
    regChunk.SerializeToString(&str_out);
    return str_out;
}

std::string DataSerialization::serializeToData(const std::string &data) {
    shorrent::Data s_data;
    s_data.set_data(data);
    std::string str_out;
    s_data.SerializeToString(&str_out);
    return str_out;
}

std::string DataSerialization::serializeToOperation(const shorrent::Operation::Type &op) {
    shorrent::Operation operation;
    operation.set_op(op);
    std::string str_out;
    operation.SerializeToString(&str_out);
    return str_out;
}

std::string DataSerialization::serializeToOperation(const shorrent::Operation::Type &op, const std::string &data,
                                            const std::string &msg) {
    shorrent::Operation operation;
    operation.set_op(op);
    operation.set_data(data);
    operation.set_msg(msg);
    std::string str_out;
    operation.SerializeToString(&str_out);
    return str_out;
}

int DataSerialization::deserializeChunk(const std::string &str_in, std::unique_ptr<Chunk> &chunk_p) {
    shorrent::Chunk s_chunk;
    s_chunk.ParseFromString(str_in);
    chunk_p.reset(new Chunk(s_chunk.id(), s_chunk.filename(), s_chunk.path(), s_chunk.size(),
                            static_cast<ChunkState>(s_chunk.state())));
    for (int j = 0; j < s_chunk.peers_size(); j++) {
        chunk_p->add_peer(s_chunk.peers(j));
    }
    return 0;
}

int DataSerialization::deserializeFile(const std::string &str_in, std::unique_ptr<File> &file_p) {
    shorrent::File s_file;
    s_file.ParseFromString(str_in);
    file_p.reset(new File(s_file.filename(), s_file.size()));
    for (int i = 0; i < s_file.chunks_size(); i++) {
        std::unique_ptr<Chunk> chunk_p;
        DataSerialization::deserializeChunk(s_file.chunks(i), chunk_p);
        assert(chunk_p);
        file_p->chunks.push_back(*chunk_p);
    }
    return 0;
}

int DataSerialization::deserializeRegFile(const std::string &str_in, std::unique_ptr<std::string> &address_p,
                                          std::unique_ptr<std::vector<File>> &files_p) {
    shorrent::RegFile regFile;
    regFile.ParseFromString(str_in);
    address_p.reset(new std::string(regFile.address()));
    files_p.reset(new std::vector<File>());
    for (int i = 0; i < regFile.files_size(); i++) {
        std::unique_ptr<File> file_p;
        DataSerialization::deserializeFile(regFile.files(i), file_p);
        files_p->push_back(*file_p);
    }
    return 0;
}

int DataSerialization::deserializeFileList(const std::string &str_in,
                                           std::unique_ptr<std::vector<File>> &files_p) {
    shorrent::FileList fileList;
    fileList.ParseFromString(str_in);
    files_p.reset(new std::vector<File>());
    for (int i = 0; i < fileList.files_size(); i++) {
        std::unique_ptr<File> file_p;
        DataSerialization::deserializeFile(fileList.files(i), file_p);
        files_p->push_back(*file_p);
    }
    return 0;
}

int DataSerialization::deserializeRegChunk(const std::string &str_in, std::unique_ptr<std::string> &filename_p,
                        std::unique_ptr<uint32_t> &id_p) {
    shorrent::RegChunk regFile;
    regFile.ParseFromString(str_in);
    filename_p.reset(new std::string(regFile.filename()));
    id_p.reset(new uint32_t(regFile.id()));
    return 0;
}

int DataSerialization::deserializeRegChunk(const std::string &str_in, std::unique_ptr<std::string> &address_p,
                                           std::unique_ptr<std::string> &filename_p, std::unique_ptr<uint32_t> &id_p) {
    shorrent::RegChunk regFile;
    regFile.ParseFromString(str_in);
    address_p.reset(new std::string(regFile.address()));
    filename_p.reset(new std::string(regFile.filename()));
    id_p.reset(new uint32_t(regFile.id()));
    return 0;
}

int DataSerialization::deserializeData(const std::string &str_in, std::unique_ptr<std::string> &data_p) {
    shorrent::Data s_data;
    s_data.ParseFromString(str_in);
    data_p.reset(new std::string(s_data.data()));
    return 0;
}

int DataSerialization::deserializeOperation(const std::string &str_in,
                                std::unique_ptr<shorrent::Operation::Type> &op_p) {
    shorrent::Operation operation;
    operation.ParseFromString(str_in);
    op_p.reset(new shorrent::Operation::Type(operation.op()));
    return 0;
}

int DataSerialization::deserializeOperation(const std::string &str_in,
                                            std::unique_ptr<shorrent::Operation::Type> &op_p,
                                            std::unique_ptr<std::string> &data_p, std::unique_ptr<std::string> &msg_p) {
    shorrent::Operation operation;
    operation.ParseFromString(str_in);
    op_p.reset(new shorrent::Operation::Type(operation.op()));
    data_p.reset(new std::string(operation.data()));
    msg_p.reset(new std::string(operation.msg()));
    return 0;
}

int DataSerialization::serializeToChunk(const Chunk &chunk, shorrent::Chunk &s_chunk) {
    s_chunk.set_id(chunk.id);
    s_chunk.set_filename(chunk.filename);
    s_chunk.set_path(chunk.path);
    s_chunk.set_size(chunk.size);
    s_chunk.set_state(static_cast<shorrent::Chunk_ChunkState>(chunk.state));
    for (auto& p: chunk.peers) {
        s_chunk.add_peers(p);
    }
    return 0;
}

int DataSerialization::serializeToFile(const File &file, shorrent::File &s_file) {
    s_file.set_filename(file.filename);
    s_file.set_size(file.size);
    for (auto& ch: file.chunks) {
        shorrent::Chunk* chunk_p = s_file.add_chunks();
        DataSerialization::serializeToChunk(ch, *chunk_p);
    }

    return 0;
}

int DataSerialization::serializeToRegFile(const std::string &address, const std::vector<File> &files,
                                          shorrent::RegFile &regFile) {
    regFile.set_address(address);
    for (auto& file: files) {
        shorrent::File* file_p = regFile.add_files();
        DataSerialization::serializeToFile(file, *file_p);
    }
    return 0;
}

int DataSerialization::serializeToFileList(const std::vector<File> &files, shorrent::FileList &fileList) {
    for (auto& file: files) {
        shorrent::File* file_p = fileList.add_files();
        DataSerialization::serializeToFile(file, *file_p);
    }
    return 0;
}

int DataSerialization::serializeToRegChunk(const std::string &address, const std::string &filename, uint32_t id,
                                           shorrent::RegChunk &regChunk) {
    regChunk.set_address(address);
    regChunk.set_filename(filename);
    regChunk.set_id(id);
    return 0;
}

int DataSerialization::serializeToData(const std::string &data, shorrent::Data &s_data) {
    s_data.set_data(data);
    return 0;
}

int DataSerialization::serializeToOperation(const shorrent::Operation::Type &op, const std::string &data,
                                            const std::string &msg, shorrent::Operation &operation) {
    operation.set_op(op);
    operation.set_data(data);
    operation.set_msg(msg);
    return 0;
}

int DataSerialization::deserializeChunk(const shorrent::Chunk &s_chunk, std::unique_ptr<Chunk> &chunk_p) {
    chunk_p.reset(new Chunk(s_chunk.id(), s_chunk.filename(), s_chunk.path(), s_chunk.size(),
                            static_cast<ChunkState>(s_chunk.state())));
    for (int j = 0; j < s_chunk.peers_size(); j++) {
        chunk_p->add_peer(s_chunk.peers(j));
    }
    return 0;
}

int DataSerialization::deserializeFile(const shorrent::File &s_file, std::unique_ptr<File> &file_p) {
    file_p.reset(new File(s_file.filename(), s_file.size()));
    for (int i = 0; i < s_file.chunks_size(); i++) {
        std::unique_ptr<Chunk> chunk_p;
        DataSerialization::deserializeChunk(s_file.chunks(i), chunk_p);
        assert(chunk_p);
        file_p->chunks.push_back(*chunk_p);
    }
    return 0;
}

int DataSerialization::deserializeRegFile(const shorrent::RegFile &regFile, std::unique_ptr<std::string> &address_p,
                                          std::unique_ptr<std::vector<File>> &files_p) {
    address_p.reset(new std::string(regFile.address()));
    files_p.reset(new std::vector<File>());
    for (int i = 0; i < regFile.files_size(); i++) {
        std::unique_ptr<File> file_p;
        DataSerialization::deserializeFile(regFile.files(i), file_p);
        files_p->push_back(*file_p);
    }
    return 0;
}

int DataSerialization::deserializeFileList(const shorrent::FileList &fileList,
                                           std::unique_ptr<std::vector<File>> &files_p) {
    files_p.reset(new std::vector<File>());
    for (int i = 0; i < fileList.files_size(); i++) {
        std::unique_ptr<File> file_p;
        DataSerialization::deserializeFile(fileList.files(i), file_p);
        files_p->push_back(*file_p);
    }
    return 0;
}

int DataSerialization::deserializeRegChunk(const shorrent::RegChunk &regFile, std::unique_ptr<std::string> &address_p,
                                           std::unique_ptr<std::string> &filename_p, std::unique_ptr<uint32_t> &id_p) {
    address_p.reset(new std::string(regFile.address()));
    filename_p.reset(new std::string(regFile.filename()));
    id_p.reset(new uint32_t(regFile.id()));
    return 0;
}

int DataSerialization::deserializeData(const shorrent::Data &s_data, std::unique_ptr<std::string> &data_p) {
    data_p.reset(new std::string(s_data.data()));
    return 0;
}

int DataSerialization::deserializeOperation(const shorrent::Operation &operation,
                                            std::unique_ptr<shorrent::Operation::Type> &op_p,
                                            std::unique_ptr<std::string> &data_p, std::unique_ptr<std::string> &msg_p) {
    op_p.reset(new shorrent::Operation::Type(operation.op()));
    data_p.reset(new std::string(operation.data()));
    msg_p.reset(new std::string(operation.msg()));
    return 0;
}
