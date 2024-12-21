#ifndef _DATA_SERVER_H
#define _DATA_SERVER_H

#include <string>
#include <mutex>
#include <condition_variable>

class DataServer
{
private:
    std::string ds_name_; // Name of the DataServer
    double size_; // Size of the data in the DataServer (in MB)

    void upload(); // Upload the data to the DataServer
    void read(); // Read the data from the DataServer
    void download(); // Download the data from the DataServer
    void find(); // Find the data in the DataServer

public:
    std::mutex mtx; // Mutex to lock the DataServer
    std::condition_variable condition; // Condition variable to notify the DataServer
    std::string cmd; // Command to run the DataServer
    char* buffer_; // Buffer to store the data
    bool is_active_; // Flag to check if the DataServer is active
    int buffer_size_, id, offset; // Size of the buffer, id of the DataServer, offset of the data
    DataServer(const std::string &name); // Constructor
    void operator()(); // Overload the operator () to run the DataServer
    double get_size()const {return size_;}; // Get the size of the DataServer 
    std::string get_name()const {return ds_name_;}; // Get the name of the DataServer
};

#endif