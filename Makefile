CC = g++
CXXFLAGS = -Wall -lboost_system -lboost_filesystem

all: 
	$(CC) $(CXXFLAGS) src/Main.cpp src/DesktopFile.cpp src/MenuWriter.cpp -o mwmmenu

clean:
	if [ -f mwmmenu ]; then rm mwmmenu; fi
