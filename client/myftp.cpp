//Christopher Ray
//Curt Freeland
//CSE 30264
//26 September 2014

//myftp.cpp
//Driver program for myftp.  

/*This is the client program for a simple file transfer application.*/

#include "clientMembers.h"

using namespace std;

int main(int argc, char * argv[])
{
	//declaring variables	
	const char * PORT, * ADDRESS, * cmd, * fileName;	//port, address, the command string, and the fileNames
	string cmdString, fileString;	//command string (will be translated to c-string), file string(will be translated to c-string)
	char * OUTBUFF, * INBUFF, * HASH; //output buffer, input buffer, hash buffer
	string tempPort = "9499"; //default port
	int SOCKET, new_SOCKET, cmdBytes, outBytes, inBytes, bytesRec, i; //various integers
	struct timeval time1, time2; //used to calculate elapsed time
	double totalTime, rate; //used to calculate elapsed time
	int32_t fsize32;	//32bit integer (used for various purposes, mostly file size
	int16_t fsize16;	//16 bit integer (used for various purposes, mostly size of filename
	FILE * myFile;		//used for dir and file io
	
	if(argc < 2) //user must provide at least the address
	{
		cout << "too few arguments" << endl;
		return -1;
	}

	ADDRESS = argv[1];
	if(argc == 2) //setting PORT to either the default port or the input port
	{
		PORT = tempPort.c_str();
	}
	else
	{
		PORT = argv[2];
	}

	if((SOCKET = openClient(ADDRESS, PORT)) < 0) //creating socket
	{
		cout << "socket() error" << endl;
		return -1;
	}

	while(1)
	{
		cout << "Command(get, put, dir, xit):  "; //user enters command
		cin >> cmdString;
		cmd = cmdString.c_str();

		if((strcmp(cmd, "XIT") == 0) || (strcmp(cmd, "xit") == 0)) //xit
		{
			if((outBytes = send(SOCKET, cmd, strlen(cmd), 0)) < 0)	 //sending command to the server
			{
				cout << "send() error, xit cmd" << endl;
				return -1;
			}		
		
			if(close(SOCKET) < 0) //closing the socket
			{
				cout << "close() error" << endl;
				return -1;
			}
			else
			{
				cout << "session ending" << endl; //telling user that the session is over 				
				return 0;
			}
		}
		else if((strcmp(cmd, "DIR") == 0) || (strcmp(cmd, "dir") == 0)) //dir
		{
			if((outBytes = send(SOCKET, cmd, strlen(cmd), 0)) < 0)	
			{
				cout << "send() error, dir cmd" << endl;
				return -1;
			}
			
			if((inBytes = recv(SOCKET, &fsize32, sizeof(fsize32), 0)) < 0) //receiving size of dir listing
			{
				cout << "recv() error, dir fsize32" << endl;
				return -1;
			}

			fsize32 = ntohl(fsize32);
			INBUFF = new char[fsize32];
			bytesRec = 0;
			while(bytesRec < fsize32) //receiving directory listing
			{
				if((inBytes = recv(SOCKET, INBUFF, fsize32, 0)) < 0)
				{
					cout << "recv() error dir INBUFF" << endl;
					return -1;
				}
				bytesRec += inBytes;
			}
			INBUFF[fsize32] = '\0';
			cout << INBUFF << endl;	 //displaying directory listing
			delete[] INBUFF;
		}
		else if((strcmp(cmd, "GET") == 0) || (strcmp(cmd, "get") == 0)) //get
		{
			if((outBytes = send(SOCKET, cmd, strlen(cmd), 0)) < 0)	
			{
				cout << "send() error, get cmd" << endl;
				return -1;
			}

			cout << "Enter filename:  "; //prompting user to enter a filename
			cin >> fileString;
			fileName = fileString.c_str();
			fsize16 = htons(strlen(fileName));

			if((outBytes = send(SOCKET, &fsize16, sizeof(fsize16), 0)) < 0)	 //sending size of filename to server
			{
				cout << "send() error, get fsize" << endl;
				return -1;
			}

			if((outBytes = send(SOCKET, fileName, ntohs(fsize16), 0)) < 0) //sending filename to server	
			{
				cout << "send() error, get fsize16" << endl;
				return -1;
			}

			if((inBytes = recv(SOCKET, &fsize32, sizeof(fsize32), 0)) < 0) //receiving size of file
			{
				cout << "recv() error, get fsize32" << endl;
				return -1;
			}

			fsize32 = ntohl(fsize32);
			if(fsize32 < 0)		//file does not exist, prompt for new input
			{
				cout << fileName << " does not exist." << endl;
			}
			else
			{
				gettimeofday(&time1, 0); //getting time at start of transfer				
				HASH = new char[16];
				bytesRec = 0;
				while(bytesRec < 16)
				{
					if((inBytes = recv(SOCKET, HASH, 16, 0)) < 0) //receiving MD5 hash
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
					if((inBytes = recv(SOCKET, INBUFF, fsize32, 0)) < 0) //receiving actual file buffer
					{
						cout << "recv() error, get INBUFF" << endl;
						return -1;
					}
					bytesRec += inBytes;
					saveFile(fileName, INBUFF, inBytes, i);
					i++;
				}

				if(prepFile(OUTBUFF, fileName, fsize32, 0) < 0) //generating client copy of hash
				{
					cout << "error generating hash" << endl;
					return -1;
				}

				if(strcmp(OUTBUFF, HASH) == 0) //comparing received hash and client hash
				{
					cout << "File transfer successful." << endl;
				}
				else
				{
					cout << "File transfer unsuccessful." << endl;
					if(remove(fileName) != 0) //removing corrupted file from directory
					{
						cout << "error removing file" << endl;
					}
				}
		
				gettimeofday(&time2, 0); //determining time at end of transfer
				//displaying throughput
				totalTime = ((time2.tv_sec - time1.tv_sec) * 1000000) + (time2.tv_usec - time1.tv_usec);
				cout << fsize32 << " bytes transferred in " << setprecision(5) << (totalTime / 1000000) << " seconds:  ";
				rate = (fsize32 / 1000000) / (totalTime / 1000000);
				cout << setprecision(5) << rate << " MB/s" << endl;
				cout << fileName << " MD5sum:  " << HASH << endl; 

				delete[] OUTBUFF;
				delete[] INBUFF;
				delete[] HASH;
				
			}
		}
		else if((strcmp(cmd, "PUT") == 0) || (strcmp(cmd, "put") == 0)) //put
		{
			if((outBytes = send(SOCKET, cmd, strlen(cmd), 0)) < 0)	
			{
				cout << "send() error, put cmd" << endl;
				return -1;
			}

			cout << "Enter filename:  "; //prompting user for filename
			cin >> fileString;
			fileName = fileString.c_str();
			fsize16 = htons(strlen(fileName));

			if((outBytes = send(SOCKET, &fsize16, sizeof(fsize16), 0)) < 0)	//sending size of filename
			{
				cout << "send() error, put fsize16" << endl;
				return -1;
			}

			if((outBytes = send(SOCKET, fileName, ntohs(fsize16), 0)) < 0)	//sending filename
			{
				cout << "send() error, put filename" << endl;
				return -1;
			}

			if((inBytes = recv(SOCKET, &fsize16, sizeof(fsize16), 0)) < 0) //server acknowledging that it's ready
			{
				cout << "recv() error, put fsize16" << endl;
				return -1;
			}
			fsize16 = ntohs(fsize16);
			if(fsize16 < 0)
			{
				cout << "server not ready" << endl; //this means there was an error opening the file to write
			}
			else
			{
				fsize32 = getFileSize(fileName);
				fsize32 = htonl(fsize32);
				if((outBytes = send(SOCKET, &fsize32, sizeof(fsize32), 0)) < 0) //sending size of file
				{
					cout << "send() error, get fsize32" << endl;
					return -1;
				}
			
				fsize32 = ntohl(fsize32);
				if(fsize32 > 0)
				{
					if(prepFile(HASH, fileName, fsize32, 0) < 0) //generating hash
					{
						cout << "Error generating hash" << endl;
						return -1;
					}
					if((outBytes = send(SOCKET, HASH, 16, 0)) < 0) //sending hash 
					{
						cout << "send() error, put HASH" << endl;
						return -1;
					}

					if(prepFile(OUTBUFF, fileName, fsize32, 1) < 0) //preparing file buffer
					{
						cout << "Error generating file buffer" << endl;
						return -1;
					}
					if((outBytes = send(SOCKET, OUTBUFF, fsize32, 0)) < 0) //sending file buffer
					{
						cout << "send() error, put OUTBUFF" << endl;
						return -1;
					}

					if((inBytes = recv(SOCKET, &fsize16, sizeof(fsize16), 0)) < 0) 
					{//determining if transfer is successful
						cout << "recv() error, put fsize16" << endl;
						return -1;
					}

					if((inBytes = recv(SOCKET, &totalTime, sizeof(totalTime), 0)) < 0)
					{//receiving total time for transfer
						cout << "recv() error, put fsize32" << endl;
						return -1;
					}

					fsize16 = ntohs(fsize16);
					totalTime = ntohl(totalTime);
					if(fsize16 < 0)
					{
						cout << "file transfer unsuccessful" << endl;
					}
					else //displaying throughput
					{
						cout << "file transfer successful" << endl;
						cout << fsize32 << " bytes transferred in " << setprecision(5) << (totalTime / 1000000) << " seconds:  ";
						rate = (fsize32 / 1000000) / (totalTime / 1000000);
						cout << setprecision(5) << rate << " MB/s" << endl;
						cout << fileName << " MD5sum:  " << HASH << endl; 

					}
				
					delete [] OUTBUFF;
					delete[] HASH;
				}
				else //just making sure user doesn't try to send a nonexistant file
				{
					cout << "file does not exist in this directory" << endl;
					return -1;
				}
			}
		}
		else
		{
			cout << "invalid command" << endl;
		}

		memset(&cmd, 0, sizeof(cmd));
		fsize32 = 0;
		fsize16 = 0;
	}

	return 0;
}
