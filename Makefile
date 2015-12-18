CC = g++
CXXFLAGS = -Wall

all: 
	$(CC) $(CXXFLAGS) Main.cpp DesktopFile.cpp MenuWriter.cpp -o mwmmenu

clean:
	if [ -f mwmmenu ]; then rm mwmmenu; fi
