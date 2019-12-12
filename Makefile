CXX = g++
CC = gcc
CFLAGS = -c -O3 -Wall -std=c++11 -pthread -ldl
LDFLAGS = -O2 -pthread -ldl
SOURCES = cppblog.c sqlite3.c

OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = cppblog.cgi

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	rm -rf *.o
sqlite3.o: sqlite3.c
	$(CC) -c -O2 -Wall -pthread -ldl $< -o $@

.c.o:
	$(CXX) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)