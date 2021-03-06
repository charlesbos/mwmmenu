CC = g++
CXXFLAGS = -s -Wall -std=c++98 -pedantic-errors -O3 -lboost_system -lboost_filesystem

all: 
	$(CC) src/Main.cpp src/DesktopFile.cpp src/MenuWriter.cpp src/Category.cpp -o mwmmenu $(CXXFLAGS)

clean:
	rm -f mwmmenu
