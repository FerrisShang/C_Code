g++ -shared *.cpp --std=c++11 -static-libgcc -Wl,--add-stdcall-alias -m32 -O3 -o Implicit_Send.dll
