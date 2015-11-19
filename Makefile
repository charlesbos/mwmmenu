CC = g++
CXXFLAGS = -Wall

all: 
	$(CC) $(CXXFLAGS) Main.cpp DesktopFile.cpp MwmMenuWriter.cpp -o mwmmenu

clean:
	if [ -f mwmmenu ]; then rm mwmmenu ; fi