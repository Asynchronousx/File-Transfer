#MAKEFILE FOR USERCLIENT

PROGNAME = FTClient
OBJECTS  = $(PROGNAME).o ../secure_networking/secure_networking.o
CFLAGS = -g
LIBS = -lpthread

$(PROGNAME): $(OBJECTS)
	gcc -o $(PROGNAME) $(OBJECTS) $(LIBS)
	rm -f $(OBJECTS)

# Clean Rule
clean: 
	rm -f $(OBJECTS) $(PROGNAME)
