#Create activity
SEND: 05a00d0d0010000200 A100
RECV: 05A40D10000D000300 {idx, 1} 0100
RECV: 05000D10000D000200 A100
#Start activity
SEND: 05a10d0d0010003a00 A4 {idx} 0205000040004000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
RECV: 05000D10000D000200 A400
SET: TIMEOUT, 5000
FOR: {i,1}, 1, 9999
RECV: 05A70D10000D00 {L,2} {_,2} {ADDR,6} {T,1} {_,8} {RSSI,1} {_,8} {D,0}
DEBUG: {RSSI} {T} {ADDR} {D}
LOOP

