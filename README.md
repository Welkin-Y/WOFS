# Write Once Filesystem (WOFS)

## how to use
+ To mount: wofs [options] <image file> <mount point>
+ To generate image: wofs -g <directory> <image file>
done (for now)
## progress: 
+ meta tree
+ fixed meta tree on mounting the fs
+ traverse meta tree to find files and dirs
+ make image files
+ read metas from image files
+ full meta information in Meta class
+ create meta tree from image file
+ find file content in image by size and offset 
+ correct permissions, mtime
+ check if generate's first param is a directory

## TODO:
+ root directory's permission and mtime still set arbitrarily

