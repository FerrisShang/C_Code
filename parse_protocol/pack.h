#ifndef __PACK_H__
#define __PACK_H__

#include "database.h"
#include <stdio.h>
#include <assert.h>
#include <iomanip>

class CPack{
	public:
	CDatabase *database;
	CPack(CDatabase* database){
		this->database = database;
	}
	class CPackData{
	};
	string operator()(const string& stream){
		return stream;
	}
	string pack(set<string>&s, string name=PROTO_ROOT, int level=0 ){
		//if(!p){ p = &database->format[PROTO_ROOT][0]; }
		//	FOR(i, subItem->size()){
		//		CParam::CParams *p = &database->param[subItem->typeRemarks[i].type];

		CFormat::CCommand& cmd = database->format[name];
		cout << cmd.name << endl;
		s.insert(cmd.name);
		FORBE(it, database->format.commands){
			CFormat::CCommand& fmt = it->second;
			FORBE(it_sub, fmt.subItems){
				CFormat::CSubItem& sub = it_sub->second;
				FORBE(it_tr, sub.typeRemarks){
					CFormat::CTypeRemark& tr = *it_tr;
					CParam::CParams& par = database->param[tr.type];
					if(par.subKey().length() && par.subKey() != "0"){
						cout << level << par.name() << " | ";
						if(s.count(par.name())) continue;
						pack(s, par.name(), level+1);
					}
				}
			}
			if(level == 0) cout << it->second.name << endl;
		}
		return "";
	}
};

#endif /* __PACK_H__ */
