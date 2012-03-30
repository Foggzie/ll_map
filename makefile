CC=g++
EXE=elevation_demo

SRC=main.cpp

CFLAGS=-g
LFLAGS=-g -c

LIBS=-lcurl

$(EXE) : $(SRC) ll_map.o
	$(CC) $(CFLAGS) $(LIBS) ll_map.o $< -o $@

ll_map.o : ll_map.cpp
	$(CC) $(LFLAGS) $< -o $@

lib :
	rm libllmap*
	g++ -fPIC -c ll_map.cpp -lcurl
	g++ -shared -Wl,-soname,libllmap.so.1 -o libllmap.so.1.0 ll_map.o
	ln -s libllmap.so.1.0 libllmap.so.1
	ln -s libllmap.so.1 libllmap.so
	mv libllmap* /usr/local/lib/
