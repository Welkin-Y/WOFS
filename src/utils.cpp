#include "utils.hpp"
#include <map>

size_t iv_len = 12;
size_t tag_len = 16;
int aes_block_size = 16;

Meta::Meta(
    std::string name,
    long start,
    long size,
    unsigned int permission,
    unsigned int owner,
    unsigned int group,
    bool isDir,
    time_t lastModified) : name(name), start(start), size(size), permission(permission), owner(owner), group(group), isDir(isDir), lastModified(lastModified) {}

// Getters 
std::string Meta::getName() { return this->name; }
long Meta::getStart() { return this->start; }
long Meta::getSize() { return this->size; }
unsigned int Meta::getPermission() { return this->permission; }
unsigned int Meta::getOwner() { return this->owner; }
unsigned int Meta::getGroup() { return this->group; }
bool Meta::isDirectory() { return this->isDir; }
time_t Meta::getLastModified() { return this->lastModified; }

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
    // std::string n;
    // if (name.at(0) == '/') {
    //     n = name.substr(1);
    // }
    // else {
    //     n = name;
    // }
    if (name == "/") {
        return this;
    }
    if (this->m.getName() == name) {
        return this;
    }
    TreeNode* t;
    if (this->child != nullptr) {
        t = this->child->find(name);
        if (t != nullptr) {
            return t;
        }
    }
    if (this->sibling != nullptr) {
        t = this->sibling->find(name);
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
                Meta m = Meta((directory + "/" + ent->d_name), -1, st.st_size, st.st_mode, st.st_uid, st.st_gid, false, st.st_mtime);
                files.push_back(m);
            }

            if (ent->d_type == DT_DIR) {
                // dir
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                    Meta m = Meta((directory + "/" + ent->d_name), -1, st.st_size, st.st_mode, st.st_uid, st.st_gid, true, st.st_mtime);
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
    int nameLen = *(int*)(buffer + sizeof(int));
    std::string name = std::string(buffer + 2 * sizeof(int), nameLen);
    long start = *(long*)(buffer + 2 * sizeof(int) + nameLen);
    char* p = buffer + 2 * sizeof(int) + nameLen + sizeof(start);
    long size = *(long*)p;
    p += sizeof(size);
    unsigned int permission = *(unsigned int*)p;
    p += sizeof(permission);
    unsigned int owner = *(unsigned int*)p;
    p += sizeof(owner);
    unsigned int group = *(unsigned int*)p;
    p += sizeof(group);
    bool isDir = *(bool*)p;
    p += sizeof(isDir);
    time_t lastModified = *(time_t*)p;
    return Meta(name, start, size, permission, owner, group, isDir, lastModified);

}

/**
 * @read 4 bytes of meta length, read said length, and call parseImageMeta
*/
Meta readMeta(FILE* f) {
    int metaLen;
    fread(&metaLen, sizeof(metaLen), 1, f);
    char* buffer = new char[metaLen];
    memcpy(buffer, &metaLen, sizeof(metaLen));
    fread(buffer + sizeof(metaLen), 1, metaLen - sizeof(metaLen), f);
    Meta m = parseImageMeta(buffer, metaLen);
    delete buffer;
    return m;
}

std::vector<Meta> readAllMeta(FILE* f) {
    // read the 4B position p at the end of f
    fseek(f, 0, SEEK_END);
    int fileSize = ftell(f);
    fseek(f, -(long)sizeof(int), SEEK_END);
    int p;
    fread(&p, (int)sizeof(int), 1, f);
    std::cout << "fileSize: " << fileSize << std::endl;
    std::cout << "p: " << p << std::endl;
    // start from position p, read all meta
    std::vector<Meta> metas;
    fseek(f, p, SEEK_SET);
    while (ftell(f) < fileSize - (int)sizeof(int)) {
        metas.push_back(readMeta(f));
    }
    return metas;
}

TreeNode* generateTree(std::vector<Meta> metaList) {
    std::map<std::string, TreeNode*> record;
    TreeNode* root = new TreeNode(Meta("", -1, 0, 0700, 0, 0, true, time(NULL)));
    record["/"] = root;
    std::string currName, currPath;
    size_t lastSlashPos;

    for (Meta m : metaList) {
        TreeNode* curr = new TreeNode(m);
        currName = m.getName();
        lastSlashPos = currName.find_last_of('/');
        currPath = currName.substr(0, lastSlashPos);
        if (record[currPath + "/"]->getMeta().getName() == currPath) {
            record[currPath + "/"]->setChild(curr);
        }
        else {
            record[currPath + "/"]->setSibling(curr);
        }
        record[currPath + "/"] = curr;
        if (m.isDirectory()) {
            record[m.getName() + "/"] = curr;
        }
    }
    return root;
}




