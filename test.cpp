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
    return 0;

}