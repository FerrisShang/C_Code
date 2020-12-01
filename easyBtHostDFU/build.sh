cp ../easyBtHost/easy*.c .
cp ../easyBtHost/easy*.h .
cp ../easyBtHost/eb_*.c .
cp ../easyBtHost/eb_*.h .
cp ../easyBtHost/usb . -r
cp ../bt_encrypt/alg . -r

gcc \
	main.c \
	*dfu*.c \
	easy*.c \
	eb_*.c \
	usb/*.c \
	alg/zip.c \
	-Ialg -I./usb -L./usb -lpthread -Wall --std=c99 -o dfu.exe \
#	-lusb-1.0 -fsanitize=address -fsanitize-memory-track-origins -fno-omit-frame-pointer  -O1 -g \
	-m32 -lusb \

rm -rf usb easy*.c easy*.h eb_*.c eb_*.h *.def alg
