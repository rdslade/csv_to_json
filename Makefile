CXX = g++

CXXFLAGS = -Wall -Werror -pedantic --std=c++11 -g

all: json

json.exe: net485_create_json.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

.SUFFIXES:

.PHONY: clean
clean:
	rm -rvf *.out *.exe *.dSYM *.stackdump
