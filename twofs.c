
#include "params.h"

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
#include <sys/types.h>

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#include "log.h"

int blk_size = 0;

/*
static void bb_fullpath(char fpath[PATH_MAX], const char *path)
{
    strcpy(fpath, BB_DATA->rootdir);
    strncat(fpath, path, PATH_MAX); // ridiculously long paths will
				    // break here

    //    log_msg("    bb_fullpath:  rootdir = \"%s\", path = \"%s\", fpath = \"%s\"\n",
    //	    BB_DATA->rootdir, path, fpath);
}
*/

/** Get file attributes.
 */
int bb_getattr(const char *path, struct stat *statbuf)
{
    
    // log_msg("\nbb_getattr(path=\"%s\", statbuf=0x%08x)\n",
    //	  path, statbuf);
    
    if (strcmp(path, "/") == 0) {
      //log_msg("peeking root dir ...\n");
      statbuf->st_mode = S_IFDIR | 0755;
      statbuf->st_nlink = 1;
      statbuf->st_size = blk_size;      
      // log_msg("allowed.\n");
      return 0;
    }
    else if (strncmp(path, "/file1", 6) == 0) {
      statbuf->st_mode = S_IFREG | 0666;
      statbuf->st_nlink = 1;
      statbuf->st_size = blk_size / 2;
      log_msg("file size: %d\n", statbuf->st_size);
      return 0;
    }
    else if (strncmp(path, "/file2", 6) == 0) {
      statbuf->st_mode = S_IFREG | 0666;
      statbuf->st_nlink = 1;
      statbuf->st_size = blk_size / 2;
      log_msg("file size: %d\n", statbuf->st_size);
      return 0;
    }
    else {
      return -2;
    }

    return -2;
}


/*
 * following functions fail because involving other files / changing the two files
 */

int bb_mknod(const char *path, mode_t mode, dev_t dev)
{
  return -2;
}


/** Create a directory */
int bb_mkdir(const char *path, mode_t mode)
{
  return -2;
}

/** Remove a file */
int bb_unlink(const char *path)
{
  return -2;
}

/** Remove a directory */
int bb_rmdir(const char *path)
{
  return -2;
}

/** Create a symbolic link */
int bb_symlink(const char *path, const char *link)
{
  return -2;
}

/** Rename a file */
// both path and newpath are fs-relative
int bb_rename(const char *path, const char *newpath)
{
  return -2;
}

/** Create a hard link to a file */
int bb_link(const char *path, const char *newpath)
{
  return -2;
}

/** Change the permission bits of a file */
int bb_chmod(const char *path, mode_t mode)
{
  return -2;
}

/** Change the owner and group of a file */
int bb_chown(const char *path, uid_t uid, gid_t gid)
{
  return -2;
}

/** Change the size of a file */
// does nothing if is f1 or f2, return -2 else
int bb_truncate(const char *path, off_t newsize)
{
    if (strncmp(path, "/file1", 6) == 0) {
      return 0;
    }
    else if (strncmp(path, "/file2", 6) == 0) {
      return 0;
    }
    else {
      return -2;
    }
}

/** Change the access and/or modification times of a file */
// you can't do that 
int bb_utime(const char *path, struct utimbuf *ubuf)
{
  return -2;
}

/** File open operation
 */
int bb_open(const char *path, struct fuse_file_info *fi)
{
  log_msg("calling open...\n");
    int fd;
    //   char fpath[PATH_MAX];
    
    //log_msg("\nbb_open(path\"%s\", fi=0x%08x)\n",
    //	    path, fi);
   
    if (strncmp(path, "/file1", 6) == 0 || strncmp(path, "/file2", 6) == 0) {
      fd = open(BB_DATA->rootdir, fi->flags);
      if (fd == -1) {
	
	return -1;
      }
      fi->fh = fd;
      log_msg("opening with file descriptor %d\n", fd);
      return 0;
    }
    else if (strcmp(path, "/") == 0) {
      return 0;
    }
    else {
      return -2;
    }
}

