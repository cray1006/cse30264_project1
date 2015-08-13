all:  myftp

myftp:  myftp.o clientMembers.o
	g++ -o myftp myftp.o clientMembers.o -std=c++0x -lmhash -g

clientMembers.o:  clientMembers.cpp clientMembers.h
	g++ -c clientMembers.cpp -std=c++0x -lmhash -g

myftp.o:   myftp.cpp clientMembers.h
	g++ -c myftp.cpp -std=c++0x -lmhash -g

clean:  
	rm -f core* myftp *.o
