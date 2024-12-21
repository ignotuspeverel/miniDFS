# include <iostream>
#include <fstream>

# include "DataServer.h"

// Set the chunk size to 2MB
int chunk_size = 2 * 1024 * 1024;

/**
 * @brief: Constructor
 * @param: name, name of the DataServer
 * @return: void
 */
DataServer::DataServer(const std::string &name): ds_name_(name), buffer_(nullptr), is_active_(false)
{
    std::string cmd = "mkdir -p " + ds_name_;
    system(cmd.c_str());
}

/**
 * @brief: Overload the operator () to run the DataServer
 * @return: void
 */
void DataServer::operator()()
{
    while (true)
    {   
        // First lock the mutex for the DataServer, continue if the DataServer is active
        std::unique_lock<std::mutex> lck(mtx);
        condition.wait(lck, [this] { return this->is_active_; });
        if (cmd == "upload")
        {
            size_ += buffer_size_ / 1024.0 / 1024.0; // Update the size of the DataServer (in MB)
            upload();
        }
        else if (cmd == "read")
        {
            read();
        }
        else if (cmd == "download")
        {
            download();
        }
        else if (cmd == "find")
        {
            find();
        }
        this->is_active_ = false; // Set the DataServer to inactive
        lck.unlock(); // Unlock the mutex
        condition.notify_all(); // Notify all the threads
    }
    
}

/**
 * @brief: Upload the data to the DataServer
 * @return: void
 */
void DataServer::upload()
{
    int start = 0;
    std::ofstream os;
    // Divide the buffer into chunks and store them in different data servers
    while(start < buffer_size_)
    {
        int offset = start / chunk_size; // Which chunk the data belongs to
        std::string filePath = ds_name_ + "/" + std::to_string(id) + " " + std::to_string(offset);
        os.open(filePath);
        if(!os)
            std::cerr << "Create file error in dataserver: (file name) " << filePath << std::endl;
        os.write(&buffer_[start], std::min(chunk_size, buffer_size_ - start)); // Write the data to the file
        start += chunk_size;
        os.close();
    }
}

/**
 * @brief: Read the data from the DataServer
 * @return: void
 */ 
void DataServer::read()
{
    int start = 0;
    buffer_ = new char[buffer_size_];
    while(start < buffer_size_)
    {
        int offset = start / chunk_size;
        std::string filePath = ds_name_ + "/" + std::to_string(id) + " " + std::to_string(offset);
        std::ifstream is(filePath);
        if(!is)
        {
            delete []buffer_;
            buffer_size_ = 0;
            break;
        }
        is.read(&buffer_[start], std::min(chunk_size, buffer_size_ - start));
        start += chunk_size;
    }
}

/**
 * @brief: Download the data from the DataServer
 * @return: void
 */ 
void DataServer::download()
{
    buffer_ = new char[chunk_size];
    std::string filePath = ds_name_ + "/" + std::to_string(id) + " " + std::to_string(offset);
    std::ifstream is(filePath);
    if(!is)
    {
        delete []buffer_;
        buffer_size_ = 0;
    }
    else
    {
        is.read(buffer_, std::min(chunk_size, buffer_size_ - chunk_size * offset));
        buffer_size_ = is.tellg();
    }
}

/**
  *@brief: Find the data in the DataServer
  *@return: void
  */
void DataServer::find()
{
    std::string filePath = ds_name_ + "/" + std::to_string(id) + " " + std::to_string(offset);
    // std::cout << "file path in " << filePath << std::endl;
    std::ifstream is(filePath);
    if(is)
    {
        // std::cout << "file found in " << ds_name_ << std::endl;
        buffer_size_ = 1;
    }
    else
        buffer_size_ = 0;
}
