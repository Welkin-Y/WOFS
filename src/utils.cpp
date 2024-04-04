#include "utils.hpp"

std::map<std::string, long> blockHashToPositionMap;
size_t iv_len = 32;
size_t tag_len = 16;
int aes_block_size = 32;
int sha_len = 32;

Meta::Meta(
    std::string name,
    long start,
    long size,
    unsigned int permission,
    unsigned int owner,
    unsigned int group,
    bool isDir,
    time_t lastModified): name(name), start(start), size(size), permission(permission), owner(owner), group(group), isDir(isDir), lastModified(lastModified) {
    this->block_info = std::vector<BlockInfo>{BlockInfo({start, size})};
}

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
    int len = fread(&metaLen, sizeof(metaLen), 1, f);
    char* buffer = new char[metaLen];
    memcpy(buffer, &metaLen, sizeof(metaLen));
    len = fread(buffer + sizeof(metaLen), 1, metaLen - sizeof(metaLen), f);
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
    int len = fread(&p, (int)sizeof(int), 1, f);
    // std::cout << "fileSize: " << fileSize << std::endl;
    // std::cout << "p: " << p << std::endl;
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
    // std::cout << "writing meta to image\n";
    fseek(file, 0, SEEK_END);
    int p = ftell(file);
    // std::cout << "p: " << p << std::endl;
    for (std::vector<Meta>::iterator it = metas.begin(); it != metas.end(); it++) {
        writeImageMeta(it->getName(), it->getStart(), it->getSize(), it->getPermission(), it->getOwner(), it->getGroup(), it->isDirectory(), it->getLastModified(), file);
    }
    // std::cout << "p rests at " << ftell(file) << std::endl;
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
                int len = fread(buffer, 1, st.st_size, toWrite);
                fwrite(buffer, 1, st.st_size, f);
                delete buffer;
                fclose(toWrite);
                int p = st.st_mode;
                p &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
                Meta m = Meta((relaDir + "/" + ent->d_name), currPosition, st.st_size, p, st.st_uid, st.st_gid, false, st.st_mtime);
                files.push_back(m);
                currPosition += st.st_size;
                // std::cout << "now the position is: " << currPosition << "\n";
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
                    // std::cout << "now the position is: " << currPosition << "\n";
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
void handleError() {
    ERR_print_errors_fp(stderr);
}

int update_iv(unsigned char* iv, int v) {
    int i;
    memcpy(&i, iv + aes_block_size - sizeof(int), sizeof(int));
    i += v;
    memcpy(iv + aes_block_size - sizeof(int), &i, sizeof(int));
    return i;
}

// single block ecb encryption. return the exact same size as input block size
int ecb_encrypt(unsigned char* in, int in_len, unsigned char* out, unsigned char* key) {
    EVP_CIPHER_CTX* ctx;
    int len;
    int out_len;

    if (!(ctx = EVP_CIPHER_CTX_new())) { handleError(); }
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, key, NULL)) { handleError(); }
    if (1 != EVP_EncryptUpdate(ctx, out, &len, in, in_len)) { handleError(); }
    out_len = len;
    if (1 != EVP_EncryptFinal_ex(ctx, out + len, &len)) { handleError(); }
    out_len += len;
    EVP_CIPHER_CTX_free(ctx);

    return out_len;
}

// single block ecb decryption. return the exact same size as input block size
int ecb_decrypt(unsigned char* in, int in_len, unsigned char* out, unsigned char* key) {
    EVP_CIPHER_CTX* ctx;
    int len;
    int out_len;

    if (!(ctx = EVP_CIPHER_CTX_new())) { handleError(); }
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, key, NULL)) { { handleError(); } }
    if (1 != EVP_DecryptUpdate(ctx, out, &len, in, in_len)) { handleError(); }
    out_len = len;
    if (1 != EVP_DecryptFinal_ex(ctx, out + len, &len)) { handleError(); }
    out_len += len;
    EVP_CIPHER_CTX_free(ctx);

    return out_len;
}



