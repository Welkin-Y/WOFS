

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

    std::string name; // full path
    long size;
    unsigned int permission;
    unsigned int owner;
    unsigned int group;
    bool isDir;
public:
    Meta(std::string name, long size, unsigned int permission, unsigned int owner, unsigned int group, bool isDir);
    std::string getName();
    long getSize();
    unsigned int getPermission();
    unsigned int getOwner();
    unsigned int getGroup();
    bool isDirectory();
};



class TreeNode {

    Meta m;
    void* data;
    TreeNode* child;
    TreeNode* sibling;
public:

    TreeNode(Meta m);
    TreeNode(Meta m, TreeNode* child, TreeNode* sibling);
    Meta getMeta();

    // in case needed
    int updateMeta(Meta v);

    TreeNode* find(const std::string& name);

    int setChild(TreeNode* child);
    int setSibling(TreeNode* sibling);
    TreeNode* getChild();
    TreeNode* getSibling();

    std::vector<TreeNode*> getAllChildren();


    void* getData();
};


std::vector<Meta> allFiles(const std::string& directory);


