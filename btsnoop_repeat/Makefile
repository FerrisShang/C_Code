all:
	gcc parse_snoop.c -lasan -g -o parse_snoop -fsanitize=address -fno-omit-frame-pointer -ggdb -Wall
	gcc btsnoop_repeat.c bt_usb.c btsnoop_rec.c param.c -lasan -g -lpthread -lusb-1.0 -o btsnoop_repeat -fsanitize=address -fno-omit-frame-pointer -ggdb -Wall
