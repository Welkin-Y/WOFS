# Write Once Filesystem (WOFS)

## Authors

*TBD*

## Project Structure

```
.
├── Makefile
├── README.md
├── run_test.sh
├── src
│   ├── log.c
│   ├── log.h
│   ├── params.h
│   ├── utils.cpp
│   ├── utils.hpp
│   └── wofs.cpp
└── test
    ├── test.cpp
    └── try.cpp
```
## Test Environment
+ Ubuntu 22.04.4 LTS

## Compile project
+ `sudo apt-get install libssl-dev -y`
+ `make`
+ `./build/wofs`

## Run test
+ test file system
+ `sh fs_test.sh`
+ test encryption
+ `sh encrypt_test.sh`

## How to use

+ To mount: wofs mount [options] \<image file> \<mount point>
+ To generate image: wofs gen [options] \<directory> \<image file>
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

### encryption

+ encrypt an image 
+ decrypt the image and build directory tree
+ read file content from encrypted image 
+ seekable random access

### compression

### issues

CI pipeline

## Acknowledgements
*TBD*

