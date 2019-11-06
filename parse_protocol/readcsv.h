#ifndef __READCSV_H__
#define __READCSV_H__

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <map>
#include <vector>
#include <deque>
using namespace std;
#define FOR(_I_, _N_) for(int _I_=0;_I_<_N_;_I_++)
#define FORBE(_I_, _T_) for(auto _I_=_T_.begin();_I_!=_T_.end();_I_++)

static inline void ltrim(string &s) {
	s.erase(s.begin(), find_if(s.begin(), s.end(),
				not1(ptr_fun<int, int>(isspace))));
}
static inline void rtrim(string &s) {
	s.erase(find_if(s.rbegin(), s.rend(),
				not1(ptr_fun<int, int>(isspace))).base(), s.end());
}
static inline void trim(string &s) { ltrim(s); rtrim(s); }

class CCellPos {
	public:
		string pos;
		const string& operator()(){ return pos; }
		CCellPos(const char *filename="", int line=0, int col=0){
			char buf[64];
			sprintf(buf, "%s(%c%d)", filename, 'A' + col, line);
			this->pos = string(buf);
		}
};

class CCell {
	public:
		CCellPos pos;
		string text;
		const string& operator()(){ return text; }
		CCell(){}
		CCell(const CCellPos& pos, string text){
			this->pos = pos;
			this->text = text;
			trim(this->text);
		}
};
class CSheet {
	public:
		string name;
		deque<deque<CCell>> cells;
		deque<CCell>& operator[](int idx){ return cells[idx]; }
		CSheet(const string& name, deque<deque<CCell>>& cells){
			this->name = name;
			this->cells = cells;
		}
};
class CReadCSV {
	int file_pair_cnt;
	vector<CSheet> data;
	void read(char *filename, FILE *fp, vector<CSheet>& data){
		int line_idx = 0;
		deque<deque<CCell>> deque_line;
		static char line[8192];
		while (fgets(line, 8192, fp)){
			vector<char*> token;

			char *p = line; char *end = line + strlen(line);
			if(p == end) continue;
			token.push_back(p);
			while(*p){
				if(*p==0x0d||*p==0x0a)*p=0;
				else if(*p == ',' && p < end-1){
					token.push_back(p+1);
					*p = 0;
				}
				p++;
			}
			deque<CCell> deque_col;
			int col_idx = 0;
			for (auto tok=token.begin();tok!=token.end();tok++) {
				deque_col.push_back(CCell(CCellPos(filename, line_idx+1, col_idx), string(*tok)));
				col_idx++;
			}
			deque_line.push_back(deque_col);
			line_idx++;
		}
		data.push_back(CSheet(filename, deque_line));
	}

	public:
	vector<CSheet> get(void){ return data; }
	CSheet& operator[](int idx){ return data[idx]; }
	CReadCSV(){
		char fileNameFormat[32];
		char fileNameParam[32];
		file_pair_cnt = 0;
		hash<string> data_hash;
		string data_text;
		for(int i=0;;i++){
			sprintf(fileNameFormat, "format%d.csv", i);
			sprintf(fileNameParam, "param%d.csv", i);
			FILE *fp_f = fopen(fileNameFormat, "r");
			FILE *fp_p = fopen(fileNameParam, "r");
			if(fp_f == NULL || fp_p == NULL){
				if(fp_f) fclose(fp_f);
				if(fp_p) fclose(fp_p);
				break;
			}
			file_pair_cnt++;
			read(fileNameFormat, fp_f, data);
			read(fileNameParam, fp_p, data);

			rewind(fp_f); rewind(fp_p);
			char tmp_str[1024];
			while(fgets(tmp_str, 1024, fp_f) != NULL){ data_text += string(tmp_str); }
			while(fgets(tmp_str, 1024, fp_p) != NULL){ data_text += string(tmp_str); }
			fclose (fp_f); fclose (fp_p);
		}

		string hashStr = to_string(data_hash(data_text));
		char tmp_str[1024];
		extern bool re_enabled;
		FILE *fp_hash = fopen(".ContentHash", "r");
		if(fp_hash && fgets(tmp_str, 1024, fp_hash) != NULL && tmp_str == hashStr){ re_enabled = false; }
		if(fp_hash) fclose(fp_hash);
		if(re_enabled){ FILE *fp_hash = fopen(".ContentHash", "w"); fputs(hashStr.c_str(), fp_hash); }
	}
};

#endif /* __READCSV_H__ */
