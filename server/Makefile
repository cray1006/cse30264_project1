all:  myftpd

myftpd:  myftpd.o serverMembers.o
	g++ -o myftpd myftpd.o serverMembers.o -std=c++0x -lmhash -g

serverMembers.o:  serverMembers.cpp serverMembers.h
	g++ -c serverMembers.cpp -std=c++0x -lmhash -g

myftpd.o:   myftpd.cpp serverMembers.h
	g++ -c myftpd.cpp -std=c++0x -lmhash -g

clean:  
	rm -f core* myftpd *.o
