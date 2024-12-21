#ifndef _FILE_SYS_H
#define _FILE_SYS_H

#include <string>
#include <map>

/**
 * @brief: FileNode class for the FileSys
 * @param: val_, the name of the file or directory
 * @param: isFile_, a boolean value to indicate if the node is a file or directory
 * @param: parent_, a pointer to the parent node
 * @param: firstChild_, a pointer to the first child node
 * @param: nextSibling_, a pointer to the next sibling node
 * 
 */
struct FileNode
{
    std::string val_;
    bool isFile_;
    FileNode *parent_;
    FileNode *firstChild_;
    FileNode *nextSibling_;

    FileNode(const std::string &_value, const bool &_isFile): 
        val_(_value), isFile_(_isFile), parent_(nullptr), firstChild_(nullptr), nextSibling_(nullptr){};
};

/**
 * @brief: FileSys class
 * @param: _root, private, a pointer to the root node
 * @interface: insert_node, insert a node to the FileSys
 * @interface: find_node, find a node in the FileSys
 * 
 */
class FileSys{
private:
    FileNode *_root;
public:
    FileSys();
    bool insert_node(const std::string &path, const bool isFile);
    bool find_node(const std::string &path, FileNode **last_node)const;
    void list_recursive(FileNode *node, std::map<std::string, std::pair<int, int>>& file_info);
    void list(std::map<std::string, std::pair<int, int>>& file_info);
};

#endif
