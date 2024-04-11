#include "params.h"


#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#define UNSUPPORTED -1
#define VERSION "0.1: nothing to see"

#include "utils.hpp"


/**
 * tests meta read/write
*/

void traversal(TreeNode* curr) {
    if (!curr) return;
    std::cout << curr->getMeta().getName() << std::endl;
    traversal(curr->getChild());
    traversal(curr->getSibling());

}

int main() {
    // std::string n = "/test/abcd";
    // std::string n2 = "lkjdslkgjds/gldskjgds/gdi";
    // FILE* f = fopen("testfile", "w");

    // writeImageMeta(n, 100, 100, 0, 2, 7, true, time(NULL), f);
    // writeImageMeta(n2, 1000, 3, 4, 2, 3, false, time(NULL), f);
    // // write 0 to the end of f
    // int p = 0;
    // fwrite(&p, 4, 1, f);
    // fclose(f);

    // f = fopen("testfile", "r");

    // // get len and namelen from file
    // std::vector<Meta> metas = readAllMeta(f);
    // for (std::vector<Meta>::iterator it = metas.begin(); it != metas.end(); it++) {
    //     fprintf(stderr, "name: %s\n", it->getName().c_str());
    //     fprintf(stderr, "size: %ld\n", it->getSize());
    // }

    // fclose(f);

    generateImage("d0", "testImage");
    FILE* f = fopen("testImage", "r");
    std::cout << "reading from image testImage" << std::endl;
    std::vector<Meta> metas = readAllMeta(f);
    for (std::vector<Meta>::iterator it = metas.begin(); it != metas.end(); it++) {
        fprintf(stderr, "name: %s\n", it->getName().c_str());
        fprintf(stderr, "size: %ld\n", it->getSize());
        fprintf(stderr, "start: %ld\n", it->getStart());
        fprintf(stderr, "permission: %d\n", it->getPermission());
        fprintf(stderr, "owner: %d\n", it->getOwner());
        fprintf(stderr, "group: %d\n", it->getGroup());
        fprintf(stderr, "isDir: %d\n", it->isDirectory());

    }


    std::cout << "\n============\nTest Tree:\n";
    TreeNode* root = generateTree(metas);
    traversal(root);


    std::cout << "\n============\nTest Crypto:\n";



    unsigned char iv[16];
    memset(iv, 0, 16);
    size_t iv_len = 16;
    memcpy(iv, iv, iv_len);


    const char plaintext[] = "0123456789abcdeflskfjwldewfoewowelgkewt4829ogvnewokn_fedcba9876544810_hello, this is robort calling. please standby... alright fire!!!!!";
    unsigned char keyhash[32];

    f = fopen("testfile", "w");
    fwrite(plaintext, 1, strlen(plaintext), f);
    fclose(f);

    std::string key = "key";
    SHA256((const unsigned char*)key.c_str(), key.size(), keyhash);

    encrypt("testfile", "key");
    unsigned char buffer[96];
    memset(buffer, 0, 64);
    f = fopen("testfile.enc", "r");
    decrypt(f, keyhash, 1, 3, buffer);

    unsigned char buffer2[3];
    fclose(f);
    f = fopen("testfile.enc", "r");
    readEncImage(f, keyhash, buffer2, 0, 3);

    fclose(f);
    generateImage("d0", "testimage");
    f = fopen("testimage", "r");
    unsigned char buf[688];
    fread(buf, 1, 688, f);
    fclose(f);
    encrypt("testimage", "key");
    f = fopen("testimage.enc", "r");
    std::vector<Meta> metas2 = readEncMeta(f, keyhash);
    for (int i = 0; i < metas2.size(); i++) {
        std::cout << metas2[i].getName() << std::endl;
    }
    fclose(f);








    std::cout << "\n===================\nTest binary crypto\n===============\n";
    f = fopen("d2/pack", "rb");
    int l;
    fseek(f, 0, SEEK_END);
    l = ftell(f);
    fseek(f, 0, SEEK_SET);
    encrypt("d2/pack", "key");
    fseek(f, 0, SEEK_SET);
    unsigned char p[32];
    fread(p, 1, 32, f);
    unsigned char buffer4[32];
    fclose(f);
    f = fopen("d2/pack.enc", "rb");
    SHA256((const unsigned char*)"key", 3, keyhash);
    decrypt(f, keyhash, 0, 0, buffer4);
    fclose(f);
    //compare
    for (int i = 0; i < 32; i++) {
        if (p[i] != buffer4[i]) {
            std::cout << "error\n";
            break;
        }
    }
    return 0;

}