#include "params.h"


#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#define UNSUPPORTED -1
#define VERSION "0.9: make image, mount, working fs"

#include "log.h"
#include "utils.hpp"


int blk_size;
TreeNode* root_node;
FILE* imageFile;
std::string imgPath;
bool encrypted = false;
int aes_blk_size = 32;
unsigned char keyhash[SHA256_DIGEST_LENGTH];

/**
 * creates a wofs image with files in path, stored in image_path
 * @param path path to directory
 * @param image_path path to image file
 * @return 0 on success, -1 on failure
*/
int wo_make_image(const char* path, const char* image_path) {
    // TODO: write an acutal implementation

    fprintf(stderr, "making image from %s to %s\n", path, image_path);
    generateImage(path, image_path);
    if (encrypted) {
        std::string key;
        std::cout << "Enter the key: ";
        if (system("stty -echo") != 0) {
            perror("Error: Cannot hide input\n");

        }
        std::cin >> key;
        if (system("stty echo") != 0) {
            perror("Error: Cannot show input\n");

        }
        std::cout << std::endl;
        encrypt(image_path, key);
        std::cout << "image encrypted as " << image_path << ".enc\n";
        remove(image_path);
    }
    encrypted = false; //reset encryption flag
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

/*
    filler functions end here
*/

/** Get file attributes.
 */
 // TODO: implement
int wo_getattr(const char* path, struct stat* statbuf) {
    log_msg("getattr: %s\n", path);
    TreeNode* t = root_node->find(path);
    if (t == nullptr) {
        log_msg("not found.\n");
        return -ENOENT;
    }
    else {
        log_msg("found: %s\n", t->getMeta().getName().c_str());
    }
    Meta m = t->getMeta();
    int p = m.getPermission();

    statbuf->st_mtime = m.getLastModified();
    if (m.isDirectory()) {

        statbuf->st_mode = S_IFDIR | p;
        statbuf->st_size = blk_size;

    }
    else {
        statbuf->st_mode = S_IFREG | p;
        statbuf->st_size = m.getSize();

    }
    statbuf->st_nlink = 1;

    return 0;

}

int wo_opendir(const char* path, struct fuse_file_info* fi) {
    log_msg("opendir: %s\n", path);
    return 0;

}

int wo_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    if (encrypted) {
        log_msg("encrypted wo_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
            path, buf, size, offset, fi);
    }
    else {
        log_msg("wo_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
            path, buf, size, offset, fi);
    }
    TreeNode* found = root_node->find(path);
    if (!found) {
        return -ENOENT;
    }
    long f_size = found->getMeta().getSize();
    if (offset > f_size) {
        return 0;
    }

    long s = (long)size > f_size - offset ? f_size - offset : size;
    memset(buf, 0, size);
    log_msg("   Found file: start: %ld, size: %ld\n", found->getMeta().getStart(), f_size);

    if (encrypted) {
        log_msg("reading encrypted file\n   keyhash: ");
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            log_msg("%02x", keyhash[i]);
        }
        log_msg("\n");
        log_msg("pos of buffer: %p\n", buf);
        log_msg("offset: %ld\n", found->getMeta().getStart() + offset);
        log_msg("size: %ld\n", s);
        int ofs = found->getMeta().getStart() + offset;

        size_t res = readEncImage(imageFile, keyhash, (unsigned char*)buf, ofs, s);
        log_msg("readEncImage done: read %ld bytes\n", s);
        std::cout << "read " << res << " bytes from position " << ofs << std::endl;
        return res;
    }
    fseek(imageFile, found->getMeta().getStart() + offset, SEEK_SET);

    size_t r = fread(buf, 1, s, imageFile);
    std::cout << "read " << r << " bytes\n";
    return r;
    // log_msg("%s\n",buf);
    // return found->getMeta().getSize();
}

