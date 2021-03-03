CC = g++
CFlags = -g -Wall -Werror -std=c++17
CLibs = -lboost_serialization -lcryptopp

main: main.cpp gitletobj.h gitletobj.cpp utils.h utils.cpp
	$(CC) $(CFlags) -o main main.cpp gitletobj.cpp utils.cpp $(CLibs)
unittest: unittest.cpp gitletobj.h gitletobj.cpp utils.h utils.cpp
	$(CC) $(CFlags) -o unittest unittest.cpp gitletobj.cpp utils.cpp $(CLibs)
clean:
	rm -rf .gitlet
	rm main
	rm unittest
format:
	clang-format -i gitletobj.*
	clang-format -i utils.*
	clang-format -i main.cpp
	clang-format -i unittest.cpp
