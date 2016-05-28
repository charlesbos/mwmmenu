CC = g++
CXXFLAGS = -Wall -std=c++11 -lboost_system -lboost_filesystem

all: 
	$(CC) $(CXXFLAGS) src/Main.cpp src/DesktopFile.cpp src/MenuWriter.cpp src/Categories.cpp -o mwmmenu

clean:
	if [ -f mwmmenu ]; then rm mwmmenu; fi
