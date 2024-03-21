#include "utils.hpp"


Meta::Meta(
    std::string name,
    long start,
    long size,
    unsigned int permission,
    unsigned int owner,
    unsigned int group,
    bool isDir) : name(name), start(start), size(size), permission(permission), owner(owner), group(group), isDir(isDir) {}

std::string Meta::getName() { return this->name; }
long Meta::getStart() { return this->start; }
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
                Meta m = Meta((directory + "/" + ent->d_name), -1, st.st_size, st.st_mode, st.st_uid, st.st_gid, false);
                files.push_back(m);
            }

            if (ent->d_type == DT_DIR) {
                // dir
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                    Meta m = Meta((directory + "/" + ent->d_name), -1, st.st_size, st.st_mode, st.st_uid, st.st_gid, true);
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



/**
 * @brief construct a meta object from a buffer, read from image file
 * image meta should be of the following structure:
 * | 4B length | 4B name length | variable length name | 8B start | 8B file size | 4B permission | 4B owner | 4B group | 1B isDir | 4B last modified time |
 * returns a Meta object
*/
Meta parseImageMeta(char* buffer, int metaLen) {
    int nameLen = *(int*)(buffer + 4);
    std::string name = std::string(buffer + 8, nameLen);
    long start = *(long*)(buffer + 8 + nameLen);
    char* p = buffer + 8 + nameLen + 8;
    long size = *(long*)p;
    p += 8;
    unsigned int permission = *(unsigned int*)p;
    p += 4;
    unsigned int owner = *(unsigned int*)p;
    p += 4;
    unsigned int group = *(unsigned int*)p;
    p += 4;
    bool isDir = *(bool*)p;
    p += 1;
    time_t lastModified = *(time_t*)p;
    return Meta(name, start, size, permission, owner, group, isDir);

}

/**
 * @read 4 bytes of meta length, read said length, and call parseImageMeta
*/
Meta readMeta(FILE* f) {
    int metaLen;
    fread(&metaLen, 4, 1, f);
    char* buffer = new char[metaLen];
    memcpy(buffer, &metaLen, 4);
    fread(buffer + 4, 1, metaLen - 4, f);
    Meta m = parseImageMeta(buffer, metaLen);
    delete buffer;
    return m;
}

std::vector<Meta> readAllMeta(FILE* f) {
    // read the 4B position p at the end of f
    fseek(f, 0, SEEK_END);
    int fileSize = ftell(f);
    fseek(f, -4, SEEK_END);
    int p;
    fread(&p, 4, 1, f);
    std::cout << "fileSize: " << fileSize << std::endl;
    std::cout << "p: " << p << std::endl;
    // start from position p, read all meta
    std::vector<Meta> metas;
    fseek(f, p, SEEK_SET);
    while (ftell(f) < fileSize - 4) {
        metas.push_back(readMeta(f));
    }
    return metas;
}

int writeImageMeta(std::string name, long start, long size, unsigned int permission, unsigned int owner, unsigned int group, bool isDir, time_t lastModified, FILE* file) {
    int nameSize = name.size();
    int metaSize = 8 + nameSize + 8 + 8 + 4 + 4 + 4 + 1 + 4;
    char* buffer = new char[metaSize];
    char* p = buffer;
    memcpy(p, &metaSize, 4);
    p += 4;
    memcpy(p, &nameSize, 4);
    p += 4;
    memcpy(p, name.c_str(), nameSize);
    p += nameSize;
    memcpy(p, &start, 8);
    p += 8;
    memcpy(p, &size, 8);
    p += 8;
    memcpy(p, &permission, 4);
    p += 4;
    memcpy(p, &owner, 4);
    p += 4;
    memcpy(p, &group, 4);
    p += 4;
    memcpy(p, &isDir, 1);
    p += 1;
    memcpy(p, &lastModified, 4);
    p += 4;
    fwrite(buffer, 1, metaSize, file);
    delete buffer;
    return 0;
}

/**
 * @brief record current file length, write all metas to image, then write the position of beginning of meta
*/
int writeAllMeta(std::vector<Meta> metas, FILE* file) {
    std::cout << "writing meta to image\n";
    fseek(file, 0, SEEK_END);
    int p = ftell(file);
    std::cout << "p: " << p << std::endl;
    for (std::vector<Meta>::iterator it = metas.begin(); it != metas.end(); it++) {
        writeImageMeta(it->getName(), it->getStart(), it->getSize(), it->getPermission(), it->getOwner(), it->getGroup(), it->isDirectory(), time(NULL), file);
    }
    fwrite(&p, 4, 1, file);

    return 0;
}


/**
 * @brief helper function of generateImage
 * recursively write file into image and record meta
*/
std::vector<Meta> genHelper(const std::string& directory, FILE* f, int currPosition) {
    std::vector<Meta> files;
    DIR* dir;
    struct dirent* ent;
    struct stat st;

    FILE* toWrite;

    if ((dir = opendir(directory.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            stat((directory + "/" + ent->d_name).c_str(), &st);
            if (ent->d_type == DT_REG) {
                // file 
                toWrite = fopen((directory + "/" + ent->d_name).c_str(), "r");
                // write toWrite to image
                char* buffer = new char[st.st_size];
                fread(buffer, 1, st.st_size, toWrite);
                fwrite(buffer, 1, st.st_size, f);
                delete buffer;
                fclose(toWrite);
                Meta m = Meta((directory + "/" + ent->d_name), currPosition, st.st_size, st.st_mode, st.st_uid, st.st_gid, false);
                files.push_back(m);
                currPosition += st.st_size;
                std::cout << "now the position is: " << currPosition << "\n";
            }

            if (ent->d_type == DT_DIR) {
                // dir
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                    toWrite = fopen((directory + "/" + ent->d_name).c_str(), "r");
                    char* buffer = new char[st.st_size];
                    fread(buffer, 1, st.st_size, toWrite);
                    fwrite(buffer, 1, st.st_size, f);
                    delete buffer;
                    fclose(toWrite);
                    Meta m = Meta((directory + "/" + ent->d_name), currPosition, st.st_size, st.st_mode, st.st_uid, st.st_gid, true);
                    files.push_back(m);
                    currPosition += st.st_size;
                    std::cout << "now the position is: " << currPosition << "\n";
                    std::vector<Meta> sub =
                        genHelper(directory + "/" + ent->d_name, f, currPosition);
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

/**
 * @brief generate an image file from a directory
*/
int generateImage(const std::string& directory, const std::string& image) {
    std::vector<Meta> files;
    int currPosition = 0;
    FILE* f = fopen(image.c_str(), "w");

    files = genHelper(directory, f, currPosition);

    writeAllMeta(files, f);
    fclose(f);
    return 0;
}
