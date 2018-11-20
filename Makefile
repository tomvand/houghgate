CXX = g++
CC = gcc
CFLAGS = 
CXXFLAGS = `pkg-config opencv --cflags`
CXXLIBS = `pkg-config opencv --libs`

OUTPUTDIR = ./build/
MKDIR = mkdir -p $(OUTPUTDIR)
OBJECTC = $(OUTPUTDIR)houghgate.o
BIN = $(OUTPUTDIR)test

CSOURCES = $(wildcard *.c)
CXXSOURCES = $(wildcard *.cpp)
HEADERS = $(wildcard *.h)

$(BIN): $(OBJECTC) $(CXXSOURCES)
	$(MKDIR)
	$(CXX) $(OBJECTC) $(CXXSOURCES) -o $(BIN) $(CXXLIBS)

$(OBJECTC): $(CSOURCES) $(HEADERS)
	$(MKDIR)
	$(CC) $(CSOURCES) $(CFLAGS) -c -o $(OBJECTC)

.PHONY: clean
clean:
	$(RM) -rf $(OUTPUTDIR)
	$(RM) ./*.gc??
	$(RM) ./*.o
