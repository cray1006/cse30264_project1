//Christopher Ray
//Curt Freeland
//CSE 30264
//26 September 2014

//serverMembers.cpp
//Header file for myftpd.  

/*This contains the implementations for various functions used in myftpd.cpp.*/

#include "serverMembers.h"
#define LISTENQ 1024

using namespace std;

int heyListen (const char * port)
{
	int listening, optval, rv;
	struct addrinfo hints; 			
	struct addrinfo * servinfo;	

	memset(&hints, 0, sizeof(hints)); 	
	hints.ai_family = AF_UNSPEC; 		
	hints.ai_socktype = SOCK_STREAM; 	
	hints.ai_flags = AI_PASSIVE;

	if((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) //generating server's address structure
	{
		cout << gai_strerror(rv) << endl; 
		return (-1);
	}

	if((listening = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) //creating socket
	{
		cout << "socket() error" << endl; 	
		return (-1);
	}

	optval = 1;
	if(setsockopt(listening, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) //ensuring socket can be reused after crash
	{
		cout << "setsockopt() error" << endl;		
		return (-1);
	} 
	
	if(bind(listening, servinfo->ai_addr, servinfo->ai_addrlen) < 0) //binding socket
	{
		cout << "bind() error" << endl;
		return (-1);
	} 

	if(listen(listening, LISTENQ) < 0) //socket now listens on the indicated port
	{
		cout << "listen() error" << endl;
		return (-1);
	}		
	
	freeaddrinfo(servinfo); //freeing the server's address structure
	return listening;
}

int32_t getDirSize(FILE * myFile)
{
	int32_t i = 0;
	char c;

	myFile = popen("ls", "r"); //opening pipe
	if(myFile == NULL)
	{
		cout << "popen() error" << endl;
		return (-1);
	}

	c = fgetc(myFile); //closing pipe
	while(c != EOF) //looping through pipe and incrementing i for every character found
	{
		i++;
		c = fgetc(myFile);
	}
	pclose(myFile);
	return i; //returning the number of characters found (i.e. size of directory)
}

int getDirListing(int32_t fileSize, char *& buffer, FILE * myFile)
{
	int32_t i = 0;
	char c;

	buffer = new char[fileSize];

	myFile = popen("ls", "r"); //opening pipe
	if(myFile == NULL)
	{
		cout << "popen() error" << endl;
		return (-1);
	}

	i = 0;
	c = fgetc(myFile);
	while(c != EOF) //looping through pipe and updating buffer with each iteration
	{
		buffer[i] = c;
		i++;
		c = fgetc(myFile);
	}
	buffer[i] = '\0'; //buffer is now a string containing directory listing
	pclose(myFile); //closing pipe

	return 1;
}

int32_t getFileSize(char * name)
{
	int32_t fSize = 0;
	char c;

	ifstream myFile(name, ios::binary); //opening file to be read
	if(myFile.is_open())
	{
		myFile.seekg(0, myFile.end); //setting current position to the end of the file
		fSize = myFile.tellg();
		myFile.close();
		return fSize; //returning the current position, which should equal the file size
	}
	else
	{
		return -1; //returned if there is an error opening the file
	}
}

int prepFile(char *& buffer, const char * name, int32_t fileSize, int option)
{
	int i = 0;	
	MHASH td;
	char hash[16];

	if(option == 1) //generate output buffer
	{
		buffer = new char[fileSize];
		ifstream myFile(name, ios::binary);	//opening file
		if(myFile.is_open())
		{
			myFile.seekg(0, ios::beg);
			myFile.read(buffer, fileSize); //reading contents into output buffer
			myFile.close(); //closing file
		}
		else
		{
			cout << "Error opening file" << endl;
			return -1;
		}
	}
	else //generating MD5 hash
	{
		td = mhash_init(MHASH_MD5);

		buffer = new char[fileSize];
		ifstream myFile(name, ios::binary);	
		if(myFile.is_open()) //opening file
		{
			myFile.seekg(0, ios::beg);
			myFile.read(buffer, fileSize); //reading file contents into output buffer
			myFile.close();
		}
		else
		{
			cout << "Error opening file" << endl;
			return -1;
		}

		mhash(td, buffer, fileSize);
		mhash_deinit(td, hash); //hash now contains the file's MD5 hash
		delete [] buffer;
		buffer = new char[16];

		for(i = 0; i < mhash_get_block_size(MHASH_MD5); i++)
		{
			buffer[i] = hash[i]; //reading contents of hash into buffer
		}
		buffer[i] = '\0';
	}

	return 1;
}

int16_t readyCheck(const char * name)
{
	ofstream myFile(name, ios::binary); //opening file to write
	if(myFile.is_open())
	{
		myFile.close();
		return 1; //indicates that we can write to the file
	}
	else
	{
		if(remove(name) != 0) //attempt to remove duplicate filenames (will destroy symbolic links)
		{
			return -1; //indicates that file could not be removed
		}
		return 1;
	}
}

int saveFile(const char * name, char * buffer, int32_t fileSize, int i)
{
	if(i == 0) //starting at beginning of file (overwriting currently existing files)
	{
     		ofstream myFile(name, ios::binary); //opening file
		if(myFile.is_open())
		{
			myFile.seekp(0, ios::beg);		
			myFile.write(buffer, fileSize); //copying contents into buffer
			myFile.close();
		}
		else
		{
			cout << "Unable to open " << name << endl; //checking for there was an error opening file
			return -1;
		}
	}
	else //appending to current write file
	{
		ofstream myFile(name, ios::app | ios::binary); 
		if(myFile.is_open())
		{
		
			myFile.seekp(0, ios::cur);
			myFile.write(buffer, fileSize);
			myFile.close();
		}
		else
		{
			cout << "Unable to open " << name << endl;
			return -1;
		}
	}
}
