#ifndef __SCRIPT_PARSE_H__
#define __SCRIPT_PARSE_H__

#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include "str_utils.h"
#include <vector>
#include <assert.h>
#include <string>
#include <map>
#include <stack>
#include <mutex>
#include <assert.h>
#include "file_list.h"
#define FOR(_I_, _N_) for(int _I_=0;_I_<_N_;_I_++)
#define FORBE(_I_, _T_) for(auto _I_=_T_.begin();_I_!=_T_.end();_I_++)
using namespace std;

class CScriptParse {
#define SC_OUTPUT printf
	vector<cmd_line_t*> cmd_lines;
	int timeout;
	int current_pos;
	int mode;
	int debug_flag;
	string title;
	mutex *mtx;
	uint32_t pending_flag;
	uint32_t pending_num;
	map<const string, sc_cmd_value_t> variable_pool;
	vector<cmd_line_t*> callback_global;
	struct for_stack{
		int for_pos;
		int cur_value;
	};
	stack<struct for_stack> for_stack;
	CFileList files;
	bool finished;
	public:
	CScriptParse(char *file_name, const char*title=NULL, mutex *mtx=NULL){
		parse_file(cmd_lines, file_name);
		current_pos = 0;
		timeout = 3000;
		mode = SC_MODE_SEQUENCE;
		debug_flag = 1;
		pending_num = 0;
		pending_flag = 0;
		if(title) this->title = string(title) + string(": ");
		this->mtx = mtx;
		finished = false;
		files = CFileList();
	}
	~CScriptParse(){
		FORBE(cmd_line, cmd_lines){
			//dump_line(*cmd_line);
			free_line(*cmd_line);
		}
	}
	bool isFinished(void){ return finished; }
	int getDebug(void){ return debug_flag; }
	void dump_lines(void){ FORBE(cmd_line, cmd_lines){ dump_line(*cmd_line); } }
	void parse_file(vector<cmd_line_t*> &cmd_lines, char*file_name){
		FILE *fp = fopen(files.get(file_name).c_str(), "r");
		if(fp == NULL){
			printf("File \"%s\" NOT found!\n", file_name);
			assert(0);
			return;
		}
		char buf[1024];
		while (!feof(fp)) {
			buf[0] = '\0';
			fgets(buf, 1024, fp);
			cmd_line_t* p = parse_line(buf);
			if(p->type >= 0 && p->type != SC_CMD_REMARK){
				if(p->type == SC_CMD_IMPORT){
					parse_file(cmd_lines, p->import.name);
					free_line(p);
				}else{
					cmd_lines.push_back(p);
					if(cmd_lines.size() > 1024){
						if(mtx)mtx->lock();
						if(title.length()) SC_OUTPUT(title.c_str());
						puts("Maximum lines exceeded !");
						assert(0);
						if(mtx)mtx->unlock();
						return;
					}
				}
			}else{
				free_line(p);
			}
		}
		fclose(fp);
	}
	int get_timeout(void){ return timeout;}
	vector<vector<uint8_t>> script_end(vector<vector<uint8_t>> data){
		finished = true;
		return data;
	}
	vector<vector<uint8_t>> get_send_data(uint8_t *received, int recv_len, vector<vector<uint8_t>> data={}){
		//vector<vector<uint8_t>> data;
		if(current_pos == cmd_lines.size()){ return script_end(data); }
		while((cmd_lines[current_pos]->type != SC_CMD_IGNORE &&
				cmd_lines[current_pos]->type != SC_CMD_RECV) || mode == SC_MODE_CALLBACK){
			cmd_line_t *cmd = cmd_lines[current_pos];
			switch(cmd->type){
				case SC_CMD_DEFINE:{
					sc_cmd_value_t value = *(sc_cmd_value_t*)&cmd->define;
					value.type = SC_VT_HEX;
					if(!variable_pool.count(cmd->define.name)){
						variable_pool[cmd->define.name] = value;
					}
					}break;
				case SC_CMD_ASSIGN:{
					sc_cmd_value_t value = *(sc_cmd_value_t*)&cmd->define;
					value.type = SC_VT_HEX;
					variable_pool[cmd->define.name] = value;
					}break;
				case SC_CMD_SEND:
					if(mode != SC_MODE_CALLBACK){
						vector<uint8_t> send_data;
						send_data.push_back(SC_CMD_SEND);
						FOR(i, cmd->send.len){
							sc_cmd_value_t *d = &cmd->send.data_list[i];
							assert(d->type == SC_VT_HEX || d->type == SC_VT_VAR);
							if(d->type == SC_VT_HEX){
								FOR(j, d->data_len){ send_data.push_back(d->data_hex[j]); }
							}else{ /* SC_VT_VAR */
								if(!variable_pool.count(d->name)){
									printf("Variable %s @ %d not defined!\n", d->name);
									assert(0);
								}
								sc_cmd_value_t *var = &variable_pool[d->name];
								FOR(j, var->data_len){ send_data.push_back(var->data_hex[j]); }
							}
						}
						data.push_back(send_data);
					}else{
						//TODO: send data in global mode
					}
					break;
				case SC_CMD_IGNORE:
				case SC_CMD_RECV:{
					assert(mode == SC_MODE_CALLBACK);
					//TODO: recv data in global mode
					}break;
				case SC_CMD_FOR:{
					assert(current_pos + 1 <= cmd_lines.size());
					struct for_stack fs = { current_pos, cmd->for_loop.first };
					sc_cmd_value_t value = *(sc_cmd_value_t*)&cmd->for_loop;
					value.type = SC_VT_HEX;
					FOR(i, 8){ value.data_hex[i] = (cmd->for_loop.first >> i*8) & 0xFF; }
					variable_pool[cmd->for_loop.name] = value;
					for_stack.push(fs);
					}break;
				case SC_CMD_LOOP:{
					assert(!for_stack.empty());
					sc_cmd_for_loop_t *f = &cmd_lines[for_stack.top().for_pos]->for_loop;
					if(for_stack.top().cur_value > f->second) for_stack.top().cur_value--;
					else if(for_stack.top().cur_value < f->second) for_stack.top().cur_value++;
					FOR(i, 8){ f->data_hex[i] = (for_stack.top().cur_value >> i*8) & 0xFF; }
					if(for_stack.top().cur_value == f->second){ for_stack.pop(); }
					else{ current_pos = for_stack.top().for_pos; }
					}break;
				case SC_CMD_SET:{
					switch(cmd->set.type){
						case SC_SET_MODE:
							mode = cmd->set.data;
							break;
						case SC_SET_TOUT:
							timeout = cmd->set.data;
							break;
						case SC_SET_DEBUG:
							debug_flag = cmd->set.data;
							break;
					}
					}break;
				case SC_CMD_CUSTOM:
				case SC_CMD_DEBUG:
				case SC_CMD_WARNING:
				case SC_CMD_ERROR:
				case SC_CMD_INFO:{
					vector<uint8_t> debug_data;
					debug_data.push_back(cmd->type);
					char buf[4096];
					strcpy(buf, cmd->type != SC_CMD_CUSTOM?"* ":"");
					FOR(i, cmd->debug.len){
						sc_cmd_value_t *data = &cmd->debug.data_list[i];
						if(data->type == SC_VT_VAR){
							sprintf(buf, "%s%s( ", buf, data->name);
							if(variable_pool.count(data->name)){
								sc_cmd_value_t *hex_data = &variable_pool[data->name];
								assert(hex_data->type == SC_VT_HEX);
								FOR(j, hex_data->data_len){
									sprintf(buf, "%s%02X ", buf, hex_data->data_hex[j]);
								}
							}else{
								strcat(buf, "Undefined");
							}
							strcat(buf, ") ");
						}else{ /* SC_VT_DEF */
							sprintf(buf, "%s%s ", buf, data->name);
						}
					}
					debug_data.resize(1 + strlen(buf) + 1);
					strcpy((char*)&debug_data[1], buf);
					data.push_back(debug_data);
					}break;
				case SC_CMD_EXIT:
					if(mtx)mtx->lock();
					if(title.length()) SC_OUTPUT(title.c_str());
					SC_OUTPUT("*** ");
					FOR(i, cmd->debug.len){
						sc_cmd_value_t *data = &cmd->debug.data_list[i];
						if(data->type == SC_VT_VAR){
							SC_OUTPUT("%s( ", data->name);
							if(variable_pool.count(data->name)){
								sc_cmd_value_t *hex_data = &variable_pool[data->name];
								assert(hex_data->type == SC_VT_HEX);
								FOR(j, hex_data->data_len){
									SC_OUTPUT("%02X ", hex_data->data_hex[j]);
								}
							}else{
								SC_OUTPUT("Undefined");
							}
							SC_OUTPUT(") ");
						}else{ /* SC_VT_DEF */
							SC_OUTPUT("%s ", data->name);
						}
					}
					SC_OUTPUT("\n");
					if(mtx)mtx->unlock();
					if(cmd->type == SC_CMD_EXIT) return script_end(data);
					break;
				case SC_CMD_REMARK:
					break;
				case SC_CMD_DELAY:{
					vector<uint8_t> send_data;
					send_data.push_back(SC_CMD_DELAY);
					send_data.push_back((cmd->delay.delay_ms >> 0) & 0xFF);
					send_data.push_back((cmd->delay.delay_ms >> 8) & 0xFF);
					send_data.push_back((cmd->delay.delay_ms >>16) & 0xFF);
					send_data.push_back((cmd->delay.delay_ms >>24) & 0xFF);
					data.push_back(send_data);
					}break;
			}
			if(++current_pos == cmd_lines.size()){ return script_end(data); }
		}
		//Process received data
		if(pending_num == 0){
			pending_flag = ~0ul;
			for(int i = current_pos;i < cmd_lines.size() && (cmd_lines[i]->type == SC_CMD_RECV ||
						cmd_lines[i]->type == SC_CMD_IGNORE);i++){
				pending_num = i - current_pos + 1;
				if(cmd_lines[i]->type == SC_CMD_IGNORE){
					pending_flag &= ~(1<<(pending_num-1));
				}
			}
		}
		if(received && recv_len){
			char processed_flag = false;
			FOR(cur_idx, pending_num){
				sc_cmd_recv_t *list = &cmd_lines[current_pos+cur_idx]->recv;
				if(!(pending_flag & (1<<cur_idx)) && cmd_lines[current_pos+cur_idx]->type == SC_CMD_RECV) continue;
				int recv_pos = 0;
				char error = false;
				FOR(i, list->len){
					sc_cmd_value_t val = list->data_list[i];
					switch(val.type){
						case SC_VT_HEX:
							if(val.data_len <= recv_len - recv_pos &&
									!memcmp(&received[recv_pos], val.data_hex, val.data_len)){
								recv_pos += val.data_len;
								break;
							}
							error = true;
							break;
						case SC_VT_VAR:
							if(variable_pool.count(val.name)){
								sc_cmd_value_t &old_val = variable_pool[val.name];
								if(old_val.data_len <= recv_len - recv_pos &&
										!memcmp(&received[recv_pos], old_val.data_hex, old_val.data_len)){
									recv_pos += old_val.data_len;
									break;
								}
							}
							error = true;
							break;
						case SC_VT_DEF:
							if(!variable_pool.count(val.name)){
								sc_cmd_value_t new_val = { SC_VT_HEX };
								new_val.name = (char*)calloc(1, val.name_len);
								memcpy(new_val.name, val.name, val.name_len);
								variable_pool[val.name] = new_val;
							}
							sc_cmd_value_t &new_val = variable_pool[val.name];
							if(val.data_len > 0 && val.data_len <= recv_len - recv_pos){
								new_val.data_len = val.data_len;
								new_val.data_hex = (uint8_t*)realloc(new_val.data_hex, new_val.data_len);
								memcpy(new_val.data_hex, &received[recv_pos], new_val.data_len);
								recv_pos += new_val.data_len;
								break;
							}else if(val.data_len < 0 && abs(val.data_len) < recv_len - recv_pos){
								new_val.data_len = recv_len - recv_pos + val.data_len;
								new_val.data_hex = (uint8_t*)realloc(new_val.data_hex, new_val.data_len);
								memcpy(new_val.data_hex, &received[recv_pos], new_val.data_len);
								recv_pos += new_val.data_len;
								break;
							}else if(val.data_len == 0){
								new_val.data_len = recv_len - recv_pos;
								new_val.data_hex = (uint8_t*)realloc(new_val.data_hex, new_val.data_len);
								memcpy(new_val.data_hex, &received[recv_pos], new_val.data_len);
								recv_pos += new_val.data_len;
								break;
							}
							error = true;
							break;
					}
					if(error){ break; }
				}
				if(!error){
					pending_flag &= ~(1<<cur_idx);
					processed_flag = true;
					if(((~(~0 << pending_num)) & pending_flag) == 0){
						current_pos += pending_num;
						pending_num = 0;
						return get_send_data(NULL, 0, data);
					}
					break;
				}
			}
			if(!processed_flag){
				if(mtx)mtx->lock();
				if(title.length()) SC_OUTPUT(title.c_str());
				SC_OUTPUT("Fatal error: Unexpected data received!\n\t");
				FOR(i, recv_len){
					SC_OUTPUT("%02X ", received[i]);
				}
				SC_OUTPUT("\n");
				if(mtx)mtx->unlock();
				return script_end(vector<vector<uint8_t>>{});
				// TODO: Global callback check
			}
		}
		if(((~(~0 << pending_num)) & pending_flag) != 0){
			vector<uint8_t> send_data;
			send_data.push_back(SC_CMD_RECV);
			data.push_back(send_data);
		}

		return data;
	}
	void dump_2_file(const string& filename){
		FILE *fp = fopen(filename.c_str(), "w");
		if(fp){
			FORBE(cmd_line, cmd_lines){
				fputs((*cmd_line)->raw, fp);
			}
		}
		fclose(fp);
	}
};
#endif /* __SCRIPT_PARSE_H__ */

