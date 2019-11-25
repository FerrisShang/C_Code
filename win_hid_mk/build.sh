cp -r ../bt_encrypt/alg .
cp -r ../bt_encrypt/bt_encrypt.? .
gcc -shared dll.c -o dll.dll -static-libgcc -m32 -Wl,--output-def,libdll.def,--out-implib,libdll.a,--add-stdcall-alias
gcc main.c hid.c bt_encrypt.c btsnoop_rec.c bt_usb.c \
	alg/aescrypt.c alg/aeskey.c alg/aestab2.c alg/cmac.c alg/uECC.c \
	-lusb -static-libgcc -mwindows -m32 -L. -std=c99 -o win_hid.exe
