CXXFLAGS := -std=c++17 -Wall -Werror -pedantic -g -O0
CXXFLAGS_TEST := ${CXXFLAGS} -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined

COMMON_HEADER := thirdparty.h

COMMON_HEADER_TEST := ${COMMON_HEADER} test_thirdparty.h

.PHONY: all clean

all: sudoku unittest

clean:
	-rm -f *.o sudoku unittest

sudoku: sudoku.cpp sudoku.h ${COMMON_HEADER}
	g++ ${CXXFLAGS} -o $@ $<

unittest: test_main.cpp ${COMMON_HEADER_TEST}
	g++ ${CXXFLAGS_TEST} -o $@ $<
