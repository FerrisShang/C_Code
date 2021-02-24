cp ../script_parse/str_utils.? .
cp ../script_parse/script_parse.h .
cp ../win_bt_uart/squeue.h .
cp ../win_bt_uart/win_serial.h .
cp ../filelist/file_list.h .

g++ main.c str_utils.c --std=c++11 -O3 -o script_debug
strip script_debug.exe
rm str_utils.? script_parse.h squeue.h win_serial.h file_list.h -f