int wo_readdir(const char* path,
    void* buf,
    fuse_fill_dir_t filler,
    off_t offset,
    struct fuse_file_info* fi) {

    log_msg("readdir: %s\n", path);

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    log_msg("looking for child\n");


    TreeNode* curr = root_node->find(path);

    std::vector<TreeNode*> children = curr->getAllChildren();


    for (std::vector<TreeNode*>::iterator it = children.begin(); it != children.end(); it++) {
        log_msg("found child: %s\n", (*it)->getMeta().getName().c_str());

        std::string name_wo_dir = (*it)->getMeta().getName().substr((*it)->getMeta().getName().find_last_of("/") + 1);
        filler(buf, name_wo_dir.c_str(), NULL, 0);
    }
    return 0;
}

void* wo_init(struct fuse_conn_info* conn) {
    log_conn(conn);
    log_fuse_context(fuse_get_context());
    return WO_DATA;
}

void wo_destroy(void* userdata) {
    log_msg("\nbb_destroy(userdata=0x%08x)\n", userdata);
}

int wo_access(const char* path, int mask) {
    return 0;
}

int wo_fgetattr(const char* path, struct stat* statbuf, struct fuse_file_info* fi) {
    return 0;
}

struct fuse_operations wo_oper = {
 .getattr = wo_getattr,
  .readlink = NULL,
  // no .getdir -- that's deprecated
  .getdir = NULL,
  .mknod = wo_mknod,
  .mkdir = wo_mkdir,
  .unlink = wo_unlink,
  .rmdir = wo_rmdir,
  .symlink = wo_symlink,
  .rename = wo_rename,
  .link = wo_link,
  .chmod = wo_chmod,
  .chown = wo_chown,
  .truncate = wo_truncate,
  //   .utime = wo_utime,
  //   .open = wo_open,
  //   .read = wo_read,
  //   .write = wo_write,

  // filler functions
      .utime = NULL,
      .open = NULL,
      .read = wo_read,
      .write = NULL,
      /** Just a placeholder, don't set */ // huh???

    //   .statfs = wo_statfs,
    //   .flush = wo_flush,
    //   .release = wo_release,
    //   .fsync = wo_fsync,
// more filler functions
    .statfs = NULL,
    .flush = NULL,
    .release = NULL,
    .fsync = NULL,

    #ifdef HAVE_SYS_XATTR_H
      .setxattr = wo_setxattr,
      .getxattr = wo_getxattr,
      .listxattr = wo_listxattr,
      .removexattr = wo_removexattr,
    #endif

      // .opendir = wo_opendir,
      //   .readdir = wo_readdir,
        //   .releasedir = wo_releasedir,
        //   .fsyncdir = wo_fsyncdir,
      //   .init = wo_init,
        //   .destroy = wo_destroy,
        //   .access = wo_access,
        //   .ftruncate = wo_ftruncate,
        //   .fgetattr = wo_fgetattr

    // even more filler functions
        .opendir = wo_opendir,
        .readdir = wo_readdir,
        .releasedir = NULL,
        .fsyncdir = NULL,
        .init = wo_init,
        .destroy = wo_destroy,
        .access = wo_access,
        .ftruncate = NULL,
        .fgetattr = wo_fgetattr
};

void wo_gen_usage() {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  wofs gen [options] <directory> <image file>    Generate an image\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -h,  --help           Print this help\n");
    fprintf(stderr, "  -e,  --encrypt        Use password for image generation\n");
}

void wo_mount_usage() {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  wofs mount [options] <image file> <mount point> Mount an image\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -h,  --help           Print this help\n");
    fprintf(stderr, "  -d,  --decrypt        Use password for image generation\n");
}

void wo_usage() {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  -h  --help            Print this help\n");
    fprintf(stderr, "  -v  --version         Print version information\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  gen                   Generate an wofs image from a directory\n");
    fprintf(stderr, "  mount                 Mount an wofs image to a directory\n");
    // Add more option descriptions here
}

