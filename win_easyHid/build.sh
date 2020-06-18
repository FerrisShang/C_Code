gcc -shared dll.c -o dll.dll -static-libgcc -m32 -Wl,--output-def,libdll.def,--out-implib,libdll.a,--add-stdcall-alias

cp ../easyBtHost/easy*.c .
cp ../easyBtHost/easy*.h .
cp ../easyBtHost/eb_*.c .
cp ../easyBtHost/eb_*.h .
cp ../win_hid_mk/win_hid_map.c .
cp ../easyBtHost/usb . -r


gcc -O3 \
	main.c \
	easy*.c \
	eb_*.c \
	usb/*.c \
	-I./usb -L./usb -lusb -static-libgcc -mwindows -m32 -L. -std=c99 -o win_easyHid.exe

rm -rf usb easy*.c easy*.h eb_*.c eb_*.h *.def win_hid_map.c

