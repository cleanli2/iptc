IPTC_SRCS := $(wildcard *cpp)
IPTC_H := $(wildcard *h)
IPTC_OBJS := $(subst .cpp,.o, $(IPTC_SRCS))
TARGET_NAME := iptc
#LIB_NAME := c:/MinGW/lib/libws2_32.a
GIT_SHA1="$(shell git log --format='_%h ' -1)"
DIRTY="$(shell git diff --quiet || echo 'dirty')"
CLEAN="$(shell git diff --quiet && echo 'clean')"
CPPFLAGS+=-DGIT_SHA1=\"$(GIT_SHA1)$(DIRTY)$(CLEAN)\"

$(TARGET_NAME):$(IPTC_OBJS) iptc_res.o
	g++ -mwindows $(IPTC_OBJS) $(LIB_NAME) iptc_res.o -l Gdi32 -o $@

iptc_res.o:iptc.rc iptc.ico iptc_rc.h
	windres iptc.rc -I. -o iptc_res.o

%.o:%.cpp $(IPTC_H)
	g++ -mwindows -Iinclude $(CPPFLAGS) -c -o $@ $<

clean:
	rm -rf *.o $(TARGET_NAME)

