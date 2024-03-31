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

    // unsigned char iv[16];
    // RAND_bytes(iv, 16);
    // size_t iv_len = 16;

    // encrypt(unsigned char* plaintext, int plaintext_len, unsigned char* aad,
    //     int aad_len, unsigned char* key, unsigned char* iv, int iv_len,
    //     unsigned char* ciphertext, unsigned char* tag);

    // int decrypt(unsigned char* ciphertext, int ciphertext_len,
    //     unsigned char* aad, int aad_len, unsigned char* tag,
    //     unsigned char* key, unsigned char* iv, int iv_len,
    //     unsigned char* plaintext);

    // char plaintext[] = "hello world";
    // unsigned char key[32];
    // RAND_bytes(key, 32);
    // unsigned char tag[16];
    // unsigned char ciphertext[100];

    // gcm_encrypt((unsigned char*)plaintext, strlen(plaintext), NULL, 0, key, iv, iv_len, ciphertext, tag);
    // std::cout << "ciphertext: " << ciphertext << std::endl;
    // unsigned char decrypted[100];
    // int l = gcm_decrypt(ciphertext, strlen(plaintext), NULL, 0, tag, key, iv, iv_len, decrypted);
    // std::cout << "decrypted: " << decrypted << std::endl;
    // std::cout << "length: " << l << std::endl;


    // generateImage("d0", "testImage");
    // int imagelen = 0;
    // FILE* image = fopen("testImage", "r");
    // fseek(image, 0, SEEK_END);
    // imagelen = ftell(image);
    // unsigned char* buffer = (unsigned char*)malloc(imagelen);
    // fseek(image, 0, SEEK_SET);
    // fread(buffer, 1, imagelen, image);
    // encryption(buffer, imagelen, "key", "testImage");
    // fclose(image);
    // std::cout << "lolololo" << std::endl;

    // std::cout << "reading encrypted meta:" << std::endl;
    // FILE* ffff = fopen("testImage", "r");
    // std::vector<Meta> emetas = decryption_read_meta(ffff, "key");
    // for (std::vector<Meta>::iterator it = emetas.begin(); it != emetas.end(); it++) {
    //     fprintf(stderr, "name: %s\n", it->getName().c_str());
    //     fprintf(stderr, "size: %ld\n", it->getSize());
    //     fprintf(stderr, "start: %ld\n", it->getStart());
    //     fprintf(stderr, "permission: %d\n", it->getPermission());
    //     fprintf(stderr, "owner: %d\n", it->getOwner());
    //     fprintf(stderr, "group: %d\n", it->getGroup());
    //     fprintf(stderr, "isDir: %d\n", it->isDirectory());

    // }

    unsigned char iv[16];
    memset(iv, 0, 16);
    size_t iv_len = 16;
    memcpy(iv, iv, iv_len);
    std::cout << std::endl << "iv: ";
    for (int i = 0; i < iv_len; i++) {
        // hex 
        std::cout << std::hex << (int)iv[i];
    }
    std::cout << std::endl;


    char plaintext[] = "hello, my name is Mr. plaintext. and I am definitely more than 16 bytes long";
    unsigned char key[32];
    RAND_bytes(key, 32);
    unsigned char tag[16];
    unsigned char ciphertext[100];
    gcm_encrypt((unsigned char*)plaintext, strlen(plaintext), NULL, 0, key, iv, iv_len, ciphertext, tag);
    std::cout << "ciphertext: " << std::endl;
    for (int i = 0; i < strlen(plaintext); i++) {
        printf("%02x", ciphertext[i]);
    }
    std::cout << std::endl;
    // partially decrypt
    unsigned char decrypted[100];
    //int l = gcm_seek_decrypt(ciphertext, strlen(plaintext), NULL, 0, tag, key, iv, iv_len, decrypted, 1, 1);
    //std::cout << "decrypted: " << decrypted << std::endl;
    int l = gcm_decrypt(ciphertext, strlen(plaintext), NULL, 0, tag, key, iv, iv_len, decrypted);
    std::cout << "decrypted: " << decrypted << std::endl;
    return 0;

}