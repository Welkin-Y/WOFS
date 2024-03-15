#include "utils.hpp"

Meta::Meta(std::string name,
    long size,
    unsigned int permission,
    unsigned int owner,
    unsigned int group) : name(name), size(size), permission(permission), owner(owner), group(group) {}

std::string Meta::getName() { return this->name; }
long Meta::getSize() { return this->size; }
unsigned int Meta::getPermission() { return this->permission; }
unsigned int Meta::getOwner() { return this->owner; }
unsigned int Meta::getGroup() { return this->group; }

template <typename T>
TreeNode<T>::TreeNode(T v) {
    this->v = v;
    this->child = nullptr;
    this->sibling = nullptr;
}

template <typename T>
TreeNode<T>::TreeNode(T v, TreeNode* child, TreeNode* sibling) {
    this->v = v;
    this->child = child;
    this->sibling = sibling;
}

template <typename T>
T TreeNode<T>::getV() {
    return this->v;
}

template <typename T>
int TreeNode<T>::updateV(T v) {
    this->v = v;
    return 0;
}

template <typename T>
int TreeNode<T>::setChild(TreeNode* child) {
    this->child = child;
    return 0;
}

template <typename T>
int TreeNode<T>::setSibling(TreeNode* sibling) {
    this->sibling = sibling;
    return 0;
}

template <typename T>
TreeNode<T>* TreeNode<T>::getChild() {
    return this->child;
}

template <typename T>
TreeNode<T>* TreeNode<T>::getSibling() {
    return this->sibling;
}

std::vector<Meta> allFiles(const std::string& directory) {
    std::vector<Meta> files;
    DIR* dir;
    struct dirent* ent;
    struct stat st;
    if ((dir = opendir(directory.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            stat((directory + "/" + ent->d_name).c_str(), &st);


            if (ent->d_type == DT_REG) {
                // file 
                Meta m = Meta((directory + "/" + ent->d_name), st.st_size, st.st_mode, st.st_uid, st.st_gid);
                files.push_back(m);
            }

            if (ent->d_type == DT_DIR) {
                // dir
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                    Meta m = Meta((directory + "/" + ent->d_name), st.st_size, st.st_mode, st.st_uid, st.st_gid);
                    files.push_back(m);
                    std::vector<Meta> sub =
                        allFiles(directory + "/" + ent->d_name);
                    for (size_t i = 0; i < sub.size(); i++) {
                        files.push_back(sub[i]);
                    }
                }
            }
        }
        closedir(dir);
    }
    else {
        std::cout << "Error opening directory\n";
    }

    return files;
}
