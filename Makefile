CC = g++ -Wall -g -DDEBUG -fPIC
CFLAGS = -c -O3 -fomit-frame-pointer -ffast-math -std=c++0x
# CFLAGS = -c -std=c++0x
EXENAME = lazyRNWRM
OBJECTS = myIO.o lazyRNWRM.o

$(EXENAME): $(OBJECTS)
	$(CC) $(CCFLAGS) -o $(EXENAME) $(OBJECTS)

lazyRNWRM.o: lazyRNWRM.cc lazyRNWRM.h 
	${CC} ${CFLAGS} -o lazyRNWRM.o lazyRNWRM.cc 

myIO.o: myIO.cc myIO.h 
	${CC} ${CFLAGS} -o myIO.o myIO.cc 

clean:
	rm -f *.o *~ myIO.o lazyRN-WRM.o
