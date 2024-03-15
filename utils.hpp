

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <iostream>
#include <vector>
#include <string>

class Meta {

    std::string name;
    long size;
    unsigned int permission;
    unsigned int owner;
    unsigned int group;
public:
    Meta(std::string name, long size, unsigned int permission, unsigned int owner, unsigned int group);
    std::string getName();
    long getSize();
    unsigned int getPermission();
    unsigned int getOwner();
    unsigned int getGroup();
};


template <typename T>
class TreeNode {

    T v; // replace with metadata
    TreeNode* child;
    TreeNode* sibling;
public:

    TreeNode(T v);
    TreeNode(T v, TreeNode* child, TreeNode* sibling);
    T getV();

    // in case needed
    int updateV(T v);

    int setChild(TreeNode* child);
    int setSibling(TreeNode* sibling);
    TreeNode* getChild();
    TreeNode* getSibling();
};


std::vector<Meta> allFiles(const std::string& directory);


