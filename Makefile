HTMLCXX_PREFIX = $(HOME)/htmlcxx
HTMLCXX_INCLUDE = $(HTMLCXX_PREFIX)/include
HTMLCXX_LIB = $(HTMLCXX_PREFIX)/lib

CXX = g++
CXXFLAGS = -Wall -O2 -g -std=gnu++17 -ftrapv -I$(HTMLCXX_INCLUDE) -L$(HTMLCXX_LIB) -Wl,-rpath,$(HTMLCXX_LIB)
LDLIBS = -lhtmlcxx

all : rater
clean:
	rm -f rater *.o

rater : rater.cpp