int handle_gen_command(int argc, char* argv[]) {
    if (argc < 4) { // Basic argument count check, might need adjustment based on actual option requirements
        wo_gen_usage();
        return EXIT_FAILURE;
    }
    for (int i = 2; i < argc - 2; i++) { // Skip command and last two arguments (directory and image file)
        if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--encrypt") == 0) {
            encrypted = true; 
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            wo_gen_usage();
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "Unknown or unsupported option: %s\n", argv[i]);
            wo_gen_usage();
            return EXIT_FAILURE;
        }
    }

    // Extract directory and image file from the last two arguments
    char* directory = argv[argc - 2];
    char* imageFilePath = argv[argc - 1];

    // Info
    printf("Generating image from directory '%s' to file '%s'. Encryption: %s\n",
           directory, imageFilePath, encrypt ? "enabled" : "disabled");
    wo_make_image(directory, imageFilePath);
    return EXIT_SUCCESS;
}

int handle_mount_command(int argc, char* argv[]) {
    if (argc < 4) { // Basic argument count check
        wo_mount_usage();
        return EXIT_FAILURE;
    }
    argc--;
    for(int j=1; j<argc; j++) {
        argv[j] = argv[j+1];
    }
    argv[argc] = NULL;
    for (int i = 1; i < argc - 2; i++) { // Skip command and last two arguments (image file and mount point)
        if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--encrypt") == 0) {
            encrypted = true; 
            argc--;
            for(int j = i; j < argc; j++) {
                argv[j] = argv[j + 1];
            }
            argv[argc] = NULL;
        } 
    }

    // Extract image file and mount point from the last two arguments
    char* imageFilePath = argv[argc - 2];
    char* mountPoint = argv[argc - 1];

    int fuse_stat;
    struct wo_state* wo_data = (struct wo_state*)malloc(sizeof(struct wo_state));
    if (wo_data == NULL) {
        perror("main malloc error");
        abort();
    }

    wo_data->rootdir = realpath(argv[argc - 2], NULL);

    argv[argc - 2] = argv[argc - 1];
    argv[argc - 1] = NULL;
    argc--;



    wo_data->logfile = log_open();


    struct stat* st = (struct stat*)malloc(sizeof(struct stat));
    stat(wo_data->rootdir, st);
    blk_size = st->st_size;

    std::string key;
    if (encrypted) {
        std::cout << "Enter the key: ";
        // hide input 
        if (system("stty -echo") != 0) {
            perror("Error: Cannot hide input\n");

        }
        std::cin >> key;
        if (system("stty echo") != 0) {
            perror("Error: Cannot show input\n");

        }
        std::cout << std::endl;
        SHA256((unsigned char*)key.c_str(), key.size(), keyhash);
    }

    std::vector<Meta> metaList;
    if (encrypted) {
        SHA256((unsigned char*)key.c_str(), key.size(), keyhash);
        imageFile = fopen(wo_data->rootdir, "rb");
        try {
            metaList = readEncMeta(imageFile, keyhash);
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        imageFile = fopen(wo_data->rootdir, "r");
    }
    else {
        imageFile = fopen(wo_data->rootdir, "rb");
        if (imageFile == nullptr) {
            perror("Error: Cannot open the image file for file system\n");
            return EXIT_FAILURE;
        }
        metaList = readAllMeta(imageFile);
    }
    std::cout << "Found " << metaList.size() << " meta data\n";
    root_node = generateTree(metaList);
    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main\n");
    for(int i=0;i<argc;i++) {
        std::cout << argv[i] << " ";
    } 
    std::cout << std::endl;
    fuse_stat = fuse_main(argc, argv, &wo_oper, wo_data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);


    // dummy root 
    // Info
    printf("Mounting image file '%s' to mount point '%s'. Decryption: %s\n",
           imageFilePath, mountPoint, decrypt ? "enabled" : "disabled");

    return fuse_stat;
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

    else if (strcmp(argv[1], "gen") == 0) {
        return handle_gen_command(argc, argv);
    } else if (strcmp(argv[1], "mount") == 0) {
        return handle_mount_command(argc, argv);
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        wo_usage();
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {

    if ((getuid() == 0) || (geteuid() == 0)) {
        fprintf(stderr, "Running wofs as root opens unnacceptable security holes. Exiting\n");
        return EXIT_FAILURE;
    }

    if (wo_options(argc, argv) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}