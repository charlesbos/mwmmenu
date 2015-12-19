CC = g++
CXXFLAGS = -Wall -std=c++11 -lboost_system -lboost_filesystem

all: 
	$(CC) $(CXXFLAGS) Main.cpp DesktopFile.cpp MenuWriter.cpp -o mwmmenu

clean:
	if [ -f mwmmenu ]; then rm mwmmenu; fi
