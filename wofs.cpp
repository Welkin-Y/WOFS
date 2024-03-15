#include "params.h"
#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#define UNSUPPORTED -1
#define VERSION "0.1: nothing to see"

#include "log.h"
#include "utils.hpp"



/**
 * creates a wofs image with files in path, stored in image_path
 * @param path path to directory
 * @param image_path path to image file
 * @return 0 on success, -1 on failure
*/
int wo_make_image(const char* path, const char* image_path) {
    // TODO: write an acutal implementation

    fprintf(stderr, "making image from %s to %s\n", path, image_path);

    std::vector<Meta> files = allFiles(path);
    fprintf(stderr, "Files:\n");
    for (std::vector<Meta>::iterator it = files.begin(); it != files.end(); it++) {
        fprintf(stderr, "%s\n", it->getName().c_str());
        fprintf(stderr, "size: %ld\n", it->getSize());
    }
    return EXIT_SUCCESS;
}


/*
    following functions return -1 to indicate unsupported operation
*/

int wo_mknod(const char* path, mode_t mode, dev_t dev) {
    return UNSUPPORTED;
}

int wo_mkdir(const char* path, mode_t mode) {
    return UNSUPPORTED;
}

int wo_unlink(const char* path) {
    return UNSUPPORTED;
}

int wo_rmdir(const char* path) {
    return UNSUPPORTED;
}

int wo_rename(const char* path, const char* newpath) {
    return UNSUPPORTED;
}

int wo_chmod(const char* path, mode_t mode) {
    return UNSUPPORTED;
}

int wo_chown(const char* path, uid_t uid, gid_t gid) {
    return UNSUPPORTED;
}

int wo_link(const char* path, const char* link) {
    return UNSUPPORTED;
}

int wo_symlink(const char* path, const char* link) {
    return UNSUPPORTED;
}

int wo_truncate(const char* path, off_t size) {
    return UNSUPPORTED;
}







int wo_usage() {
    fprintf(stderr, "To mount: wofs [options] <image file> <mount point>\n");
    fprintf(stderr, "To generate image: wofs -g <directory> <image file>\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -h  --help            Print help\n");
    fprintf(stderr, "  -v  --version         Print version\n");
    return EXIT_SUCCESS;
}

int wo_options(int argc, char* argv[]) {
    if (argc < 2) {
        wo_usage();
        return EXIT_FAILURE;
    }
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        wo_usage();
        return EXIT_FAILURE;
    }
    if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
        fprintf(stderr, "wofs version %s\n", VERSION);
        return EXIT_FAILURE;
    }

    // other options
    if (strcmp(argv[1], "-g") == 0) {
        if (argc < 4) {
            wo_usage();
            return EXIT_FAILURE;
        }
        return wo_make_image(argv[2], argv[3]);
    }
    // no options
    return EXIT_SUCCESS;
}


int main(int argc, char* argv[]) {

    if ((getuid() == 0) || (geteuid() == 0)) {
        fprintf(stderr, "Running wofs as root opens unnacceptable security holes. Exiting\n");
        return EXIT_FAILURE;
    }

    if (wo_options(argc, argv) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }


}