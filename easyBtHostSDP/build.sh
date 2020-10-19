cp ../easyBtHost/easy*.c .
cp ../easyBtHost/easy*.h .
cp ../easyBtHost/eb_*.c .
cp ../easyBtHost/eb_*.h .
cp ../easyBtHost/usb . -r

gcc -O0 -g \
	main.c sdp.c \
	easy*.c \
	eb_*.c \
	usb/*.c \
	-I./usb -L./usb -lpthread -lusb-1.0 -L.

#rm -rf usb easy*.c easy*.h eb_*.c eb_*.h *.def