/** Read data from an open file
 */
int bb_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
  //log_msg("\nbb_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
  //	    path, buf, size, offset, fi);
     //log_fi(fi);
    
    if (strncmp(path, "/file2", 6) == 0) {
      offset += blk_size / 2;
      
    }
    int rd = pread(fi->fh, buf, size, offset);
    if (rd == -1) {
      log_msg("read gave error code %d\n", errno);
    }
    return rd;
}

/** Write data to an open file
 */
int bb_write(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi)
{
    int oldsize = size;
    // log_msg("\nbb_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
    //	    path, buf, size, offset, fi
    //	    );
    //log_fi(fi);
    log_msg("attempting to write %s to %s\n", buf, path);

    if (strncmp(path, "/file2", 6) == 0) {
      offset += blk_size / 2;
      /*if (size + offset > blk_size) {
	log_msg("attempting to write more than determined size of file...\nreducing write size from %d to %d.\n", size, blk_size / 2);
	size = blk_size - offset;
	}*/
    }
    /*
    else if (size + offset > blk_size / 2) {
      log_msg("attempting to write more than determined size of file...\nreducing write size from %d to %d.\n", size, blk_size / 2);
      size = blk_size / 2 - offset;
      
      log_msg("now size is %d.\n", size);
    }
    */
    int p = pwrite(fi->fh, buf, size, offset);
    if (p == -1) {
      log_msg("write got error number %d\n", errno);
      return -1;
    }
    return p;
}


int bb_statfs(const char *path, struct statvfs *statv)
{
  return 0;

}

int bb_flush(const char *path, struct fuse_file_info *fi)
{	
    return 0;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 * Changed in version 2.2
 */
int bb_release(const char *path, struct fuse_file_info *fi)
{
  //    log_msg("\nbb_release(path=\"%s\", fi=0x%08x)\n",
  //	  path, fi);
  // log_fi(fi);

    // We need to close the file.  Had we allocated any resources
    // (buffers etc) we'd need to free them here as well.

  return log_syscall("close", close(fi->fh), 0);
}

/** Synchronize file contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data.
 *
 * Changed in version 2.2
 */
int bb_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
    log_msg("\nbb_fsync(path=\"%s\", datasync=%d, fi=0x%08x)\n",
	    path, datasync, fi);
    log_fi(fi);
    
    // some unix-like systems (notably freebsd) don't have a datasync call
#ifdef HAVE_FDATASYNC
    if (datasync)
	return log_syscall("fdatasync", fdatasync(fi->fh), 0);
    else
#endif	
	return log_syscall("fsync", fsync(fi->fh), 0);
}

#ifdef HAVE_SYS_XATTR_H
/** Note that my implementations of the various xattr functions use
    the 'l-' versions of the functions (eg bb_setxattr() calls
    lsetxattr() not setxattr(), etc).  This is because it appears any
    symbolic links are resolved before the actual call takes place, so
    I only need to use the system-provided calls that don't follow
    them */

/** Set extended attributes */
int bb_setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{
    char fpath[PATH_MAX];
    
    log_msg("\nbb_setxattr(path=\"%s\", name=\"%s\", value=\"%s\", size=%d, flags=0x%08x)\n",
	    path, name, value, size, flags);
    bb_fullpath(fpath, path);

    return log_syscall("lsetxattr", lsetxattr(fpath, name, value, size, flags), 0);
}

/** Get extended attributes */
int bb_getxattr(const char *path, const char *name, char *value, size_t size)
{
    int retstat = 0;
    char fpath[PATH_MAX];
    
    log_msg("\nbb_getxattr(path = \"%s\", name = \"%s\", value = 0x%08x, size = %d)\n",
	    path, name, value, size);
    bb_fullpath(fpath, path);

    retstat = log_syscall("lgetxattr", lgetxattr(fpath, name, value, size), 0);
    if (retstat >= 0)
	log_msg("    value = \"%s\"\n", value);
    

    return retstat;
}

