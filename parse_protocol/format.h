#ifndef __FORMAT_H__
#define __FORMAT_H__

#include <cstring>
#include <map>

#define FMT_RSV_LINE    1
enum{
	FMT_CMD_COL,
	FMT_REMARK_COL,
	FMT_KEY_COL,
	FMT_PARAM_COL,
	FMT_MAX_COL,
};

class CFormat {
	public:
		class CTypeRemark {
			public:
				CCellPos pos;
				string type;
				string remark;
				CTypeRemark(){}
				CTypeRemark(CCellPos pos, string t, string n){ type = t; remark = n; this->pos = pos; }
		};

		class CSubItem {
			public:
				CCellPos pos;
				string remark;
				int key_value;
				vector<CTypeRemark> typeRemarks;
				CTypeRemark& operator[](int idx){ return typeRemarks[idx]; }
				CSubItem(){}
				CSubItem(CCellPos pos, int k, string r){ key_value = k; remark = r; this->pos = pos; }
				void addTypeRemark(const CCellPos& pos, string type, string remark){
					if(remark.length() == 0){ remark = type; }
					typeRemarks.push_back(CTypeRemark(pos, type, remark));
				}
				int size(void){ return typeRemarks.size(); }
		};

		class CCommand {
			public:
				CCellPos pos;
				string name;
				map<int, CSubItem> subItems;
				CSubItem& operator[](int key_value){ return subItems[key_value]; }
				CCommand(CCellPos pos=CCellPos("",0,0), string name=""){
					this->name = name;
					this->pos = pos;
				}
				bool addSubItem(const CCellPos& pos, int key_value, string remark=""){
					if(!subItems.count(key_value)){
						CSubItem item(pos, key_value, remark);
						subItems[key_value] = item;
						return true;
					}else{
						return false;
					}
				}
		};

		map<string, CCommand> commands;
		CCommand& operator[](string cmd){ return commands[cmd]; }
		bool addCmd(const CCell& cell){
			if(!commands.count(cell.text)){
				commands[cell.text] = CCommand(cell.pos, cell.text);
				return true;
			}else{
				return false;
			}
		}
		void dump(void){
			for(auto it_cmd = commands.begin(); it_cmd != commands.end();it_cmd++){
				cout << it_cmd->first << endl;
				for(auto it_s = it_cmd->second.subItems.begin(); it_s != it_cmd->second.subItems.end();it_s++){
					cout << it_s->first << "\t" << it_s->second.remark << endl;
					for(auto it_t = it_s->second.typeRemarks.begin(); it_t != it_s->second.typeRemarks.end();it_t++){
						cout << it_t->type << " " << it_t->remark << " | ";
					}
					cout << endl;
				}
			}
		}
};

#endif /* __FORMAT_H__ */
