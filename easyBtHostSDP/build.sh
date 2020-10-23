cp ../easyBtHost/easy*.c .
cp ../easyBtHost/easy*.h .
cp ../easyBtHost/eb_*.c .
cp ../easyBtHost/eb_*.h .
cp ../easyBtHost/usb . -r

clang \
	main.c sdp.c \
	easy*.c \
	eb_*.c \
	usb/*.c \
	-I./usb -L./usb -lpthread -lusb-1.0 -L. \
    -Wall -fsanitize=address -fsanitize-memory-track-origins -fno-omit-frame-pointer  -O1 -g \

#rm -rf usb easy*.c easy*.h eb_*.c eb_*.h *.def

