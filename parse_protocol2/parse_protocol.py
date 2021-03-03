from ctypes import *


def b2s(b):
    return b.decode("utf-8")


class ParseRes(Structure):
    _fields_ = [
        ("error_msg", POINTER(c_char_p)),
        ("error_num", c_int),
        ("warning_msg", POINTER(c_char_p)),
        ("warning_num", c_int),
        ]


class ParsedItem(Structure):
    _fields_ = [
        ("param_type", c_void_p),
        ("out_priority", c_int),
        ("indent", c_int),
        ("data", POINTER(c_ubyte)),
        ("bit_width", c_int),
        ("title", c_char_p),
        ("lines", POINTER(c_char_p)),
        ("line_num", c_int),
        ]

class ParsedData(Structure):
    pass
ParsedData._fields_ = [
        ("item", POINTER(ParsedItem)),
        ("next", POINTER(ParsedData)),
        ]


class ParseProtocol:
    dll_handle = None

    def __init__(self, dll_name):
        if ParseProtocol.dll_handle is None:
            ParseProtocol.dll_handle = cdll.LoadLibrary(dll_name)
            ParseProtocol.dll_handle.parse_init.restype = ParseRes
            ParseProtocol.dll_handle.unpack.argtypes = [POINTER(c_ubyte), c_int]
            ParseProtocol.dll_handle.unpack.restype = POINTER(ParsedData)
            ParseProtocol.dll_handle.unpack_free.argtypes = [POINTER(ParsedData)]
            ParseProtocol.parse_init()

    def __del__(self):
        if ParseProtocol.dll_handle is not None:
            ParseProtocol.dll_handle.parse_free()

    @staticmethod
    def parse_init():
        res = ParseProtocol.dll_handle.parse_init()
        if res.error_num > 0:
            for i in range(res.error_num):
                print(b2s(res.error_msg[i]))
            exit(-1)

        if res.warning_num > 0:
            for i in range(res.warning_num):
                print(b2s(res.warning_msg[i]))
        print('')

    @staticmethod
    def unpack(data):
        assert type(data) is bytes
        buffer = (c_ubyte * len(data)).from_buffer(bytearray(data))
        p_res = ParseProtocol.dll_handle.unpack(buffer, len(buffer))
        res = p_res[0]
        print("-"*24 + ' : ' + ' '.join(['%02X' % x for x in data]))
        while res.next:
            res = res.next[0]
            item = res.item[0]
            for i in range(item.line_num):
                if item.out_priority >= 0:
                    print('{:24s} : {}'.format(b2s(item.title), b2s(item.lines[i])))
        ParseProtocol.dll_handle.unpack_free(p_res)

    class Item:
        def __init__(self):
            self.out_priority = 0
            self.indent = 0
            self.data = 0
            self.bit_width = 0
            self.title = ''
            self.lines = []

    class Unpack:
        def __init__(self, data):
            assert type(data) is bytes
            self.bdata = data
            buffer = (c_ubyte * len(data)).from_buffer(bytearray(data))
            self.p_res = ParseProtocol.dll_handle.unpack(buffer, len(buffer))
            self.items = []

            res = self.p_res[0]
            while res.next:
                res = res.next[0]
                c_item = res.item[0]
                new_item = ParseProtocol.Item()
                new_item.out_priority = c_item.out_priority
                new_item.indent = c_item.indent
                new_item.data = c_item.data
                new_item.bit_width = c_item.bit_width
                new_item.title = b2s(c_item.title)
                for i in range(c_item.line_num):
                    new_item.lines.append(b2s(c_item.lines[i]))
                self.items.append(new_item)

        def __del__(self):
            ParseProtocol.dll_handle.unpack_free(self.p_res)


p = ParseProtocol('./parse_protocol.dll')
d1 = b'\x01\x03\x0c\x00'
d2 = b'\x04\x0e\x04\x01\x03'
d3 = b'\x01\x36\x20\x19\x01\x00\x00\x90\x01\x00\x58\x02\x00\x07\x00\x00\x24\xc2\x28\x66\xbf\x02\x00\x7f\x01\x00\x01\x0f\x00'
d4 = b'\x04\x3e\x39\x0d\x01\x10\x00\x01\xe8\x3a\x5c\xca\xd3\x54\x01\x00\xff\x7f\xc6\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1f\x1e\xff\x06\x00\x01\x09\x20\x02\x02\xd3\xa3\xf6\x81\xd0\xc0\x08\x94\x78\x1f\x52\xe9\xd4\x02\xb0\x8b\xe2\x29\xdd\x73\x63\x43'


def test(data):
    print("-"*24 + ' : ' + ' '.join(['%02X' % x for x in data]))
    d = ParseProtocol.Unpack(data)
    for item in d.items:
        if item.out_priority >= 0:
            for line in item.lines:
                print('{:24s} : {}'.format(item.title, line))


test(d1)
test(d2)
test(d3)
test(d4)
