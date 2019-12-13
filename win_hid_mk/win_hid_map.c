#ifndef __WIN_HID_MAP__
#define __WIN_HID_MAP__
struct win_key_map{
	int win_scanCode;
	int hid_code;
	char *remark;
};
struct win_key_map win_key_map[] = {
    {30, 0x04,  " Keyboard a and A "},
    {48, 0x05,  " Keyboard b and B "},
    {46, 0x06,  " Keyboard c and C "},
    {32, 0x07,  " Keyboard d and D "},
    {18, 0x08,  " Keyboard e and E "},
    {33, 0x09,  " Keyboard f and F "},
    {34, 0x0a,  " Keyboard g and G "},
    {35, 0x0b,  " Keyboard h and H "},
    {23, 0x0c,  " Keyboard i and I "},
    {36, 0x0d,  " Keyboard j and J "},
    {37, 0x0e,  " Keyboard k and K "},
    {38, 0x0f,  " Keyboard l and L "},
    {50, 0x10,  " Keyboard m and M "},
    {49, 0x11,  " Keyboard n and N "},
    {24, 0x12,  " Keyboard o and O "},
    {25, 0x13,  " Keyboard p and P "},
    {16, 0x14,  " Keyboard q and Q "},
    {19, 0x15,  " Keyboard r and R "},
    {31, 0x16,  " Keyboard s and S "},
    {20, 0x17,  " Keyboard t and T "},
    {22, 0x18,  " Keyboard u and U "},
    {47, 0x19,  " Keyboard v and V "},
    {17, 0x1a,  " Keyboard w and W "},
    {45, 0x1b,  " Keyboard x and X "},
    {21, 0x1c,  " Keyboard y and Y "},
    {44, 0x1d,  " Keyboard z and Z "},
    {2, 0x1e,  " Keyboard 1 and ! "},
    {3, 0x1f,  " Keyboard 2 and @ "},
    {4, 0x20,  " Keyboard 3 and # "},
    {5, 0x21,  " Keyboard 4 and $ "},
    {6, 0x22,  " Keyboard 5 and % "},
    {7, 0x23,  " Keyboard 6 and ^ "},
    {8, 0x24,  " Keyboard 7 and & "},
    {9, 0x25,  " Keyboard 8 and * "},
    {10, 0x26,  " Keyboard 9 and ( "},
    {11, 0x27,  " Keyboard 0 and ) "},
    {28, 0x28,  " Keyboard Return (ENTER) "},
    {1, 0x29,  " Keyboard ESCAPE "},
    {14, 0x2a,  " Keyboard DELETE (Backspace) "},
    {15, 0x2b,  " Keyboard Tab "},
    {57, 0x2c,  " Keyboard Space bar "},
    {12, 0x2d,  " Keyboard - and _ "},
    {13, 0x2e,  " Keyboard = and + "},
    {26, 0x2f,  " Keyboard [ and { "},
    {27, 0x30,  " Keyboard ] and } "},
    {43, 0x31,  " Keyboard \\ and | "},
    {39, 0x33,  " Keyboard ; and , "},
    {40, 0x34,  " Keyboard ' and \" "},
    {41, 0x35,  " Keyboard ` and ~ "},
    {51, 0x36,  " Keyboard , and < "},
    {52, 0x37,  " Keyboard . and > "},
    {53, 0x38,  " Keyboard / and ? "},
    {58, 0x39,  " Keyboard Caps Lock "},
    {59, 0x3a,  " Keyboard F1 "},
    {60, 0x3b,  " Keyboard F2 "},
    {61, 0x3c,  " Keyboard F3 "},
    {62, 0x3d,  " Keyboard F4 "},
    {63, 0x3e,  " Keyboard F5 "},
    {64, 0x3f,  " Keyboard F6 "},
    {65, 0x40,  " Keyboard F7 "},
    {66, 0x41,  " Keyboard F8 "},
    {67, 0x42,  " Keyboard F9 "},
    {68, 0x43,  " Keyboard F10 "},
    {87, 0x44,  " Keyboard F11 "},
    {88, 0x45,  " Keyboard F12 "},
    {0x80|55, 0x46,  " Keyboard Print Screen "},
    {0x80|82, 0x49,  " Keyboard Insert "},
    {0x80|71, 0x4a,  " Keyboard Home "},
    {0x80|73, 0x4b,  " Keyboard Page Up "},
    {0x80|83, 0x4c,  " Keyboard Delete Forward "},
    {0x80|79, 0x4d,  " Keyboard End "},
    {0x80|81, 0x4e,  " Keyboard Page Down "},
    {0x80|77, 0x4f,  " Keyboard Right Arrow "},
    {0x80|75, 0x50,  " Keyboard Left Arrow "},
    {0x80|80, 0x51,  " Keyboard Down Arrow "},
    {0x80|72, 0x52,  " Keyboard Up Arrow "},
    {69, 0x53, " Keypad num lock "},
    {0x80|53, 0x54, " Keypad / "},
    {55, 0x55, " Keypad * "},
    {74, 0x56, " Keypad - "},
    {78, 0x57, " Keypad + "},
    {0x80|28, 0x58, " Keypad enter "},
    {79, 0x1e/*0x59*/, " Keypad 1 "},
    {80, 0x1f/*0x5A*/, " Keypad 2 "},
    {81, 0x20/*0x5b*/, " Keypad 3 "},
    {75, 0x21/*0x5c*/, " Keypad 4 "},
    {76, 0x22/*0x5d*/, " Keypad 5 "},
    {77, 0x23/*0x5e*/, " Keypad 6 "},
    {71, 0x24/*0x5f*/, " Keypad 7 "},
    {72, 0x25/*0x60*/, " Keypad 8 "},
    {73, 0x26/*0x61*/, " Keypad 9 "},
    {82, 0x27/*0x62*/, " Keypad 0 "},
    {83, 0x37/*0x63*/, " Keypad . "},
    {29, 0xe0,  " Keyboard Left Control "},
    {42, 0xe1,  " Keyboard Left Shift "},
    {56, 0xe2,  " Keyboard Left Alt "},
    {91, 0xe3,  " Keyboard Left GUI "},
    {29, 0xe4,  " Keyboard Right Control "},
    {54, 0xe5,  " Keyboard Right Shift "},
    {56, 0xe6,  " Keyboard Right Alt "},
    {92, 0xe3,  " Keyboard Right GUI "},
};

#define PARAM_KEY_DOWN    0
#define PARAM_KEY_UP      1
#define PARAM_KEY_MASK    0x100
#define PARAM_MOUSE_MOVE  0x200
#define PARAM_MOUSE_LB_D  0x201
#define PARAM_MOUSE_LB_U  0x202
#define PARAM_MOUSE_RB_D  0x204
#define PARAM_MOUSE_RB_U  0x205
#define PARAM_MOUSE_MB_D  0x207
#define PARAM_MOUSE_MB_U  0x208
#define PARAM_MOUSE_WHELL 0x20A
#define PARAM_MOUSE_FUNCD 0x20B
#define PARAM_MOUSE_FUNCU 0x20C

#endif /* __WIN_HID_MAP__ */
