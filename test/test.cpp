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


    // std::cout << "\n============\nTest Crypto:\n";



    // unsigned char iv[16];
    // memset(iv, 0, 16);
    // size_t iv_len = 16;
    // memcpy(iv, iv, iv_len);


    // const char plaintext[] = "0123456789abcdeflskfjwldewfoewowelgkewt4829ogvnewokn_fedcba9876544810_hello, this is robort calling. please standby... alright fire!!!!!";
    // unsigned char keyhash[32];

    // f = fopen("testfile", "w");
    // fwrite(plaintext, 1, strlen(plaintext), f);
    // fclose(f);

    // std::string key = "key";
    // SHA256((const unsigned char*)key.c_str(), key.size(), keyhash);

    // encrypt("testfile", "key");
    // unsigned char buffer[96];
    // memset(buffer, 0, 64);
    // f = fopen("testfile.enc", "r");
    // decrypt(fileno(f), 96, keyhash, 0, 1, buffer);

    // unsigned char buffer2[3];
    // fclose(f);
    // f = fopen("testfile.enc", "r");
    // readEncImage(fileno(f), 96, keyhash, buffer2, 0, 3);

    // fclose(f);
    // generateImage("d0", "testimage");
    // f = fopen("testimage", "r");
    // unsigned char buf[688];
    // fread(buf, 1, 688, f);
    // fclose(f);
    // encrypt("testimage", "key");
    // f = fopen("testimage.enc", "r");
    // std::vector<Meta> metas2 = readEncMeta(fileno(f), 688, f, keyhash);
    // for (int i = 0; i < metas2.size(); i++) {
    //     std::cout << metas2[i].getName() << std::endl;
    // }
    // fclose(f);








    std::cout << "\n===================\nTest binary crypto\n===============\n";


    std::string key = "key";
    unsigned char keyhash[32];
    SHA256((const unsigned char*)key.c_str(), key.size(), keyhash);

    // f = fopen("d2/pack", "rb");
    // int l;
    // fseek(f, 0, SEEK_END);
    // l = ftell(f);
    // fseek(f, 0, SEEK_SET);
    // encrypt("d2/pack", "key");
    // fseek(f, 0, SEEK_SET);
    // unsigned char p[32];
    // fread(p, 1, 32, f);
    // unsigned char buffer4[32];
    // fclose(f);
    // f = fopen("d2/pack.enc", "rb");
    // SHA256((const unsigned char*)"key", 3, keyhash);
    // decrypt(fileno(f), l, keyhash, 0, 1, buffer4);
    // fclose(f);
    // //compare
    // for (int i = 0; i < 32; i++) {
    //     if (p[i] != buffer4[i]) {
    //         std::cout << "error\n";
    //         break;
    //     }
    // }



    std::cout << "\n===================\nTest compression\n===============\n";

    size_t block_size = 4096;
    unsigned char in[block_size];
    memset(in, 'a', block_size);
    unsigned char out[block_size];



    unsigned char compressed_data[block_size];
    size_t compressed_len = 0;
    unsigned char decompressed_data[block_size];
    size_t decompressed_len = 0;

    // Compress data
    if (compress(in, block_size, compressed_data, &compressed_len) == 0) {
        printf("Compressed data length: %zu\n", compressed_len);
        for (int i = 0; i < compressed_len; i++) {
            std::cout << compressed_data[i];
        }
        std::cout << std::endl;
    }
    else {
        fprintf(stderr, "Compression failed\n");
        return EXIT_FAILURE;
    }

    // Decompress data
    if (decompress(compressed_data, compressed_len, decompressed_data, &decompressed_len) == 0) {

        std::cout << "Decompressed data length: " << decompressed_len << std::endl;
        for (int i = 0; i < decompressed_len; i++) {
            std::cout << decompressed_data[i];
        }
        std::cout << std::endl;
    }
    else {
        fprintf(stderr, "Decompression failed\n");
        return EXIT_FAILURE;
    }


    std::cout << "\n===================\nTest genHelper w compress\n===============\n";


    generateImage("d1", "testImage");
    f = fopen("testImage", "r");
    encrypt("testImage", "key");
    fclose(f);
    f = fopen("testImage.enc", "r");
    fseek(f, 0, SEEK_END);
    size_t s = ftell(f);
    std::vector<Meta> metas3 = readEncMeta(fileno(f), s, f, keyhash);
    for (int i = 0; i < metas3.size(); i++) {
        std::cout << metas3[i].getName() << std::endl;
    }
    fclose(f);


    generateCompressedImage("d1", "testImage");
    f = fopen("testImage", "r");

    encrypt("testImage", "key");
    fclose(f);
    f = fopen("testImage.enc", "r");
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    std::vector<Meta> metas4 = readEncComMeta(fileno(f), size, f, keyhash);
    fclose(f);

    std::cout << "compressed\n";

    for (int i = 0; i < metas4.size(); i++) {
        std::cout << metas4[i].getName() << std::endl;
    }
    std::cout << "done testing\n" << std::endl;
    return 0;

}