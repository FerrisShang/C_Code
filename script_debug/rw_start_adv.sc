#Create activity
SEND: 05a00d0d0010002800 a0000000000103000000000000000000000000002000000040000000070100000000000000000000
RECV: 05A40D10000D000300 {idx, 1} 0000
RECV: 05000D10000D000200 A000
#Set Adv Data
SEND: 05a60d0d0010000a00 A9 {idx} 0600 0509F09F9882
RECV: 05000D10000D000200 A900
#Set ScanRsp Data
SEND: 05a60d0d0010000a00 AA {idx} 0600 020106000000
RECV: 05000D10000D000200 AA00
#Start activity
SEND: 05A10D0D0010003A00 A4 {idx} 0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
RECV: 05000D10000D000200 A400

