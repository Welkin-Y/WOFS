#pragma once


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

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include <iostream>
#include <vector>
#include <string>

class Meta {

    std::string name; // full path
    long start;
    long size;
    unsigned int permission;
    unsigned int owner;
    unsigned int group;
    bool isDir;
    time_t lastModified;
public:
    Meta(std::string name, long start, long size, unsigned int permission, unsigned int owner, unsigned int group, bool isDir, time_t lastModified);
    std::string getName();
    long getStart();
    long getSize();
    unsigned int getPermission();
    unsigned int getOwner();
    unsigned int getGroup();
    bool isDirectory();
    time_t getLastModified();
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

/*
image layout:

    |      file 0   |     file 1     | file 2 | file 3 | ... | file n |    meta 0    | meta 1 | meta 2 | meta 3 | ... | meta n |   4B position p |
beginning                                                            end                                                      end                end
    of                                                               of                                                        of                of
   image                                                        file content,                                                metas             image
                                                                 position p
*/

/*
image meta layout:
| 4B length | 4B name length | variable length name | 8B start | 8B file size | 4B permission | 4B owner | 4B group | 1B isDir | 4B last modified time |

*/
// For generate filesystem from image:
Meta parseImageMeta(char* buffer, int metaLen);

Meta readMeta(FILE* f);

std::vector<Meta> readAllMeta(FILE* f);

TreeNode* generateTree(std::vector<Meta> metaList);

// For generate Image
int writeImageMeta(std::string name, long start, long size, unsigned int permission, unsigned int owner, unsigned int group, bool isDir, time_t lastModified, FILE* file);

int writeAllMeta(std::vector<Meta> metas, FILE* file);

int generateImage(const std::string& directory, const std::string& image);



// GCM mode of operation: still needs hmac? 

int gcm_encrypt(unsigned char* plaintext, int plaintext_len, unsigned char* aad,
    int aad_len, unsigned char* key, unsigned char* iv, int iv_len,
    unsigned char* ciphertext, unsigned char* tag);

int gcm_decrypt(unsigned char* ciphertext, int ciphertext_len,
    unsigned char* aad, int aad_len, unsigned char* tag,
    unsigned char* key, unsigned char* iv, int iv_len,
    unsigned char* plaintext);

int gcm_seek_decrypt(unsigned char* ciphertext, int ciphertext_len,
    unsigned char* aad, int aad_len, unsigned char* tag,
    unsigned char* key, unsigned char* iv, int iv_len,
    unsigned char* plaintext, int start, int end);

int encryption(unsigned char* plaintext, int plaintext_len, const std::string& key, const std::string& path);
std::vector<Meta> decryption_read_meta(FILE* f, const std::string& key);

std::vector<Meta> readAllMeta(unsigned char* buf, int fileSize, bool e);

int decryption_read_data(FILE* f, const std::string& key, int start, int size, unsigned char* buf);