/** List extended attributes */
int bb_listxattr(const char *path, char *list, size_t size)
{
    int retstat = 0;
    char fpath[PATH_MAX];
    char *ptr;
    
    log_msg("\nbb_listxattr(path=\"%s\", list=0x%08x, size=%d)\n",
	    path, list, size
	    );
    bb_fullpath(fpath, path);

    retstat = log_syscall("llistxattr", llistxattr(fpath, list, size), 0);
    if (retstat >= 0) {
	log_msg("    returned attributes (length %d):\n", retstat);
	if (list != NULL)
	    for (ptr = list; ptr < list + retstat; ptr += strlen(ptr)+1)
		log_msg("    \"%s\"\n", ptr);
	else
	    log_msg("    (null)\n");
    }
    
    return retstat;
}

/** Remove extended attributes */
int bb_removexattr(const char *path, const char *name)
{
    char fpath[PATH_MAX];
    
    log_msg("\nbb_removexattr(path=\"%s\", name=\"%s\")\n",
	    path, name);
    bb_fullpath(fpath, path);

    return log_syscall("lremovexattr", lremovexattr(fpath, name), 0);
}
#endif

/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this  directory
 *
 * Introduced in version 2.3
 */
int bb_opendir(const char *path, struct fuse_file_info *fi)
{

  if (strcmp(path, "/") == 0) {
    return 0;
  }
  return -2;
}

/** Read directory
  */

int bb_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
	       struct fuse_file_info *fi)
{

  if (strcmp(path, "/") == 0) {
    //log_msg("accessing root dir ... \n");
    filler(buf, "file1", NULL, 0);
    filler(buf, "file2", NULL, 0);
    return 0;
  }
  return -2;

}

/** Release directory
 *
 * Introduced in version 2.3
 */
int bb_releasedir(const char *path, struct fuse_file_info *fi)
{
  log_msg("calling release dir...\n");
  return -2;
}

/** Synchronize directory contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data
 *
 * Introduced in version 2.3
 */
int bb_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{
    int retstat = 0;
    
    log_msg("\nbb_fsyncdir(path=\"%s\", datasync=%d, fi=0x%08x)\n",
	    path, datasync, fi);
    log_fi(fi);
    
    return retstat;
}

/**
 * Initialize filesystem
 *
 * The return value will passed in the private_data field of
 * fuse_context to all file operations and as a parameter to the
 * destroy() method.
 *
 * Introduced in version 2.3
 * Changed in version 2.6
 */
void *bb_init(struct fuse_conn_info *conn)
{
  //log_msg("\nbb_init()\n");
    
    log_conn(conn);
    log_fuse_context(fuse_get_context());
    
    return BB_DATA;
}

/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 * Introduced in version 2.3
 */
void bb_destroy(void *userdata)
{
    log_msg("\nbb_destroy(userdata=0x%08x)\n", userdata);
}

/**
 * Check file access permissions
 *
 * This will be called for the access() system call.  If the
 * 'default_permissions' mount option is given, this method is not
 * called.
 *
 * This method is not called under Linux kernel versions 2.4.x
 *
 * Introduced in version 2.5
 */
int bb_access(const char *path, int mask)
{
  int res = access(path, mask);
  if (res == -1) {
    return -errno;
  }
  return 0;
}

/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the mknod() and open() methods
 * will be called instead.
 *
 * Introduced in version 2.5
 */
// Not implemented.  I had a version that used creat() to create and
// open the file, which it turned out opened the file write-only.

/**
 * Change the size of an open file
 *
 * This method is called instead of the truncate() method if the
 * truncation was invoked from an ftruncate() system call.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the truncate() method will be
 * called instead.
 *
 * Introduced in version 2.5
 */
