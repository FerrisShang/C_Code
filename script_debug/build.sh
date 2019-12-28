cp ../script_parse/str_utils.? .
cp ../script_parse/script_parse.h .
cp ../win_bt_uart/squeue.h .
cp ../win_bt_uart/win_serial.h .

g++ main.c str_utils.c --std=c++11 -O3 -o script_debug
rm str_utils.? script_parse.h squeue.h win_serial.h -f
