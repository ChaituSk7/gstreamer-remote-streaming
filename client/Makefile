CC = g++
path = src
header = ./includes/
LIBS = `pkg-config --cflags --libs gstreamer-1.0`

all: remotesrc.o remotesrc.so remotemp3.o remotemp3.so remoteWebM.o remoteWebM.so remoteAvi.o remoteAvi.so thumbnail.o thumbnail.so exe 

remotesrc.o:	$(path)/remotesrc.cpp
	$(CC) -c $(path)/remotesrc.cpp $(LIBS) -fPIC -I $(header)
remotesrc.so:	remotesrc.o
	$(CC) -shared -o libremotesrc.so remotesrc.o $(LIBS)
remotemp3.o:	$(path)/remotemp3.cpp
	$(CC) -c $(path)/remotemp3.cpp $(LIBS) -fPIC -I $(header)
remotemp3.so:	remotemp3.o
	$(CC) -shared -o libremotemp3.so remotemp3.o $(LIBS)
remoteWebM.o:	$(path)/remoteWebM.cpp
	$(CC) -c $(path)/remoteWebM.cpp $(LIBS) -fPIC -I $(header)
remoteWebM.so:	remoteWebM.o
	$(CC) -shared -o libremoteWebM.so remoteWebM.o $(LIBS)
remoteAvi.o:	$(path)/remoteAvi.cpp
	$(CC) -c $(path)/remoteAvi.cpp $(LIBS) -fPIC -I $(header)
remoteAvi.so:	remoteAvi.o
	$(CC) -shared -o libremoteAvi.so remoteAvi.o $(LIBS)
thumbnail.o:	$(path)/thumbnail.cpp
	$(CC) -c $(path)/thumbnail.cpp $(LIBS) -fPIC -I $(header)
thumbnail.so:	thumbnail.o
	$(CC) -shared -o libthumbnail.so thumbnail.o $(LIBS)
exe: main.cpp 
	$(CC) -o exe main.cpp -lremotesrc -lremotemp3 -lremoteWebM -lremoteAvi -lthumbnail $(LIBS) -I $(header) -L .
run: exe
	./exe
clean:
	rm -rf *.o *.so exe