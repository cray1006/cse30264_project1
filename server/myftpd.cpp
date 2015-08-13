//Christopher Ray
//Curt Freeland
//CSE 30264
//26 September 2014

//myftpd.cpp
//Driver program for myftpd.  

/*This is the server program for a simple file transfer application.*/

#include "serverMembers.h"

using namespace std;

int main(int argc, char * argv[])
{
	//declaring variables
	const char * PORT; //port
	char cmdBuff[10]; //holds the command received from the client
	string tempPort = "9499"; //the default port
	char * OUTBUFF, * INBUFF, * HASH, * fileName; //output buffer, input buffer, hash buffer, filename
	int SOCKET, new_SOCKET, cmdBytes, outBytes, inBytes, bytesRec, i, check; //various integers
	int32_t fsize32; //32bit file size
	int16_t fsize16; //16bit file size
	struct timeval time1, time2; //used for calculating time elapsed during transfer
	double totalTime, rate;
	FILE * myFile; //used mainly in dir
	struct sockaddr_storage CLIENT; //holds the client's address information
	socklen_t addrLen = sizeof(CLIENT); 
	
	if(argc == 1) //use default port if no arguments given
	{
		PORT = tempPort.c_str();
	}
	else
	{
		PORT = argv[1]; //use user input port
	}

	SOCKET = heyListen(PORT); //creating and binding server's socket

	while(1)
	{
		if((new_SOCKET = accept(SOCKET, (struct sockaddr * )&CLIENT, &addrLen)) < 0) //accepting a new connection from client
		{
			cout << "accept() error" << endl;
			return -1;
		}
		while(1)
		{
			if((cmdBytes = recv(new_SOCKET, cmdBuff, sizeof(cmdBuff), 0)) < 0) //receiving command from client
			{
				cout << "recv() error, cmd" << endl;
				return -1;
			}
			cmdBuff[cmdBytes] = '\0';

			if((strcmp(cmdBuff, "XIT") == 0) || (strcmp(cmdBuff, "xit") == 0))  //xit
			{
					break; //break out of while loop and wait for new connection
			}
			else if((strcmp(cmdBuff, "DIR") == 0) || (strcmp(cmdBuff, "dir") == 0)) //dir
			{
				fsize32 = getDirSize(myFile);	//getting size of directory			
				if(getDirListing(fsize32, OUTBUFF, myFile) < 0) //putting directory list into buffer
				{
					cout << "error retrieving directory" << endl;
					return -1;
				}
		
				fsize32 = htonl(fsize32);
			
				if((outBytes = send(new_SOCKET, &fsize32, sizeof(fsize32), 0)) < 0) //sending size of dir list
				{
					cout << "send() error, dir fsize32" << endl;
					return -1;
				}

				if((outBytes = send(new_SOCKET, OUTBUFF, ntohl(fsize32), 0)) < 0) //sending directory list
				{
					cout << "send() error dir OUTBUFF" << endl;
					return -1;
				}
			
				delete [] OUTBUFF;	
			}
			else if((strcmp(cmdBuff, "GET") == 0) || (strcmp(cmdBuff, "get") == 0)) //get
			{
				if((inBytes = recv(new_SOCKET, &fsize16, sizeof(fsize16), 0)) < 0) //getting size of filename
				{
					cout << "recv() error, get fsize16" << endl;
					return -1;
				}
				fsize16 = ntohs(fsize16);

				INBUFF = new char[fsize16];
				bytesRec = 0;
				while(bytesRec < fsize16)
				{
					if((inBytes = recv(new_SOCKET, INBUFF, fsize16, 0)) < 0) //getting filename
					{
						cout << "recv() error get INBUFF" << endl;
						return -1;
					}
					bytesRec += inBytes;
				}
				INBUFF[fsize16] = '\0';

				fsize32 = getFileSize(INBUFF);
				fsize32 = htonl(fsize32);
				if((outBytes = send(new_SOCKET, &fsize32, sizeof(fsize32), 0)) < 0) //sending size of file
				{
					cout << "send() error, get fsize32" << endl;
					return -1;
				}
				
				fsize32 = ntohl(fsize32);
				if(fsize32 > 0) //if the file exists...
				{
					if(prepFile(HASH, INBUFF, fsize32, 0) < 0) //generating MD5 hash
					{
						cout << "Error generating hash" << endl;
						return -1;
					}
					if((outBytes = send(new_SOCKET, HASH, 16, 0)) < 0) //sending MD5 hash
					{
						cout << "send() error, get HASH" << endl;
						return -1;
					}

					if(prepFile(OUTBUFF, INBUFF, fsize32, 1) < 0) //generating output buffer from file
					{
						cout << "Error generating file buffer" << endl;
						return -1;
					}
					if((outBytes = send(new_SOCKET, OUTBUFF, fsize32, 0)) < 0) //sending output buffer
					{
						cout << "send() error, get OUTBUFF" << endl;
						return -1;
					}
					delete [] OUTBUFF;
					delete[] HASH;
				}
				delete [] INBUFF;
			}
			else if((strcmp(cmdBuff, "PUT") == 0) || (strcmp(cmdBuff, "put") == 0)) //put
			{
				if((inBytes = recv(new_SOCKET, &fsize16, sizeof(fsize16), 0)) < 0) //getting size of filename
				{
					cout << "recv() error, get fsize16" << endl;
					return -1;
				}
				fsize16 = ntohs(fsize16);

				fileName = new char[fsize16];
				bytesRec = 0;
				while(bytesRec < fsize16)
				{
					if((inBytes = recv(new_SOCKET, fileName, fsize16, 0)) < 0) //receiving filename
					{
						cout << "recv() error get fileName" << endl;
						return -1;
					}
					bytesRec += inBytes;
				}
				fileName[fsize16] = '\0';

				fsize16 = readyCheck(fileName); //checking if write file can be opened
				fsize16 = htons(fsize16);
				if((outBytes = send(new_SOCKET, &fsize16, sizeof(fsize16), 0)) < 0) //sending ack to client	
				{
					cout << "send() error, put fsize16" << endl;
					return -1;
				}

				if((inBytes = recv(new_SOCKET, &fsize32, sizeof(fsize32), 0)) < 0) //receiving size of files
				{
					cout << "recv() error, put fsize32" << endl;
					return -1;
				}

				fsize32 = ntohl(fsize32);
				if(fsize32 >= 0) //this block only executes if a file will actually be sent over
				{
					gettimeofday(&time1, 0);				
					HASH = new char[16];
					bytesRec = 0;
					while(bytesRec < 16)
					{
						if((inBytes = recv(new_SOCKET, HASH, 16, 0)) < 0) //receiving MD5 hash
						{
							cout << "recv() error, get HASH" << endl;
							return -1;
						}
						bytesRec += inBytes;
					}
					HASH[16] = '\0';

					INBUFF = new char[fsize32];
					bytesRec = 0;
					i = 0;
					while(bytesRec < fsize32)
					{
						if((inBytes = recv(new_SOCKET, INBUFF, fsize32, 0)) < 0) //receiving file buffer
						{
							cout << "recv() error, get INBUFF" << endl;
							return -1;
						}
						bytesRec += inBytes;
						saveFile(fileName, INBUFF, inBytes, i);
						i++;
					}

					if(prepFile(OUTBUFF, fileName, fsize32, 0) < 0) //generating server's MD5 hash
					{
						cout << "error generating hash" << endl;
						return -1;
					}

					if(strcmp(OUTBUFF, HASH) == 0) //comparing server and client hashes
					{
						fsize16 = htons(1);
						//let client know that transfer was successful
						if((outBytes = send(new_SOCKET, &fsize16, sizeof(fsize16), 0)) < 0)	
						{
							cout << "send() error, put fsize16" << endl;
							return -1;
						} 
			
						gettimeofday(&time2, 0); //sending throughput info
						totalTime = ((time2.tv_sec - time1.tv_sec) * 1000000) + (time2.tv_usec - time1.tv_usec);
						totalTime = htonl(totalTime);
						if((outBytes = send(new_SOCKET, &totalTime, sizeof(totalTime), 0)) < 0)	
						{
							cout << "send() error, put fsize16" << endl;
							return -1;
						}
					}
					else
					{
						fsize16 = htons(-1); //let client know transfer was not successful
						if((outBytes = send(new_SOCKET, &fsize16, sizeof(fsize16), 0)) < 0)	
						{
							cout << "send() error, put fsize16" << endl;
							return -1;
						}

						if(remove(fileName) != 0)
						{
							cout << "error removing file" << endl;
						}
					}

					delete[] OUTBUFF;
					delete[] INBUFF;
					delete[] HASH;
					delete[] fileName;
				}
			}
			memset(&cmdBuff, '0', sizeof(cmdBuff)); 
			fsize32 = 0;
			fsize16 = 0;
		}

		if(close(new_SOCKET) < 0) //closing socket if xit was detected
		{
			cout << "close() error" << endl;
			return -1;
		}
		
	}	

	return 0;
}
