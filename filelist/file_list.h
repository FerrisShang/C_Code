#ifndef __FILE_LIST_H__
#define __FILE_LIST_H__
#include <dirent.h>
#include <vector>
#include <map>
#include <string>
#include <stdio.h>
#include <assert.h>
using namespace std;

class CFileList{
	vector<string> fileList;
	map<string, string> fileMap;
	void initFileList(const char *path = (char*)".") {
		DIR *dir;
		struct dirent *ent;
		if ((dir = opendir (path)) != NULL) {
			while ((ent = readdir (dir)) != NULL) {
				if(strcmp(".", ent->d_name) && strcmp("..", ent->d_name)){
					auto filePath = string(path) + string("/") + string(ent->d_name);
					fileList.push_back(filePath);
						if(fileMap.count(string(ent->d_name))){
							printf("Filename \"%s\" redefined is NOT allowed!\n", ent->d_name);
							assert(0);
						}
						fileMap[string(ent->d_name)] = filePath;
					initFileList(filePath.c_str());
				}
			}
			closedir (dir);
		}
	}
	public:
	CFileList(){
		initFileList();
	}
	vector<string> get(void){ return fileList; }
	string get(const string& name){
		if(fileMap.count(name)){ return fileMap[name]; }
		return string("");
	}
};

#endif /* __FILE_LIST_H__ */
