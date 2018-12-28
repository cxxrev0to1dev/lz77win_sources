# Variables.
UNAME := $(shell uname)

CXX=g++
CXXFLAGS_COMMON=-D_UNIX -L../src/.libs -lckcore -I../include
CXXFLAGS_LINUX=$(CXXFLAGS_COMMON) -lpthread -ldl

ifeq ($(UNAME), Linux)
CXXFLAGS=$(CXXFLAGS_LINUX)
else
CXXFLAGS=$(CXXFLAGS_COMMON)
endif

# Targets.
all: clean test streambench smallclient filetester

clean:
	rm -f bin/test bin/streambench test.cc

test:
	cxxtestgen.pl --error-printer -o test.cc cast.hh convert.hh directory.hh file.hh linereader.hh path.hh process.hh stream.hh string.hh thread.hh threadpool.hh
	$(CXX) $(CXXFLAGS) test.cc -o bin/test

streambench:
	$(CXX) $(CXXFLAGS) streambench.cc -o bin/streambench

smallclient:
	$(CXX) $(CXXFLAGS) smallclient.cc -o bin/smallclient

filetester:
	$(CXX) $(CXXFLAGS) filetester.cc -o bin/filetester
