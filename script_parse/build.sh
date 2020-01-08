cp ../filelist/file_list.h .
g++ main.c str_utils.c --std=c++11 -O3
rm -rf file_list.h