/**
 * @brief encrypt a file with a key, calculate hmac. does NOT remove the plaintext file.
*/
int encrypt(const std::string& path, const std::string& key) {
    FILE* f = fopen(path.c_str(), "r");
    std::string out = path + ".enc";
    FILE* out_f = fopen(out.c_str(), "w");
    fseek(f, 0, SEEK_END);
    int fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char iv[aes_block_size];
    RAND_bytes(iv, aes_block_size);
    unsigned char origin_iv[aes_block_size];
    memcpy(origin_iv, iv, aes_block_size);
    unsigned char keyhash[aes_block_size];
    SHA256((const unsigned char*)key.c_str(), key.size(), keyhash);
    unsigned char plain_buff[aes_block_size];
    unsigned char cipher_buff[aes_block_size];

    EVP_MD_CTX* mdctx;

    unsigned char digest[sha_len];
    unsigned int digest_len;
    if ((mdctx = EVP_MD_CTX_new()) == NULL) {
        handleError();
    }

    if (1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL)) {
        handleError();
    }

    int len;
    while ((len = fread(plain_buff, 1, aes_block_size, f)) > 0) {

        // padding 
        // TODO: random padding

        if (len != aes_block_size) {
            memset(plain_buff + len, 0, aes_block_size - len);
        }

        int x = ecb_encrypt(iv, aes_block_size, cipher_buff, keyhash);

        for (int j = 0; j < aes_block_size; j++) {
            cipher_buff[j] = plain_buff[j] ^ cipher_buff[j];
        }
        int i = update_iv(iv, 1);
        fwrite(cipher_buff, 1, aes_block_size, out_f);

        if (1 != EVP_DigestUpdate(mdctx, cipher_buff, aes_block_size)) {
            handleError();
        }

    }
    if (1 != EVP_DigestFinal_ex(mdctx, digest, &digest_len)) {
        handleError();
    }
    EVP_MD_CTX_free(mdctx);
    // hash encrypted image
    unsigned char hmac[aes_block_size];
    ecb_encrypt(digest, sha_len, hmac, keyhash);


    // std::cout << "digest: \n";
    // for (int i = 0; i < digest_len; i++) {
    //     std::cout << std::hex << (int)digest[i];
    // }
    // std::cout << "\n" << std::dec;

    // std::cout << "encrypted digest: \n";
    // for (int i = 0; i < sha_len; i++) {
    //     std::cout << std::hex << (int)hmac[i];
    // }
    // std::cout << "\n" << std::dec;

    // write hmac, iv and original image size (before padding) to end of encrypted file
    fwrite(hmac, 1, aes_block_size, out_f);
    fwrite(origin_iv, 1, aes_block_size, out_f);
    fwrite(&fileSize, 1, sizeof(int), out_f);
    fclose(f);
    fclose(out_f);
    std::cout << "fsize: " << fileSize << std::endl;


    return 0;
}

/**
 * @brief decrypt a range of blocks from an encrypted file
 * @param f the file
 * @param key key to decrypt the file
 * @param st_blk start block to decrypt
 * @param ed_blk end block to decrypt (inclusive)
 * @param buffer buffer to write the decrypted content
*/
int decrypt(FILE* f, unsigned char* keyhash, int st_blk, int ed_blk, unsigned char* buffer) {

    fseek(f, -aes_block_size - sizeof(int), SEEK_END);
    unsigned char iv[aes_block_size];
    int len = fread(iv, 1, aes_block_size, f);
    int fileSize;
    len = fread(&fileSize, 1, sizeof(int), f);

    if (st_blk * aes_block_size > fileSize) {
        fprintf(stderr, "Error: start block is out of range: start block is %d, file size is %d\n", st_blk * aes_block_size, fileSize);
        return -1;
    }
    if (ed_blk * aes_block_size > fileSize) {
        fprintf(stderr, "Error: end block is out of range: end block is %d, file size is %d\n", ed_blk * aes_block_size, fileSize);
        return -1;
    }
    fseek(f, st_blk * aes_block_size, SEEK_SET);

    unsigned char cipher_buff[aes_block_size];
    unsigned char plain_buff[aes_block_size];

    update_iv(iv, st_blk);
    for (int i = st_blk; i <= ed_blk; i++) {
        len = fread(plain_buff, 1, aes_block_size, f);
        ecb_encrypt(iv, aes_block_size, cipher_buff, keyhash);
        for (int j = 0; j < aes_block_size; j++) {
            plain_buff[j] = cipher_buff[j] ^ plain_buff[j];
        }

        memcpy(buffer + (i - st_blk) * aes_block_size, plain_buff, aes_block_size);
        update_iv(iv, 1);
    }

    return 0;
}

