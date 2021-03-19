CPPC = g++
CPPFlags = -g -Wall -Werror -std=c++17
BoostLib = -lboost_serialization 
CryptLib = -lcryptopp
CPPLibs = $(BoostLib) $(CryptLib)

main: main.o gitletobj.o utils.o
	$(CPPC) $(CPPFlags) -o main main.o gitletobj.o utils.o $(CPPLibs)
unittest: unittest.o gitletobj.o utils.o
	$(CPPC) $(CPPFlags) -o unittest unittest.o gitletobj.o utils.o $(CPPLibs)
gitletobj.o: gitletobj.cpp gitletobj.h
	$(CPPC) $(CPPFlags) -c gitletobj.cpp $(BoostLib)
utils.o: utils.cpp utils.h
	$(CPPC) $(CPPFlags) -c utils.cpp $(CPPLibs)
main.o: main.cpp
	$(CPPC) $(CPPFlags) -c main.cpp $(CPPLibs)
unittest.o: unittest.cpp
	$(CPPC) $(CPPFlags) -c unittest.cpp $(CPPLibs)
clean:
	rm -rf .gitlet
	rm *.o
	rm main
	rm unittest
format:
	clang-format -i gitletobj.h
	clang-format -i gitletobj.cpp
	clang-format -i utils.h
	clang-format -i utils.cpp
	clang-format -i main.cpp
	clang-format -i unittest.cpp