int bb_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi)
{
  
  int retstat = 0;
    
    log_msg("\nbb_ftruncate(path=\"%s\", offset=%lld, fi=0x%08x)\n",
	    path, offset, fi);
    log_fi(fi);
    
    retstat = ftruncate(fi->fh, offset);
    if (retstat < 0)
	retstat = log_error("bb_ftruncate ftruncate");
    
    return retstat;
}

/**
 * Get attributes from an open file
 *
 * This method is called instead of the getattr() method if the
 * file information is available.
 *
 * Currently this is only called after the create() method if that
 * is implemented (see above).  Later it may be called for
 * invocations of fstat() too.
 *
 * Introduced in version 2.5
 */
int bb_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
    int retstat = 0;
    
    log_msg("\nbb_fgetattr(path=\"%s\", statbuf=0x%08x, fi=0x%08x)\n",
	    path, statbuf, fi);
    log_fi(fi);

    // On FreeBSD, trying to do anything with the mountpoint ends up
    // opening it, and then using the FD for an fgetattr.  So in the
    // special case of a path of "/", I need to do a getattr on the
    // underlying root directory instead of doing the fgetattr().
    if (!strcmp(path, "/"))
	return bb_getattr(path, statbuf);
    
    retstat = fstat(fi->fh, statbuf);
    if (retstat < 0)
	retstat = log_error("bb_fgetattr fstat");
    
    log_stat(statbuf);
    
    return retstat;
}

struct fuse_operations bb_oper = {
  .getattr = bb_getattr,
  .readlink = NULL,
  // no .getdir -- that's deprecated
  .getdir = NULL,
  .mknod = bb_mknod,
  .mkdir = bb_mkdir,
  .unlink = bb_unlink,
  .rmdir = bb_rmdir,
  .symlink = bb_symlink,
  .rename = bb_rename,
  .link = bb_link,
  .chmod = bb_chmod,
  .chown = bb_chown,
  .truncate = bb_truncate,
  .utime = bb_utime,
  .open = bb_open,
  .read = bb_read,
  .write = bb_write,
  /** Just a placeholder, don't set */ // huh???
  .statfs = bb_statfs,
  .flush = bb_flush,
  .release = bb_release,
  .fsync = bb_fsync,
  
#ifdef HAVE_SYS_XATTR_H
  .setxattr = bb_setxattr,
  .getxattr = bb_getxattr,
  .listxattr = bb_listxattr,
  .removexattr = bb_removexattr,
#endif
  
  .opendir = bb_opendir,
  .readdir = bb_readdir,
  .releasedir = bb_releasedir,
  .fsyncdir = bb_fsyncdir,
  .init = bb_init,
  .destroy = bb_destroy,
  .access = bb_access,
  .ftruncate = bb_ftruncate,
  .fgetattr = bb_fgetattr
};

void bb_usage()
{
    fprintf(stderr, "usage:  bbfs [FUSE and mount options] rootDir mountPoint\n");
    abort();
}

int main(int argc, char *argv[])
{
    int fuse_stat;
    struct bb_state *bb_data;

    if ((getuid() == 0) || (geteuid() == 0)) {
    	fprintf(stderr, "Running BBFS as root opens unnacceptable security holes\n");
    	return 1;
    }

    // See which version of fuse we're running
    fprintf(stderr, "Fuse library version %d.%d\n", FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);
    
    if ((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-'))
	bb_usage();

    bb_data = malloc(sizeof(struct bb_state));
    if (bb_data == NULL) {
	perror("main calloc");
	abort();
    }

    bb_data->rootdir = realpath(argv[argc-2], NULL);
    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;
    
    bb_data->logfile = log_open();


    struct stat * st = malloc(sizeof(struct stat));
    stat(bb_data->rootdir, st);
    blk_size = st->st_size;

    
    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main\n");
    fuse_stat = fuse_main(argc, argv, &bb_oper, bb_data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);
    
    return fuse_stat;
}
