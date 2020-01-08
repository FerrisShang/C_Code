#include "file_list.h"
#include "stdio.h"

int main(void)
{
	auto files = CFileList().get();
	for(auto it=files.begin();it!=files.end();it++){
		printf("%s\n", it->c_str());
	}
	printf("%s\n", CFileList().get("main.c").c_str());
}