int writeImageMeta(std::string name, long start, long size, unsigned int permission, unsigned int owner, unsigned int group, bool isDir, time_t lastModified, FILE* file) {
    int nameSize = name.size();
    int metaSize = 2 * sizeof(int) + nameSize + sizeof(start) + sizeof(size) + sizeof(permission) + sizeof(owner) + sizeof(group) + sizeof(isDir) + sizeof(time_t);
    char* buffer = new char[metaSize];
    char* p = buffer;
    memcpy(p, &metaSize, sizeof(metaSize));
    p += sizeof(metaSize);
    memcpy(p, &nameSize, sizeof(nameSize));
    p += sizeof(nameSize);
    memcpy(p, name.c_str(), nameSize);
    p += nameSize;
    memcpy(p, &start, sizeof(start));
    p += sizeof(start);
    memcpy(p, &size, sizeof(size));
    p += sizeof(size);
    memcpy(p, &permission, sizeof(permission));
    p += sizeof(permission);
    memcpy(p, &owner, sizeof(owner));
    p += sizeof(owner);
    memcpy(p, &group, sizeof(group));
    p += sizeof(group);
    memcpy(p, &isDir, sizeof(isDir));
    p += sizeof(isDir);
    memcpy(p, &lastModified, sizeof(lastModified));
    p += sizeof(lastModified);
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
        writeImageMeta(it->getName(), it->getStart(), it->getSize(), it->getPermission(), it->getOwner(), it->getGroup(), it->isDirectory(), it->getLastModified(), file);
    }
    fwrite(&p, sizeof(p), 1, file);

    return 0;
}


/**
 * @brief helper function of generateImage
 * recursively write file into image and record meta
*/
std::vector<Meta> genHelper(const std::string& directory, const std::string& relaDir, FILE* f, long currPosition) {
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
                int p = st.st_mode;
                p &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
                Meta m = Meta((relaDir + "/" + ent->d_name), currPosition, st.st_size, p, st.st_uid, st.st_gid, false, st.st_mtime);
                files.push_back(m);
                currPosition += st.st_size;
                std::cout << "now the position is: " << currPosition << "\n";
            }

            if (ent->d_type == DT_DIR) {
                // dir
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                    // toWrite = fopen((directory + "/" + ent->d_name).c_str(), "r");
                    // char* buffer = new char[st.st_size];
                    // fread(buffer, 1, st.st_size, toWrite);
                    // fwrite(buffer, 1, st.st_size, f);
                    // delete buffer;
                    // fclose(toWrite);
                    int p = st.st_mode;
                    p &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
                    Meta m = Meta((relaDir + "/" + ent->d_name), currPosition, st.st_size, p, st.st_uid, st.st_gid, true, st.st_mtime);
                    files.push_back(m);
                    // currPosition += st.st_size;
                    std::cout << "now the position is: " << currPosition << "\n";
                    std::vector<Meta> sub =
                        genHelper(directory + "/" + ent->d_name, relaDir + "/" + ent->d_name, f, currPosition);
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

    // check if directory is a directory
    struct stat st;
    if (stat(directory.c_str(), &st) != 0) {
        perror("Error: Cannot open the directory\n");
        return EXIT_FAILURE;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Error: %s is not a directory\n", directory.c_str());
        return EXIT_FAILURE;
    }

    std::vector<Meta> files;
    long currPosition = 0;
    FILE* f = fopen(image.c_str(), "w");
    if (f != nullptr) {
        files = genHelper(directory, "", f, currPosition);
        writeAllMeta(files, f);
        fclose(f);
        return EXIT_SUCCESS;
    }
    perror("Error: Cannot open the image file\n");
    return EXIT_FAILURE;
}


// copied from my ECE 560 homework, which was copied from https://wiki.openssl.org/index.php/EVP_Authenticated_Encryption_and_Decryption


