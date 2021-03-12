LIBROOT 	= /usr/local/lib
CXXFLAGS 	= -Wall -Wextra -Wpedantic -std=c++11 -Os -O2 -fpermissive -lpthread -ltbb
LDFLAGS 	= -L$(LIBROOT) -I$(LIBROOT) -ltins -lpcap

PROGS 	= asura23

UNAME := $(shell uname -s)
ifeq ($(UNAME), $(filter $(UNAME), Darwin FreeBSD))
	CC = clang
else
	CC = gcc
endif


$(PROGS): asura23.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)
	strip -s $(PROGS)
clean:
	rm -f $(PROGS) *.o
