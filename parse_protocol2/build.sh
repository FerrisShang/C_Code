gcc *.c --std=c99 -O3 -o parse_protocol.exe
strip parse_protocol.exe

gcc -shared *.c -O3 -o parse_protocol.dll -static-libgcc --std=c99
strip parse_protocol.dll

