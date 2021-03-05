CPPC = g++
CPPFlags = -g -Wall -Werror -std=c++17
CPPLibs = -lboost_serialization -lcryptopp

main: main.cpp gitletobj.h gitletobj.cpp utils.h utils.cpp
	$(CPPC) $(CPPFlags) -o main main.cpp gitletobj.cpp utils.cpp $(CPPLibs)
unittest: unittest.cpp gitletobj.h gitletobj.cpp utils.h utils.cpp
	$(CPPC) $(CPPFlags) -o unittest unittest.cpp gitletobj.cpp utils.cpp $(CPPLibs)
clean:
	rm -rf .gitlet
	rm main
	rm unittest
format:
	clang-format -i gitletobj.*
	clang-format -i utils.*
	clang-format -i main.cpp
	clang-format -i unittest.cpp
