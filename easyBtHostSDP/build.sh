cp ../easyBtHost/easy*.c .
cp ../easyBtHost/easy*.h .
cp ../easyBtHost/eb_*.c .
cp ../easyBtHost/eb_*.h .
cp ../easyBtHost/usb . -r

gcc \
	main.c sdp.c \
	easy*.c \
	eb_*.c \
	usb/*.c \
	-I./usb -L./usb -lpthread -Wall --std=c99 \
	-m32 -lusb \
#	-lusb-1.0 -fsanitize=address -fsanitize-memory-track-origins -fno-omit-frame-pointer  -O1 -g \

rm -rf usb easy*.c easy*.h eb_*.c eb_*.h *.def