/**
 * @brief encrypt plaintext using AES-256-GCM
 * @param plaintext: the plaintext to be encrypted
 * @param plaintext_len: the length of the plaintext
 * @param aad: safe to be null
 * @param aad_len: the length of aad (safe to be 0)
 * @param key: the key for encryption
 * @param iv: the initialization vector
 * @param iv_len: the length of iv
 * @param ciphertext: the buffer to store the ciphertext
 * @param tag: the buffer to store the tag
*/
int gcm_encrypt(unsigned char* plaintext, int plaintext_len, unsigned char* aad,
    int aad_len, unsigned char* key, unsigned char* iv, int iv_len,
    unsigned char* ciphertext, unsigned char* tag) {
    EVP_CIPHER_CTX* ctx;

    int len;

    int ciphertext_len;

    /* Create and initialise the context */
    if (!(ctx = EVP_CIPHER_CTX_new())) { std::cout << "error in EVP_CIPHER_CTX_new" << std::endl; }

    /* Initialise the encryption operation. */
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
    {
        std::cout << "error in EVP_EncryptInit_ex" << std::endl;
    }

    /*
     * Set IV length if default 12 bytes (96 bits) is not appropriate
     */
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv_len, NULL))
    {
        std::cout << "error in EVP_CIPHER_CTX_ctrl" << std::endl;
    }

    /* Initialise key and IV */
    if (1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv)) { std::cout << "error in EVP_EncryptInit_ex" << std::endl; }

    /*
     * Provide any AAD data. This can be called zero or more times as
     * required
     */
    if (1 != EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_len)) { std::cout << "error in EVP_EncryptUpdate" << std::endl; }

    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    {
        std::cout << "error in EVP_EncryptUpdate" << std::endl;
    }
    ciphertext_len = len;

    /*
     * Finalise the encryption. Normally ciphertext bytes may be written at
     * this stage, but this does not occur in GCM mode
     */
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) { std::cout << "error in EVP_EncryptFinal_ex" << std::endl; }
    ciphertext_len += len;

    /* Get the tag */
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag))
    {
        std::cout << "error in EVP_CIPHER_CTX_ctrl" << std::endl;
    }

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;

}

/**
 * @brief decrypt ciphertext using AES-256-GCM
 * @param ciphertext: the ciphertext to be decrypted
 * @param ciphertext_len: the length of the ciphertext
 * @param aad: safe to be null
 * @param aad_len: the length of aad (safe to be 0)
 * @param tag: the tag to be verified
 * @param key: the key for decryption
 * @param iv: the initialization vector
 * @param iv_len: the length of iv
 * @param plaintext: the buffer to store the plaintext
*/

int gcm_decrypt(unsigned char* ciphertext, int ciphertext_len,
    unsigned char* aad, int aad_len, unsigned char* tag,
    unsigned char* key, unsigned char* iv, int iv_len,
    unsigned char* plaintext) {
    EVP_CIPHER_CTX* ctx;
    int len;
    int plaintext_len;
    int ret;
    ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv_len, NULL);
    EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv);
    EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len);
    plaintext_len = len;
    std::cout << std::endl;
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag);
    ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
    EVP_CIPHER_CTX_free(ctx);
    if (ret > 0) {
        return plaintext_len;
    }
    else {
        std::cout << "verification of tag failed: plaintext is not trustworthy" << std::endl;
        return -1;
    }
}



/**
 * @brief decrypt only specific blocks of ciphertext
 * @param ciphertext: the ciphertext to be decrypted
 * @param ciphertext_len: the length of the ciphertext
 * @param aad: safe to be null
 * @param aad_len: the length of aad (safe to be 0)
 * @param tag: the tag to be verified
 * @param key: the key for decryption
 * @param iv: the initialization vector
 * @param iv_len: the length of iv
 * @param plaintext: the buffer to store the plaintext
 * @param start: the start position of the block to be decrypted
 * @param end: the end position of the block to be decrypted (inclusive)
*/
int gcm_seek_decrypt(unsigned char* ciphertext, int ciphertext_len,
    unsigned char* aad, int aad_len, unsigned char* tag,
    unsigned char* key, unsigned char* iv, int iv_len,
    unsigned char* plaintext, int start, int end) {
    EVP_CIPHER_CTX* ctx;
    int len;
    int plaintext_len;
    int ret;


    /* Create and initialise the context */
    if (!(ctx = EVP_CIPHER_CTX_new())) { std::cout << "error in EVP_CIPHER_CTX_new" << std::endl; }

    /* Initialise the decryption operation. */
    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
    {
        std::cout << "error in EVP_DecryptInit_ex" << std::endl;
    }

    /* Set IV length. Not necessary if this is 12 bytes (96 bits) */
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv_len, NULL))
    {
        std::cout << "error in EVP_CIPHER_CTX_ctrl" << std::endl;
    }

    // calculate the iv at offset start * aes_block_size
    unsigned char iv_true[iv_len];
    memcpy(iv_true, iv, iv_len);

    /* Initialise key and IV */
    if (!EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv_true)) { std::cout << "error in EVP_DecryptInit_ex" << std::endl; }



    if (!EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, end))
    {
        std::cout << "error in EVP_DecryptUpdate" << std::endl;
    }
    plaintext_len = len;

    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag)) { std::cout << "error in EVP_CIPHER_CTX_ctrl" << std::endl; }
    ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);

    EVP_CIPHER_CTX_free(ctx);
    return plaintext_len;
}

