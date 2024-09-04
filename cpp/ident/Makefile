# Ident project

CC=g++

CFLAGS=-std=c++1z -Wall

LIBS=pthread

SOURCES=main.cxx ident.cxx

OBJECTS=$(SOURCES:.cpp=.o)
	EXECUTABLE=ident

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(CFLAGS) $(OBJECTS) -l$(LIBS) -o $@

