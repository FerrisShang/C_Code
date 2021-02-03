echo Compile test_hci_cache
gcc test_hci_cache.c ../usb/hci_cache.c  -I../usb -pthread -Wall -m32 -O3 -o _test_hci_cache
echo Compile test_bt_usb
gcc test_bt_usb.c ../usb/bt_usb.c ../usb/btsnoop_rec.c ../usb/hci_cache.c -I.. -I../usb -L../usb -lusb -m32 -O3 -o _test_bt_usb
echo Compile test_eb_hci
gcc ../eb_hci.c test_eb_hci.c -I.. -pthread -Wall -m32 -O3 -o _test_eb_hci
echo Compile test_l2cap
gcc test_l2cap.c ../eb_l2cap.c -I.. -m32 -o _test_l2cap
