#ifndef _UTIL_H
#define _UTIL_H

#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

using std::string;

/**
 * @brief: Split a string by a character
 * @param: s, the string to split
 * @param: split_char, the character to split the string
 * @return: a vector of strings
 */
inline std::vector<std::string> split(const std::string & s, const char & split_char)
{
    std::stringstream ss(s);
    std::vector<std::string> res;
    std::string str;
    while(std::getline(ss, str, split_char))
        if(!str.empty())
            res.push_back(str);
    return res;
}

/**
 * @brief: Get the index of the sorted vector
 * @param: vec, the vector to sort
 * @return: a vector of integers representing the index of the sorted vector
 */
template<typename T>
std::vector<int> argsort(const std::vector<T> &vec) 
{
    std::vector<int> idx;
    for(int i=0; i<vec.size(); ++i)
        idx.push_back(i);
    sort(idx.begin(), idx.end(), [&vec](int id1, int id2){return vec[id1] < vec[id2];});
    return idx;
}

#endif