// encrypted image structure:
//  | encrypted content | 16B iv | 16B tag |

/**
 * @brief wrapper around gcm_encrypt
 * @param plaintext: the plaintext to be encrypted
 * @param plaintext_len: the length of the plaintext
 * @param key: the key for encryption in std::string format
 * @param path: the path to store the encrypted image
*/

// TODO: replace nums with constants
int encryption(unsigned char* plaintext, int plaintext_len, const std::string& key, const std::string& path) {
    unsigned char iv[iv_len];

    unsigned char keyBuf[32];
    unsigned char hash[32];
    SHA256((const unsigned char*)key.c_str(), key.size(), hash);
    unsigned char* keyhash = hash;
    unsigned char tag[tag_len];
    unsigned char ciphertext[plaintext_len + 16];
    int l = gcm_encrypt(plaintext, plaintext_len, NULL, 0, keyhash, iv, iv_len, ciphertext, tag);
    FILE* f = fopen(path.c_str(), "w");

    fwrite(ciphertext, 1, l, f);
    fwrite(iv, 1, iv_len, f);
    fwrite(tag, 1, tag_len, f);
    fclose(f);

    std::cout << std::endl;

    return 0;
}

/**
 * @brief read all metas from an encrypted image
*/

// TODO: implement readMeta to take in a buffer instead of f
std::vector<Meta> decryption_read_meta(FILE* f, const std::string& key) {
    unsigned char iv[iv_len];
    unsigned char tag[tag_len];
    unsigned char keyBuf[32];
    unsigned char hash[32];
    SHA256((const unsigned char*)key.c_str(), key.size(), hash);
    unsigned char* keyhash = hash;
    fseek(f, -tag_len - iv_len, SEEK_END);
    fread(iv, 1, iv_len, f);
    fread(tag, 1, tag_len, f);


    fseek(f, 0, SEEK_END);
    int l = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::cout << "l: " << l << std::endl;
    l = l - tag_len - iv_len;
    std::cout << "l: " << l << std::endl;
    unsigned char* ciphertext = new unsigned char[l];
    fread(ciphertext, 1, l, f);
    unsigned char plaintext[l];
    int plain_len = gcm_decrypt(ciphertext, l, NULL, 0, tag, keyhash, iv, iv_len, plaintext);

    return readAllMeta(plaintext, plain_len, true);

}

Meta readMeta(unsigned char* buf, int* rec) {
    int metaLen;
    memcpy(&metaLen, buf, sizeof(metaLen));
    *rec = metaLen;
    char* buffer = new char[metaLen];
    memcpy(buffer, &metaLen, sizeof(metaLen));
    memcpy(buffer + sizeof(metaLen), buf + sizeof(metaLen), metaLen - sizeof(metaLen));
    Meta m = parseImageMeta(buffer, metaLen);
    delete buffer;
    return m;
}

std::vector<Meta> readAllMeta(unsigned char* buf, int fileSize, bool e) {
    int p;
    memcpy(&p, buf + fileSize - sizeof(int), sizeof(int));
    std::cout << "fileSize: " << fileSize << std::endl;
    std::cout << "p: " << p << std::endl;
    std::vector<Meta> metas;
    int i = p;
    int rec = 0;

    while (i < fileSize - sizeof(int)) {
        Meta m = readMeta(buf + i, &rec);
        metas.push_back(m);
        i += rec;
    }
    return metas;
}

/**
 * read from beginning to size + start's end block, decrypt, store in buffer
*/
int decryption_read_data(FILE* f, const std::string& key, int start, int size, unsigned char* buf) {
    unsigned char iv[iv_len];
    unsigned char tag[tag_len];
    unsigned char keyBuf[32];
    unsigned char hash[32];
    SHA256((const unsigned char*)key.c_str(), key.size(), hash);
    unsigned char* keyhash = hash;
    fseek(f, -tag_len - iv_len, SEEK_END);
    fread(iv, 1, tag_len, f);
    fread(tag, 1, iv_len, f);


    fseek(f, 0, SEEK_SET);

    // if start + size is not a multiple of aes_block_size, pad the full length to full block
    int to_read = start + size;
    if (to_read % aes_block_size != 0) {
        to_read += aes_block_size - to_read % aes_block_size;
    }
    unsigned char* ciphertext = new unsigned char[to_read];
    fread(ciphertext, 1, to_read, f);
    unsigned char plaintext[to_read];
    int plain_len = gcm_seek_decrypt(ciphertext, to_read, NULL, 0, tag, keyhash, iv, iv_len, plaintext, start, start + size);
    memcpy(buf, plaintext + start, size);
    return 0;
}