#g++ main.cpp -c -o main.o
#g++ -mwindows main.o -lgdi32 -luser32 -lkernel32 -lcomctl32 -o iptc.exe
g++ -mwindows main.cpp -o iptc.exe
