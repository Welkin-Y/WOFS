#include "params.h"


#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#define UNSUPPORTED -1
#define VERSION "0.3: yay, progress! don't ask if it's correct though"

#include "log.h"
#include "utils.hpp"


int blk_size;
TreeNode* root_node;
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
    if (m.isDirectory()) {
        statbuf->st_mode = S_IFDIR | 0755;
        statbuf->st_size = blk_size;
    }
    else {
        statbuf->st_mode = S_IFREG | 0777;
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
    log_msg("read: %s\n", path);

    return 0;
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
        wo_make_image(argv[2], argv[3]);
        return EXIT_FAILURE;
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
    
    // Meta root = Meta("", -1, 3200, 0777, 0, 0, true);
    // fprintf(stderr, "root: %s\n", root.getName().c_str());
    // root_node = new TreeNode(root);
    // Meta f1 = Meta("file1", -1, 400, 0777, 0, 0, false);
    // TreeNode* f1_node = new TreeNode(f1);
    // root_node->setChild(f1_node);

    // Meta f2 = Meta("file2", -1, 400, 0777, 0, 0, false);
    // TreeNode* f2_node = new TreeNode(f2);
    // f1_node->setSibling(f2_node);

    // Meta f3 = Meta("dir1", -1, 720, 0777, 0, 0, true);
    // TreeNode* f3_node = new TreeNode(f3);
    // f2_node->setSibling(f3_node);

    // Meta f4 = Meta("dir1/file3", -1, 400, 0777, 0, 0, false);
    // TreeNode* f4_node = new TreeNode(f4);
    // f3_node->setChild(f4_node);
    FILE* f = fopen(wo_data->rootdir, "r");
    if(f==nullptr){
        perror("Error: Cannot open the image file for file system\n");
        return EXIT_FAILURE;
    }
    std::vector<Meta> metaList= readAllMeta(f);
    std::cout << "Found "<<metaList.size()<<" meta data\n";
    root_node = generateTree(metaList);
    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main\n");
    fuse_stat = fuse_main(argc, argv, &wo_oper, wo_data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);


    // dummy root 

    return fuse_stat;
}