int getImageSize(FILE* f) {

    fseek(f, -sizeof(int), SEEK_END);
    int p;
    int len = fread(&p, sizeof(int), 1, f);

    return p;
}


/**
 * @brief verify the integrity of an encrypted file
*/
int verify(FILE* f, unsigned char* keyhash) {

    fseek(f, -aes_block_size * 2 - sizeof(int), SEEK_END);
    int enc_size = ftell(f);
    // std::cout << "enc_size: " << enc_size << std::endl;
    unsigned char hmac[aes_block_size];
    int len = fread(hmac, 1, aes_block_size, f);
    unsigned char dec[aes_block_size];
    ecb_decrypt(hmac, aes_block_size, dec, keyhash);
    // std::cout << "decrypted hmac: \n";
    // for (int i = 0; i < aes_block_size; i++) {
    //     std::cout << std::hex << (int)dec[i];
    // }
    // std::cout << "\n" << std::dec;
    fseek(f, 0, SEEK_SET);

    EVP_MD_CTX* mdctx;

    unsigned char digest[sha_len];
    unsigned int digest_len;
    if ((mdctx = EVP_MD_CTX_new()) == NULL) {
        handleError();
    }

    if (1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL)) {
        handleError();
    }
    // read aes_block_size bytes at a time, update hash.
    unsigned char buffer[aes_block_size];
    while (true) {
        int l = aes_block_size;

        int len = fread(buffer, 1, l, f);

        if (1 != EVP_DigestUpdate(mdctx, buffer, aes_block_size)) {
            handleError();
        }
        enc_size -= aes_block_size;
        if (enc_size == 0) {
            break;
        }

    }
    // compare digest with decrypted hmac

    if (1 != EVP_DigestFinal_ex(mdctx, digest, &digest_len)) {
        handleError();
    }
    EVP_MD_CTX_free(mdctx);
    if (memcmp(digest, dec, sha_len) != 0) {
        fprintf(stderr, "Error: HMAC verification failed: This file has been corrupted. Do not trust its content.\n");
        return -1;
    }
    return 0;

}

/**
 * @brief read a block of image file, decrypt it, crop it to proper size, and write it to buffer
*/
int readEncImage(FILE* f, unsigned char* keyhash, unsigned char* buffer, int begin, int len) {
    // create a log file
    fseek(f, 0, SEEK_SET);
    int end = begin + len;


    int st_blk = begin / aes_block_size;
    int ed_blk = end / aes_block_size;
    int st_offset = begin % aes_block_size;


    unsigned char holder[(ed_blk - st_blk + 1) * aes_block_size];


    decrypt(f, keyhash, st_blk, ed_blk, holder);
    memcpy(buffer, holder + st_offset, len);

    return len;
}

/**
 * @brief read all meta from an encrypted image.
*/

std::vector<Meta> readEncMeta(FILE* f, unsigned char* keyhash) {
    if (verify(f, keyhash) != 0) {
        fprintf(stderr, "Error: HMAC verification failed\n");
        throw std::runtime_error("failed HMAC verification");
    }

    int imgsize = getImageSize(f);
    fseek(f, 0, SEEK_SET);
    unsigned char buffer[sizeof(int)];

    if (!readEncImage(f, keyhash, buffer, imgsize - sizeof(int), sizeof(int))) {
        fprintf(stderr, "Error: failed to read image when doing imgsize read\n");
        throw std::runtime_error("failed to read image");
    }
    int p;
    memcpy(&p, buffer, sizeof(int));
    unsigned char metaBuffer[imgsize - p];
    if (!readEncImage(f, keyhash, metaBuffer, p, imgsize - p)) {
        fprintf(stderr, "Error: failed to read image when doing meta read\n");
        throw std::runtime_error("failed to read image");

    }
    std::vector<Meta> metas = readAllMeta((char*)metaBuffer, imgsize - p);
    return metas;
}

std::vector<Meta> readAllMeta(char* buffer, int size) {
    std::vector<Meta> metas;
    int i = 0;
    while (i < size - sizeof(int)) {
        int metaLen;
        memcpy(&metaLen, buffer + i, sizeof(int));
        Meta m = parseImageMeta(buffer + i, metaLen);
        metas.push_back(m);
        i += metaLen;

    }
    return metas;
}