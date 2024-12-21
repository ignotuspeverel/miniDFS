#include <iostream>
#include <string>
#include <cmath>
#include "filesys.h"
#include "md5.h"
#include "utils.h"

/**
 * @brief: Create a new file system, out with a root node
 */
FileSys::FileSys()
{
    _root = new FileNode("/", false);
}

/**
 * @brief: Insert a node in the FileSys
 * @param: path, the path of the node
 * @param: isFile, whether the node is a file or directory
 * @return: bool, true if the node is inserted successfully
 */
bool FileSys::insert_node(const std::string &path, const bool isFile)
{
    FileNode *parent = nullptr;
    // first try to find the if the node is already in the filesys, if not update the parent node
    bool isFound = find_node(path, &parent); 
    if(isFound) return false;

    std::vector<std::string> path_folders = split(path, '/');
    FileNode *newNode = new FileNode(path, isFile);
    newNode->parent_ = parent;
    FileNode* child = parent->firstChild_;
    if(!child)
        parent->firstChild_ = newNode;
    else
    {
        while(child->nextSibling_)
            child = child->nextSibling_;
        child->nextSibling_ = newNode;
    }
    return true;
}

/**
 * @brief: Find a node in a given path in the filesys
 * @param: path, the path of the node
 * @param: last_node, the last node searched
 * @return: bool, true if the node is found
 */
bool FileSys::find_node(const std::string &path, FileNode **last_node)const
{
    std::vector<std::string> path_folders = split(path, '/');
    FileNode *node = _root->firstChild_;
    *last_node = _root;
    for(const auto &name: path_folders)
    {
        while(node && node->val_ != name)
            node = node->nextSibling_;
        if(!node)
            return false;
        *last_node = node;
        node = node->firstChild_;
    }
    return true && node->isFile_;
}

/**
 * @brief: List the files in the FileSys with a recursive way
 * @param: node, the node to start listing
 * @param: file_info, a map of file information (file name -> (file id, file size))
 */
void FileSys::list_recursive(FileNode *node, std::map<std::string, std::pair<int, int>>& file_info)
{
    static int chunk_size = 2 * 1024 * 1024;
    if(node)
    {
        int file_id = file_info[node->val_].first;
        int file_size = (int)ceil(1.0 * file_info[node->val_].second / chunk_size);
        std::cout << node->val_ << "\t" << file_id << "\t" << file_size << std::endl;
        list_recursive(node->firstChild_, file_info);
        list_recursive(node->nextSibling_, file_info);
    }
}

/**
 * @brief: Interface of the list_recursive function
 * @param: file_info, a map of file information (file name -> (file id, file size))
 */
void FileSys::list(std::map<std::string, std::pair<int, int>>& file_info)
{
    list_recursive(_root, file_info);
}