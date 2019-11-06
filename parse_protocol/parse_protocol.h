#ifndef __PARSE_PROTOCOL_H__
#define __PARSE_PROTOCOL_H__

#include "unpack.h"
#include "pack.h"

class CParseProtocol{
	public:
	CDatabase *pDatabase;
	CPack *pPack;
	CUnpack *pUnpack;
	CParseProtocol(){
		pDatabase = new CDatabase();
		pPack = new CPack(pDatabase);
		pUnpack = new CUnpack(pDatabase);
	}
	~CParseProtocol(){
		delete pDatabase;
		delete pPack;
		delete pUnpack;
	}
	int getStatus(void){
		pDatabase->dump_status();
		return -pDatabase->getStatus();
	}
	inline CUnpack::CRec unpack(const string& stream){ return pUnpack->unpack(stream); }
	inline CUnpack::CRec unpack(const vector<uint8_t>& stream){ return pUnpack->unpack(stream); }
	set<string> s; string pack(void){ return pPack->pack(s); }

};
#endif /* __PARSE_PROTOCOL_H__ */
