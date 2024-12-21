#ifndef _NAME_SERVER_H
#define _NAME_SERVER_H

#include <vector>
#include <map>
#include "DataServer.h"
#include "filesys.h"

class NameServer{
private:
    std::vector<DataServer *> dataServers_; // List of DataServers
    FileSys fileSys_; // File System
    
    int num_replicate_;
    int cnt_;

    std::vector<std::string> read_cmd(); // Read the command

public:
    std::map<std::string, std::pair<int, int>> file_info;

    explicit NameServer(int replicate_num);
    void add(DataServer *server);
    void operator()();
};


#endif
