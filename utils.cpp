#include "utils.hpp"


Meta::Meta(
    std::string name,
    long size,
    unsigned int permission,
    unsigned int owner,
    unsigned int group,
    bool isDir) : name(name), size(size), permission(permission), owner(owner), group(group), isDir(isDir) {}

std::string Meta::getName() { return this->name; }
long Meta::getSize() { return this->size; }
unsigned int Meta::getPermission() { return this->permission; }
unsigned int Meta::getOwner() { return this->owner; }
unsigned int Meta::getGroup() { return this->group; }
bool Meta::isDirectory() { return this->isDir; }

TreeNode::TreeNode(Meta m) : m(m), child(nullptr), sibling(nullptr) {}
TreeNode::TreeNode(Meta m, TreeNode* child, TreeNode* sibling) : m(m), child(child), sibling(sibling) {

    if (!m.isDirectory() && child != nullptr) {
        fprintf(stderr, "Error: trying to set child of a non-directory node\n");
        this->child = nullptr;
    }
}

Meta TreeNode::getMeta() { return this->m; }
int TreeNode::updateMeta(Meta v) {
    this->m = v;
    return 0;
}
int TreeNode::setChild(TreeNode* child) {
    if (!this->getMeta().isDirectory()) {
        fprintf(stderr, "Error: trying to set child of a non-directory node\n");
        return -1;
    }
    this->child = child;
    return 0;
}
int TreeNode::setSibling(TreeNode* sibling) {
    this->sibling = sibling;
    return 0;
}

TreeNode* TreeNode::getChild() { return this->child; }
TreeNode* TreeNode::getSibling() { return this->sibling; }

std::vector<TreeNode*> TreeNode::getAllChildren() {
    std::vector<TreeNode*> children;
    TreeNode* current = this->child;
    while (current != nullptr) {
        children.push_back(current);
        current = current->getSibling();
    }
    return children;

}

void* TreeNode::getData() { return this->data; }



TreeNode* TreeNode::find(const std::string& name) {
    std::string n;
    if (name.at(0) == '/') {
        n = name.substr(1);
    }
    else {
        n = name;
    }
    if (this->m.getName() == n) {
        return this;
    }
    TreeNode* t;
    if (this->child != nullptr) {
        t = this->child->find(n);
        if (t != nullptr) {
            return t;
        }
    }
    if (this->sibling != nullptr) {
        t = this->sibling->find(n);
        if (t != nullptr) {
            return t;
        }
    }
    return nullptr;
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
                Meta m = Meta((directory + "/" + ent->d_name), st.st_size, st.st_mode, st.st_uid, st.st_gid, false);
                files.push_back(m);
            }

            if (ent->d_type == DT_DIR) {
                // dir
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                    Meta m = Meta((directory + "/" + ent->d_name), st.st_size, st.st_mode, st.st_uid, st.st_gid, true);
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
