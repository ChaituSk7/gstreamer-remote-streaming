CC = g++
path = src
header = ./include/
LIBS = `pkg-config --cflags --libs gstreamer-1.0 gstreamer-pbutils-1.0`

all: hostmp4.o hostmp3.o hostwebm.o hostavi.o metadata.o padprobe.o keyboardhandler.o thumbnail.o hostthumbnail.o hostmp4.so hostmp3.so hostwebm.so hostavi.so metadata.so padprobe.so keyboard.so thumbnail.so hostthumbnail.so exe

hostmp4.o: $(path)/hostmp4.cpp
	$(CC) -c $(path)/hostmp4.cpp $(LIBS) -fPIC -I $(header)

hostmp3.o:	$(path)/hostmp3.cpp
	$(CC) -c $(path)/hostmp3.cpp $(LIBS) -fPIC -I $(header)

hostwebm.o: $(path)/hostwebm.cpp
	$(CC) -c $(path)/hostwebm.cpp $(LIBS) -fPIC -I $(header)

hostavi.o: $(path)/hostavi.cpp
	$(CC) -c $(path)/hostavi.cpp $(LIBS) -fPIC -I $(header)

metadata.o: $(path)/metadata.cpp 
	$(CC) -c $(path)/metadata.cpp $(LIBS) -fPIC -I $(header)

padprobe.o: $(path)/padprobe.cpp 
	$(CC) -c $(path)/padprobe.cpp $(LIBS) -fPIC -I $(header)

keyboardhandler.o: $(path)/keyboardhandler.cpp
	$(CC) -c $(path)/keyboardhandler.cpp $(LIBS) -fPIC -I $(header)

thumbnail.o: $(path)/thumbnail.cpp
	$(CC) -c $(path)/thumbnail.cpp $(LIBS) -fPIC -I $(header)

hostthumbnail.o: $(path)/hostthumbnail.cpp 
	$(CC) -c $(path)/hostthumbnail.cpp $(LIBS) -fPIC -I $(header)

hostmp4.so:	hostmp4.o
	$(CC) -shared -o libhostmp4.so hostmp4.o $(LIBS)

hostmp3.so:	hostmp3.o
	$(CC) -shared -o libhostmp3.so hostmp3.o $(LIBS)

hostwebm.so: hostwebm.o
	$(CC) -shared -o libhostwebm.so hostwebm.o $(LIBS)

hostavi.so: hostavi.o
	$(CC) -shared -o libhostavi.so hostavi.o $(LIBS)

metadata.so: metadata.o
	$(CC) -shared -o libmetadata.so metadata.o $(LIBS)

padprobe.so: padprobe.o 
	$(CC) -shared -o libpadprobe.so padprobe.o $(LIBS)

keyboard.so: keyboardhandler.o 
	$(CC) -shared -o libkeyboard.so keyboardhandler.o metadata.o $(LIBS)

thumbnail.so: thumbnail.o
	$(CC) -shared -o libthumbnail.so thumbnail.o $(LIBS)

hostthumbnail.so:	hostthumbnail.o
	$(CC) -shared -o libhostthumbnail.so hostthumbnail.o $(LIBS)

exe: main/main.cpp 
	$(CC) -o exe main/main.cpp -lhostmp4 -lhostmp3 -lhostwebm -lhostavi -lmetadata -lpadprobe -lkeyboard -lthumbnail -lhostthumbnail $(LIBS) -I $(header) -L .

clean:
	rm -rf *.o *.so *.jpg exe
