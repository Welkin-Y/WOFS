#include <stdio.h>
#include <dirent.h>
#include <string.h>
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
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <iostream>
#include <vector>
#include <string>
void print_dir(std::string directory){
    DIR* dir;
    struct dirent* ent;
    struct stat st;
    FILE* toWrite;
    dir = opendir(directory.c_str());
    while ((ent = readdir(dir)) != NULL){
        std::cout << ent->d_name<<"\n";
        stat((directory + "/" + ent->d_name).c_str(), &st);
        if (ent->d_type == DT_DIR){
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0){
                // toWrite = fopen((directory + "/" + ent->d_name).c_str(), "r");
                // char* buffer = new char[st.st_size];
                // fread(buffer, 1, st.st_size, toWrite);
                // std::cout <<"size: "<<st.st_size<<"  "<< buffer<<"done\n";
                // delete buffer;
                print_dir(directory+'/'+ent->d_name);
            }
        }
    }
}
int try_main() {
    // std::string directory();
    // print_dir("/home/xc139/try");
    

    return 0;
}