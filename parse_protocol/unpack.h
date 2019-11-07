#ifndef __UNPACK_H__
#define __UNPACK_H__

#include "database.h"
#include <stdio.h>
#include <assert.h>
#include <iomanip>

#define UPST_SUCCESS 0
#define PSD pair<string, vector<uint8_t>>

class CUnpack{
	public:
	CDatabase *database;
	CUnpack(CDatabase* database){
		this->database = database;
	}
	class CRec{
		public:
		vector<uint8_t> data;
		CParam::CParams *dataType;
		string text;
		vector<string> vecText; //used by bitmap
		string remark;
		CFormat::CSubItem *subItem;
		vector<CRec> subRec;
		CUnpack *unpack;
		int indent;
		CRec(){}
		CRec(const vector<uint8_t>& data, CParam::CParams *dataType,
					CFormat::CSubItem *subItem, CUnpack *unpack, const string& remark="", int indent=0){
			parse(data, dataType, subItem, unpack, remark, indent);
		}
		inline string operator()(void){ return text; }
		string byteArray2hex(vector<uint8_t>& s){
			string ret("0x"); char buf[8];
			FOR(i, s.size()){ sprintf(buf, "%02X", s[i]); ret += string(buf); }
			return ret;
		}
		uint32_t byteArray2uint(vector<uint8_t>& s){
			assert(s.size()==1||s.size()==2||s.size()==4);
			if(s.size() == 1) return s[0];
			else if(s.size() == 2) return s[0]+(s[1]<<8);
			else if(s.size() == 4) return s[0]+(s[1]<<8)+(s[2]<<16)+(s[3]<<24);
			assert(0);
		}
		vector<uint8_t> uint2byteArray(uint32_t value, int byteWidth){
			if(byteWidth == 1){ return vector<uint8_t> { (uint8_t)value}; }
			if(byteWidth == 2){ return vector<uint8_t> { (uint8_t)value, (uint8_t)(value>>8) }; }
			assert(0);
		}
		int32_t byteArray2int(vector<uint8_t>& s){
			assert(s.size()==1||s.size()==2||s.size()==4);
			if(s.size() == 1) return (int8_t)s[0];
			else if(s.size() == 2) return (int16_t)(s[0]+(s[1]<<8));
			else if(s.size() == 4) return (int32_t)(s[0]+(s[1]<<8)+(s[2]<<16)+(s[3]<<24));
			assert(0);
		}
		void data2text(void){
			char buf[128];
			if(data.size() == 0 || (0 < dataType->bitWidth.w/8 && dataType->bitWidth.w/8 > data.size())){
				if(data.size() > 0){
				}
				text += "No more data";
				return;
			}
			if(dataType->type.text == PTYPE_ENUM){
				uint32_t value = byteArray2uint(data);
				const char fmt[][16] = {"%s (0x%02x)", "%s (0x%04x)", "%s (0x%08x)"};
				if(dataType->enums.count(value)){
					sprintf(buf, fmt[dataType->bitWidth.w/8/2], dataType->enums[value].key().c_str(), value);
				}else{
					sprintf(buf, fmt[dataType->bitWidth.w/8/2], "Unknown", value);
				}
				text = string(buf);
			}else if(dataType->type.text == PTYPE_BITMAP){
				text = byteArray2hex(data);
				FOR(i, data.size() * 8){
					bool enabled = false;
					if(data[i/8] & (1<<(i%8))){ enabled = true; }
					if(!dataType->enums.count(i)){ continue; }
					if(enabled || isSet(dataType->configFlag, BC_OUTPUT_BIT)){
						const char fmt[][16] = {"BIT%d  (%d) %s", "BIT%d (%d) %s"};
						sprintf(buf, fmt[i>9], i, enabled, dataType->enums[i].desciption.length()?\
														dataType->enums[i].desciption.c_str():dataType->enums[i].key().c_str());
						vecText.push_back(string(buf));
					}
				}
			}else if(dataType->type.text == PTYPE_HEX){
				text = byteArray2hex(data);
			}else if(dataType->type.text == PTYPE_UNSIGNED){
				uint32_t value = byteArray2uint(data);
				sprintf(buf, "%d", value); text = string(buf);
			}else if(dataType->type.text == PTYPE_SIGNED){
				int32_t value = byteArray2int(data);
				sprintf(buf, "%d", value); text = string(buf);
			}else if(dataType->type.text == PTYPE_ADDRESS){
				sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", data[5], data[4], data[3], data[2], data[1], data[0]);
				text = string(buf);
			}else if(dataType->type.text == PTYPE_T0_625){
				sprintf(buf, "%.3f ms", 0.625 * byteArray2uint(data)); text = string(buf);
			}else if(dataType->type.text == PTYPE_T1_25){
				sprintf(buf, "%.2f ms", 1.25 * byteArray2uint(data)); text = string(buf);
			}else if(dataType->type.text == PTYPE_T10){
				sprintf(buf, "%.0f ms", 10.0 * byteArray2uint(data)); text = string(buf);
			}else{ //dataType->type.text == PTYPE_STREAM
				text.resize(3*data.size()-1, ' ');
				FOR(i, data.size()){
					uint8_t h = data[i] >> 4; text[i*3+0] = h>9?'A'+h-10:'0'+h;
					uint8_t l = data[i] & 15; text[i*3+1] = l>9?'A'+l-10:'0'+l;
				}
				if(dataType->type.text != PTYPE_STREAM){
					text += " (" + dataType->type.text + ")";
				}
			}
		}
		void parse(const vector<uint8_t>& data, CParam::CParams *dataType,
					CFormat::CSubItem *subItem, CUnpack *unpack, const string& remark="", int indent=0){
			CDatabase *database = unpack->database;
			assert(dataType && database);
			this->unpack = unpack;
			this->data = data;
			this->dataType = dataType;
			this->subItem = subItem;
			this->remark = remark;
			this->indent = indent;
			//cout << "push in : " << this->remark << ", data len: "<< this->data.size() << endl;
			unpack->recList.push_back(PSD(dataType->name.text, this->data));
			data2text();
			if(!subItem) return;
			int cp=0, ml=data.size(), bitShareWidth=0, bitPos=0xFF;
			int new_indent = isSet(dataType->configFlag, BC_INC_INDENT)?indent+1:indent;
			FOR(i, subItem->size()){
				CParam::CParams *p = &database->param[subItem->typeRemarks[i].type];
				//cout << p->name() << " " << p->bitWidth.isBitShare() << endl;
				int cpInc = p->bitWidth.m==0?ml-cp:p->bitWidth.m/8;
				if(p->bitWidth.isBitShare()){
					if(!bitShareWidth){ bitShareWidth=p->bitWidth.m/8; }
					if(bitPos > p->bitWidth.p){
						bitPos = p->bitWidth.p;
						cpInc = 0;
					}else{
						cp += bitShareWidth;
						bitPos = 0xFF;
					}
				}else{
					cp += bitShareWidth;
					bitShareWidth = 0;
					bitPos = 0xFF;
				}
				//cout << data.size() << ":" << cp << "|" << p->bitWidth.m/8 << "|" << ml << endl;
				int width = p->bitWidth.m==0?ml-cp:p->bitWidth.m/8;
				if(cp + width <= ml){ // normal data
					vector<uint8_t> newData(data.begin()+cp, p->bitWidth.m==0?data.end():(data.begin()+cp+p->bitWidth.m/8));
					if(p->bitWidth.isBitShare()){
						uint32_t v = byteArray2uint(newData);
						v >>= p->bitWidth.p;
						v &= ~((-1ul)<<p->bitWidth.w);
						newData = uint2byteArray(v, (p->bitWidth.w+7)/8);
					}
					cp += cpInc;
					CFormat::CSubItem *newSubItem = NULL;
					if(p->subKey().length()){
						int itemValue = keyValue(p->subKey());
						if(itemValue < 0){ continue; }
						assert(database->format.commands.count(subItem->typeRemarks[i].type));
						if(database->format[subItem->typeRemarks[i].type].subItems.count(itemValue)){
							newSubItem = &database->format[subItem->typeRemarks[i].type][itemValue];
						}
					}
					if(newData.size()){
						subRec.push_back(CRec(newData, p, newSubItem, unpack, subItem->typeRemarks[i].remark, new_indent));
					}
				}else{ // not enough data
					vector<uint8_t> newData(data.begin()+cp, data.end());
					cp = ml;
					subRec.push_back(CRec(newData, p, NULL, unpack, subItem->typeRemarks[i].remark, new_indent));
				}
			}
			if(cp < ml && bitPos == 0xFF){ // unused data
				vector<uint8_t> newData(data.begin()+cp, data.end());
				subRec.push_back(CRec(newData, &database->param[PROTO_UNUSED], NULL, unpack, "Unused", new_indent));
			}
		}
		int keyValue(const string& subKey){
			//cout << "Check subKey " << subKey << endl;
			for(auto it=unpack->recList.end()-1;it>=unpack->recList.begin();it--){
				//cout << "*" << subKey << " . " << it->first << endl;
				if(it->first == subKey){
					auto& s = it->second;
					if(s.size() == 0){ return -1; }
					return byteArray2uint(s);
				}
			}
			return 0;
		}
		void dump(CRec* rec=NULL){
			if(rec == NULL){ rec = &unpack->unpackRec; }
			if(!isSet(rec->dataType->configFlag, BC_UNUSED) &&
				(!rec->subItem || isSet(rec->dataType->configFlag, BC_OUTPUT_SUBKEY))){
				FOR(_, rec->indent)cout << '\t';
				cout << setw(16) << left << rec->remark << " : " << rec->text << endl;
				if(rec->vecText.size()){
					FOR(i, rec->vecText.size()){
						FOR(_, rec->indent)cout << '\t';
						cout << setw(16) << " " << "\t: " << rec->vecText[i] << endl;
					}
				}
			}
			if(rec->subRec.size()){
				FOR(i, rec->subRec.size()){
					dump(&rec->subRec[i]);
				}
			}
		}
	};
	CRec operator()(const string& stream){
		return unpack(stream);
	}
	CRec unpackRec;
	vector<PSD> recList;
	static vector<uint8_t> s2b(const string& stream){
		vector<uint8_t> hex;
		FOR(i, stream.length()){
			if('0'<=stream[i]&&stream[i]<='9'){
				hex.push_back((uint8_t)stream[i]);
			}else if(('a'<=stream[i]&&stream[i]<='f')||('A'<=stream[i]&&stream[i]<='F')){
				hex.push_back((uint8_t)stream[i] | 0x20);
			}else if(isspace(stream[i])){
				continue;
			}else{ break; }
		}
		FOR(i, hex.size()/2){
			uint8_t byte = hex[i+i+0] >= 'a'?(10+hex[i+i+0]-'a'):(hex[i+i+0]-'0');
			hex[i] = (byte << 4) | (hex[i+i+1] >= 'a'?(10+hex[i+i+1]-'a'):(hex[i+i+1]-'0'));
		}
		hex.resize(hex.size()/2);
		return hex;
	}
	CRec unpack(const string& stream){
		return unpack(s2b(stream));
	}
	CRec unpack(const vector<uint8_t>& h){
		recList.resize(0);
		unpackRec.subRec.resize(0);
		unpackRec.parse(h, &database->param.params[PROTO_ROOT], &database->format[PROTO_ROOT][0], this, "ROOT");
		return unpackRec;
	}
};

#endif /* __UNPACK_H__ */
