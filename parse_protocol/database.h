#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <assert.h>
#include <set>
#include "re_param.h"
#include "readcsv.h"
#include "format.h"
#include "param.h"

#define DEFAULT_SUBKEY 0

class CDatabase {
	public:
	CReadCSV csv_data;
	CFormat format;
	CParam param;
	vector<string> warning_list;
	vector<string> error_list;
	CDatabase(){
		const vector<CSheet>& data = csv_data.get();
		if(data.size() == 0){
			error_list.push_back("ERROR: No format/param files found.");
			return;
		}
		for(int i=0;i<data.size();i++){
			CSheet sheet = data[i];
			if(i&1){ // param
				if(sheet.cells.size() <= PARAM_RSV_LINE){
					warning_list.push_back(string("WARNING: No record in file ")+sheet.name);
					continue;
				}
				CParam::CParams *enum_param = NULL;
				for(int j=PARAM_RSV_LINE;j<sheet.cells.size();j++){
					auto line = sheet[j];
					if(line.size() < PARAM_MAX_COL){
						error_list.push_back(string("ERROR: Not enough colume @ ")+sheet.name+"("+to_string(j+1)+")");
						return;
					}
					auto& cell_name = line[PARAM_NAME_COL];
					if(cell_name()[0] == '#'){ // It's remark line
						continue;
					}
					auto& cell_bitWidth = line[PARAM_WIDTH_COL];
					auto& cell_type     = line[PARAM_TYPE_COL];
					auto& cell_key      = line[PARAM_KEY_SUB_COL];
					auto& cell_value    = line[PARAM_VAL_RNE_COL];
					auto& cell_def      = line[PARAM_DEF_COL];
					auto& cell_config   = line[PARAM_CONFIG_COL];
					auto& cell_desc     = line[PARAM_DESC_COL];
					if(cell_name().length()){ // parameter define
						if(!RE(cell_name(), RE_PARAM_NAME)){ // check name
							error_list.push_back(string("ERROR: Invalid 'param_name' formating: '")+
									cell_name()+string("' @ ")+cell_name.pos());
							return;
						}else if(param.params.count(cell_name())){ // check redefined
							error_list.push_back(string("ERROR: Redefined param: '")+
									cell_name()+string("' @ ")+cell_name.pos());
							return;
						}else if(!RE(cell_bitWidth(), RE_PARAM_WIDTH)){ // check bit width
							error_list.push_back(string("ERROR: Invalid 'bit_width' formating: '")+
									cell_bitWidth()+string("' @ ")+cell_bitWidth.pos());
							return;
						}else if(!RE(cell_type(), RE_PARAM_TYPE)){ // check type
							error_list.push_back(string("ERROR: Invalid 'type' formating: '")+
									cell_type()+string("' @ ")+cell_type.pos());
							return;
						}else if(cell_def().length() && !RE(cell_def(), RE_PARAM_DEFAULT)){ // check default
							error_list.push_back(string("ERROR: Invalid 'default' formating: '")+
									cell_def()+string("' @ ")+cell_def.pos());
							return;
						}else if(cell_config().length()){
							if(!RE(cell_config(), RE_PARAM_VALUE)){ // check cfg_flag
								error_list.push_back(string("ERROR: Invalid 'cfg_flag' formating: '")+
										cell_config()+string("' @ ")+cell_config.pos());
								return;
							}
							if(isSet(stoi(cell_config(), nullptr, 0), BC_PACK_LENGTH) &&
									!(cell_def().length() && RE(cell_def(), RE_PARAM_WIDTH_NUM))){
								error_list.push_back(string("ERROR: Invalid 'default'(as pack length) formating: '")+
										cell_def()+string("' @ ")+cell_def.pos());
								return;
							}
						}
						if(cell_type() == PTYPE_ENUM || cell_type() == PTYPE_BITMAP){ // type==enum/bitmap
							if(!cell_key().length()){
								error_list.push_back(string("ERROR: 'sub_key' can NOT be empty @ ")+cell_key.pos());
								return;
							}else if(!cell_value().length()){
								error_list.push_back(string("ERROR: 'value/range' can NOT be empty @ ")+cell_value.pos());
								return;
							}else if(!RE(cell_key(), RE_PARAM_NAME)){ // check sub_key
								error_list.push_back(string("ERROR: Invalid 'key' formating: '")+
										cell_key()+string("' @ ")+cell_key.pos());
								return;
							}else if(!RE(cell_value(), RE_PARAM_VALUE)){ // check value/range
								error_list.push_back(string("ERROR: Invalid 'value' formating: '")+
										cell_value()+string("' @ ")+cell_value.pos());
								return;
							}
						}else{ // type is others
							if(cell_key().length() && cell_key()!="0" && !RE(cell_key(), RE_PARAM_NAME) ){ // check sub_key
								error_list.push_back(string("ERROR: Invalid 'sub_key' formating: '")+
										cell_key()+string("' @ ")+cell_key.pos());
								return;
							}else if(cell_value().length() && !RE(cell_value(), RE_PARAM_RANGE)){ // check range
								error_list.push_back(string("ERROR: Invalid 'range' formating: '")+
										cell_value()+string("' @ ")+cell_value.pos());
								return;
							}
						}
						//check DONE finally ............................................
						if(!cell_config().length()){ cell_config.text = "0"; }
						param.addParam(cell_name, cell_bitWidth, cell_type, cell_key, cell_value,
										cell_def, stoi(cell_config(), nullptr, 0), cell_desc());
						if(cell_type() == PTYPE_ENUM || cell_type() == PTYPE_BITMAP){ // type==enum/bitmap
								enum_param = &param[cell_name()];
								int value = stoi(cell_value(), nullptr, 0);
								if(enum_param->enums.count(value)){
										error_list.push_back(string("ERROR: Redefined enum value : '")+
														cell_value()+string("' @ ")+cell_value.pos());
										return;
								}
								enum_param->addEnum(cell_key, cell_value, cell_desc());
						}else{ enum_param = NULL; }
					}else if(cell_key().length() && cell_value().length() && !cell_bitWidth().length()
							&& !cell_type().length()){ // enum subkey & value
							if(!enum_param){
										error_list.push_back(string("ERROR: key/value found without enum definitionv @ ")
														+cell_value.pos());
										return;
							}
							// check cells
							if(!cell_key().length()){
									error_list.push_back(string("ERROR: 'sub_key' can NOT be empty @ ")+cell_key.pos());
									return;
							}else if(!cell_value().length()){
									error_list.push_back(string("ERROR: 'value/range' can NOT be empty @ ")+cell_value.pos());
									return;
							}else if(!RE(cell_key(), RE_PARAM_NAME)){ // check sub_key
									error_list.push_back(string("ERROR: Invalid 'key' formating: '")+
													cell_key()+string("' @ ")+cell_key.pos());
									return;
							}else if(!RE(cell_value(), RE_PARAM_VALUE)){ // check value/range
									error_list.push_back(string("ERROR: Invalid 'value' formating: '")+
													cell_value()+string("' @ ")+cell_value.pos());
									return;
							}
							// check done ~~~~~~~~~~~~~~~~~~~~~~~~~
							int value = stoi(cell_value(), nullptr, 0);
							if(enum_param->enums.count(value)){
									error_list.push_back(string("ERROR: Redefined enum value : '")+
													cell_value()+string("' @ ")+cell_value.pos());
									return;
							}
							enum_param->addEnum(cell_key, cell_value, cell_desc());
					}else if(!cell_key().length() && !cell_value().length() && !cell_bitWidth().length()
							&& !cell_type().length()){ // empty
						continue;
					}else{ // invalid input
						error_list.push_back(string("Invalid input line @ ")+sheet.name+"("+to_string(j+1)+")");
						return;
					}
				}
			}else{ // format
				if(sheet.cells.size() <= FMT_RSV_LINE){
					warning_list.push_back(string("WARNING: No record in file ")+sheet.name);
					continue;
				}
				CFormat::CCommand *cmd = NULL;
				for(int j=FMT_RSV_LINE;j<sheet.cells.size();j++){
					auto line = sheet[j];
					if(line.size() < FMT_MAX_COL){
						error_list.push_back(string("ERROR: Not enough colume @ ")+sheet.name+"("+to_string(j+1)+")");
						return;
					}
					auto& cell_cmd = line[FMT_CMD_COL];
					if(cell_cmd()[0] == '#'){ // It's remark line
						continue;
					}
					auto& cell_remark = line[FMT_REMARK_COL];
					auto& cell_key_code = line[FMT_KEY_COL];
					auto& cell_param = line[FMT_PARAM_COL];
					if(cell_cmd().length()){ // It's a first command line
						if(!RE(cell_cmd(), RE_PARAM_NAME)){
							error_list.push_back(string("ERROR: Invalid name formating: '")+
									cell_cmd()+string("' @ ")+cell_cmd.pos());
							return;
						}

						if(format.commands.count(cell_cmd())){
							error_list.push_back(string("ERROR: Redefined format: '")+
									cell_cmd()+string("' @ ")+cell_cmd.pos());
							return;
						}
						format.addCmd(cell_cmd);
						cmd = &format.commands[cell_cmd()];
					}else if(!cell_remark().length() && !cell_key_code().length() && !cell_param().length()){ // It's empty line
						continue;
					}
					if(!RE(cell_key_code(), RE_PARAM_VALUE)){
						error_list.push_back(string("ERROR: Invalid key_code: '")+
								cell_key_code()+string("' @ ")+cell_key_code.pos());
						return;
					}
					int key_code = stoi(cell_key_code(), nullptr, 0);
					if(!cmd){
						error_list.push_back(string("ERROR: Missing command defination @ ")+cell_param.pos());
						return;
					}
					cmd->addSubItem(cell_param.pos, key_code, cell_remark());
					if(!RE(cell_param(), RE_FORMAT_PARAM)){
						error_list.push_back(string("ERROR: Invalid parameters format: '")+
								cell_param()+string("' @ ")+cell_param.pos());
						return;
					}
					CFormat::CSubItem& subItem = cmd->subItems[key_code];
					char *str = _strdup(cell_param().c_str());
					for (const char* tok = strtok(str, "|"); tok && *tok; tok = strtok(NULL, "|"))
					{
						const char *p = tok; while(*p == ' ') p++;
						const char *type = p, *name;
						while(*p != 0 && *p != ' ') p++;
						if(*p == ' '){ *(char*)p = 0; name = p+1; }
						else{ name = type; }
						string str_type(type); trim(str_type);
						string str_name(name); trim(str_name);
						subItem.addTypeRemark(subItem.pos, str_type, str_name);
					}
					free(str);
				}
			}
		}
		validation();
	}
	void validation(void){
		if(!format.commands.count(PROTO_ROOT)){
			error_list.push_back(string("ERROR: '" PROTO_ROOT "' as ROOT entrance must be defined"));
			return;
		}
		if(!param.params.count(PROTO_UNUSED)){
			param.addParam(CCell(CCellPos(), PROTO_UNUSED), CCell(CCellPos(), "0"),
					CCell(CCellPos(), PTYPE_STREAM), CCell(CCellPos(), ""),
					CCell(CCellPos(), ""), CCell(CCellPos(), "0")
					);
		}
		FORBE(it, param.params){ // referenced type to basic tye
			CParam::CParams& pa = it->second;
			if(!isBasicType(pa.type())){ // validate referenced type
				if(pa.type() == pa.name()){
					error_list.push_back(string("ERROR: Invalid type, can NOT reference itself. '")+
							pa.type()+string("' @ ")+pa.type.pos());
					return;
				}
				if(!param.params.count(pa.type())){
					error_list.push_back(string("ERROR: Referenced type NOT found: '")+
							pa.type()+string("' @ ")+pa.type.pos());
					return;
				}
				if((pa.bitWidth.cell().length()) || (pa.subKey().length()) || pa.range.cell().length()
						|| pa.def_value.cell().length() || pa.description.length()){
					warning_list.push_back(string("WARNING: Reference type should leave cells as empty except 'name & type'. '")+
							pa.name()+string("' @ ")+pa.name.pos());
				}
				while(!isBasicType(pa.type())){ // referenced type transform
					if(pa.type() == pa.name()){
						error_list.push_back(string("ERROR: Loop reference found. '")+
								pa.name()+string("' @ ")+pa.name.pos());
						return;
					}
					CCell name = pa.name;
					pa = param[pa.type()];
					pa.name = name;
				}
			}else{ // validate basic type
				if(!pa.bitWidth.cell().length()){
					error_list.push_back(string("ERROR: Invalid length, basic type length can NOT be empty. '")+
							pa.name()+string("' @ ")+pa.bitWidth.cell.pos());
					return;
				}
			}
		}
		FORBE(it, param.params){ // param sub_key validation
			CParam::CParams& pa = it->second;
			if(pa.type() != PTYPE_ENUM && pa.type() != PTYPE_BITMAP){
				if(pa.subKey().length()){
					if(pa.subKey() != "0"){
						if(!param.params.count(pa.subKey())){
							error_list.push_back(string("ERROR: sub_key '")+
									pa.subKey()+string("' not defined @ ")+pa.subKey.pos());
							return;
						}
						if(param[pa.subKey()].type() != PTYPE_ENUM){
							error_list.push_back(string("ERROR: Invalid type, sub_key's type must be enum @ ")+pa.subKey.pos());
							return;
						}
					}
					if(!format.commands.count(pa.name())){ // parameter with sub_key must defined in format
							error_list.push_back(string("ERROR: Param '")+pa.name()+string("' with sub_key '")+
									pa.subKey()+string("' not defined in format @ ")+pa.name.pos());
							return;
					}
				}
			}else if(pa.type() == PTYPE_ENUM){
				if(pa.bitWidth.m != 8 && pa.bitWidth.m != 16 && pa.bitWidth.m != 32){
					error_list.push_back(string("ERROR: Param '")+pa.name()+
										string("' width MUST be 8bit/16bit/32bit @ ")+pa.name.pos());
					return;
				}
			}
			if(pa.bitWidth.isBitShare()){
				if(pa.bitWidth.p >= pa.bitWidth.m){ // offset beyond limit
					error_list.push_back(string("ERROR: Param '")+pa.name()+
									string("' bit position exceed max width @ ")+pa.bitWidth.cell.pos());
					return;
				}else if(pa.bitWidth.w > pa.bitWidth.m - pa.bitWidth.p){ // width too large
					error_list.push_back(string("ERROR: Param '")+pa.name()+
									string("' bit width exceed max width @ ")+pa.bitWidth.cell.pos());
					return;
				}
			}
		}
		FORBE(it, format.commands){ // auto define param which as only 1 item in format
			if(it->second.subItems.size() == 1 && it->second.subItems.count(0) && !param.params.count(it->first)){
				param.addParam(CCell(it->second.pos, it->first), CCell(it->second.pos, "0"),
								CCell(it->second.pos, PTYPE_STREAM), CCell(it->second.pos, "0"),
								CCell(it->second.pos, ""), CCell(it->second.pos, "0")
				);
			}
		}
		FORBE(it, format.commands){ // format subItems validation
			FORBE(it_sub, it->second.subItems){
				FORBE(it_tr, it_sub->second.typeRemarks){
					if(!param.params.count(it_tr->type)){// undefined check
							error_list.push_back(string("ERROR: Type '")+it_tr->type+string("' in format '")+
											it->second.name+string("' not defined in param @ ")+it_tr->pos());
							return;
					}
				}
			}
		}
		//Unused validation
		set<string> set_p_in_format, set_type, sub_key;
		FORBE(it, param.params){
			CParam::CParams& pa = it->second;
			set_type.insert(pa.type());
			sub_key.insert(pa.subKey());
		}
		FORBE(it, format.commands){
			CFormat::CCommand& cmd = it->second;
			FORBE(it_item, cmd.subItems){
				CFormat::CSubItem& subItem = it_item->second;
				FORBE(it_tr, subItem.typeRemarks){
					set_p_in_format.insert(it_tr->type);
				}
			}
		}
		FORBE(it, param.params){
			CParam::CParams& pa = it->second;
			if(!(set_p_in_format.count(pa.name()) || set_type.count(pa.name()) || sub_key.count(pa.name()))){
				if(pa.name() != PROTO_ROOT){
						warning_list.push_back(string("WARNING: Unused parameter '")+pa.name()+string("' @ ")+pa.name.pos());
				}
			}
		}
	}
	void dump_status(void){
		if(error_list.size()){
			cout << error_list[0] << endl;
			return;
		}
		if(warning_list.size()){
			for(auto it=warning_list.begin();it!=warning_list.end();it++){
				cout << *it << endl;
			}
		}
		return;
		FORBE(it_cmd, format.commands){
			cout << it_cmd->first << endl;
			FORBE(it_sub, it_cmd->second.subItems){
				cout << "\t" << it_sub->second.remark << " : " << it_sub->first << endl;
				FORBE(it_tn, it_sub->second.typeRemarks){
					cout << "\t\t" << it_tn->type << "  " << it_tn->remark << endl;
				}
			}
		}
		FORBE(it_pa, param.params){
			auto param = it_pa->second;
			cout << param.name() << " " << param.type() << endl;
			if(param.type() == PTYPE_ENUM || param.type() == PTYPE_BITMAP){
				FORBE(it_enum, param.enums){
					cout << "\t" << it_enum->second.value << " : " << it_enum->second.key() << endl;
				}
			}
		}
	}
	int getStatus(void){ return -error_list.size(); }
};

#endif /* __DATABASE_H__ */

