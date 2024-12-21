#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include "NameServer.h"
#include "md5.h"
#include "utils.h"

/**
 * @brief: Constructor
 * @param: replicate_num, number of replicas
 * @return: void
 */
NameServer::NameServer(int replicate_num):num_replicate_(replicate_num), cnt_(0){}

/**
 * @brief: Add a DataServer to the NameServer
 * @return: void
 */
void NameServer::add(DataServer *server)
{
    dataServers_.push_back(server);
}

/**
 * @brief: Get the cmd from the user
 * @return: vector of strings
 */ 
std::vector<std::string> NameServer::read_cmd()
{
    std::cout << "DFS command line > ";
    std::string cmd, tmp;
    std::getline(std::cin, cmd);
    std::vector<std::string> params;
    std::stringstream ss(cmd);
    while(ss >> tmp)
        params.push_back(tmp);
    return params;
}

/**
 * @brief: Overload the operator () to run the NameServer
 * @return: void
 */
void NameServer::operator()()
{
    while (true)
    {
        std::vector<std::string> cmd_lines = read_cmd();
        std::vector<int> idx; // Index of the DataServers
        std::ifstream is;
        char* buffer = nullptr;
        // NOTE: CHECK CMD
        if (cmd_lines.empty())
        {
            std::cerr << "[Error] Input blank." << std::endl;
            continue;
        }
        // NOTE: QUIT
        if (cmd_lines[0] == "quit")
        {
            std::cout << "Exiting..." << std::endl;
            exit(0);
        }
        // NOTE: LIST
        else if (cmd_lines[0] == "ls")
        {
            if (cmd_lines.size() != 1)
            {
                std::cerr << "[Error] Usage: ls (list the file system)" << std::endl;
                continue;
            }
            else
            {
                std::cout << "FileName\tFileID\tChunks(2MB)" << std::endl;
                fileSys_.list(file_info);
            }
            continue;
        }
        // NOTE: UPLOAD
        else if (cmd_lines[0] == "upload")
        {
            if (cmd_lines.size() != 3)
            {
                std::cerr << "[Error] Usage: upload <src_file_path> <dst_file_path>" << std::endl;
                continue;
            }
            is.open(cmd_lines[1], std::ifstream::ate | std::ifstream::binary);
            if(!is)
            {
                std::cerr << "[Error] Open file error." << std::endl;
                continue;
            }
            else if (!fileSys_.insert_node(cmd_lines[2], true))
            {
                std::cerr << "[Error] File creation error. File may already exists." << std::endl;
                continue;
            }
            else
            {
                int file_size = is.tellg();
                buffer = new char[file_size];
                is.seekg(0, is.beg);
                is.read(buffer, file_size);
                std::vector<double> ds_server_size;
                for (auto &ds: dataServers_)
                    ds_server_size.push_back(ds->get_size());
                idx = argsort<double>(ds_server_size);
                ++cnt_;
                for (int i = 0; i < 3; ++i)
                {
                    std::unique_lock<std::mutex> lck(dataServers_[idx[i]]->mtx);
                    file_info[cmd_lines[2]] = std::make_pair(cnt_, file_size); // Update the file info (file_name->(file_id, file_size))
                    DataServer *ds = dataServers_[idx[i]];
                    ds->cmd = "upload";
                    ds->id = cnt_;
                    ds->buffer_ = buffer;
                    ds->buffer_size_ = file_size;
                    ds->is_active_ = true;
                    lck.unlock();
                    ds->condition.notify_all();
                }
            }
        }
        // NOTE: READ / DOWNLOAD
        else if (cmd_lines[0] == "read" || cmd_lines[0] == "download")
        {
            if (cmd_lines.size() != 3 && cmd_lines.size() != 4)
            {
                std::cerr << "[Error] Usage: " << " read <src_file_path> <dst_file_path>" << std::endl;
                std::cerr << "[Error] Usage: " << " download <FileID> <Offset> <dst_file_path>" << std::endl;
                continue;
            }
            else
            {
                if (cmd_lines[0] == "read" && file_info.find(cmd_lines[1]) == file_info.end())
                {
                    std::cerr << "[Error] File not found." << std::endl;
                    continue;
                }
                for (int i = 0; i < 4; ++i)
                {
                    DataServer *ds = dataServers_[i];
                    std::unique_lock<std::mutex> lck(ds->mtx);
                    ds->cmd = cmd_lines[0];
                    if (cmd_lines[0] == "read")
                    {
                        std::pair<int, int> fileInfo = file_info[cmd_lines[1]];
                        ds->id = fileInfo.first;
                        ds->buffer_size_ = fileInfo.second;
                    }
                    else // cmd_lines[0] == "download"
                    {
                        ds->id = std::stoi(cmd_lines[1]);
                        ds->offset = std::stoi(cmd_lines[2]);
                    }
                    ds->is_active_ = true;
                    lck.unlock();
                    ds->condition.notify_all();
                }
            }
        }
        // NOTE: FIND
        else if (cmd_lines[0] == "find")
        {
            if (cmd_lines.size() != 3)
            {
                std::cerr << "[Error] Usage: find <FileID> <Offset>" << std::endl;
                continue;
            }
            else
            {
                for(int i = 0; i < 4; ++i)
                {
                    DataServer *ds = dataServers_[i];
                    std::unique_lock<std::mutex> lck(ds->mtx);
                    ds->cmd = "find";
                    ds->id = std::stoi(cmd_lines[1]);
                    ds->offset = std::stoi(cmd_lines[2]);
                    ds->is_active_ = true;
                    lck.unlock();
                    ds->condition.notify_all();
                }
            }
        }
        // NOTE: ERROR
        else
        {
            std::cerr << "[Error] Command not found." << std::endl;
        }
        // Wait for the DataServers to finish the task
        for (const auto &ds: dataServers_)
        {
            std::unique_lock<std::mutex> lck(ds->mtx);
            (ds->condition).wait(lck, [&](){ return !ds->is_active_; });
            lck.unlock();
            (ds->condition).notify_all();
        }
        // Post moves for Read/Download the file
        if (cmd_lines[0] == "read" || cmd_lines[0] == "download")
        {
            std::string md5_checksum, pre_checksum;
            for (int i = 0; i < 4; ++i)
            {
                if (dataServers_[i]->buffer_size_ )
                {
                    std::ofstream os;
                    if (cmd_lines[0] == "read") os.open(cmd_lines[2]);
                    else if (cmd_lines[0] == "download") os.open(cmd_lines[3]);
                    if (!os)
                    {
                        std::cerr << "[Error] Read/Download file error." << std::endl;
                    }
                    else
                    {
                        os.write(dataServers_[i]->buffer_, dataServers_[i]->buffer_size_);
                        os.close();
                        auto input_string = std::string(dataServers_[i]->buffer_, dataServers_[i]->buffer_size_);
                        MD5 md5(input_string);
                        md5_checksum = md5.toStr();
                        if(!pre_checksum.empty() && pre_checksum != md5_checksum)
                        {
                            std::cerr << "[Error] Checksum error for different dataServers. " << std::endl;
                        }
                        else if (!pre_checksum.empty() && pre_checksum == md5_checksum)
                        {
                            std::cout << "Checksum: " << md5_checksum << std::endl;
                        }
                        pre_checksum = md5_checksum;
                    }
                    delete []dataServers_[i]->buffer_;
                }
            }      
        }
        // Post moves for Upload
        else if (cmd_lines[0] == "upload")
        {
            std::cout << "Successfully uploaded. The fileID is " << cnt_ << std::endl;
        }
        // Post moves for Find and List
        else if (cmd_lines[0] == "find" || cmd_lines[0] == "ls")
        {
            bool not_found = true;
            for (int i = 0; i < 4; ++i)
            {
                if (dataServers_[i]->buffer_size_ != 0)
                {
                    not_found = false;
                    std::cout << "File FOUND. FileID: " << dataServers_[i]->id << " Offset: " << dataServers_[i]->offset << " Place: " << dataServers_[i]->get_name() << std::endl;
                }
            }
            if (not_found)
                std::cout << "File NOT FOUND." << std::endl;
        }
        delete []buffer;
        is.close();
    }   
}