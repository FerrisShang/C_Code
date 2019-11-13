cp ../bt_encrypt/* .
gcc -shared dll.c -o dll.dll -static-libgcc -m32 -Wl,--output-def,libdll.def,--out-implib,libdll.a,--add-stdcall-alias
gcc main.c hid.c bt_encrypt.c btsnoop_rec.c aes.c bt_usb.c -lusb -static-libgcc -mwindows -m32 -L. -o win_hid.exe
