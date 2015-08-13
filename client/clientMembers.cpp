//Christopher Ray
//Curt Freeland
//CSE 30264
//26 September 2014

//clientMembers.cpp
//Header file for myftpd.  

/*This contains the implementations for various functions used in myftp.cpp.*/

#include "clientMembers.h"

using namespace std;

int openClient(const char * name, const char * port)
{
	int client, rv;
	struct addrinfo hints;
	struct addrinfo * servinfo;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	if((rv = getaddrinfo(name, port, &hints, &servinfo)) != 0) //generating address structure
	{
		cout << "getaddrinfo() error" << endl;
		return -1;
	}

	if((client = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) //creating socket
	{
		cout << "socket() error" << endl;
		return -1;
	}

	if(connect(client, servinfo->ai_addr, servinfo->ai_addrlen) < 0) //connecting to server
	{
		cout << "connect() error" << endl;
		return -1;
	}

	freeaddrinfo(servinfo); //freeing address structure
	return client;
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

int prepFile(char *& buffer, const char * name, int32_t fileSize, int option)
{
	int i = 0;	
	MHASH td;
	char hash[16];

	if(option == 1) //creating output buffer
	{
		buffer = new char[fileSize];
		ifstream myFile(name, ios::binary);	
		if(myFile.is_open())
		{
			myFile.seekg(0, ios::beg);
			myFile.read(buffer, fileSize); //copying in file to buffer
			myFile.close();
		}
		else
		{
			cout << "Error opening file" << endl; //catches if there was an error opening the file
			return -1;
		}
	}
	else //generating MD5 hash
	{
		td = mhash_init(MHASH_MD5);

		buffer = new char[fileSize];
		ifstream myFile(name, ios::binary);	//opening file
		if(myFile.is_open())
		{
			myFile.seekg(0, ios::beg);
			myFile.read(buffer, fileSize); //copying contents into buffer
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
			buffer[i] = hash[i]; //copying contents of hash into buffer
		}
		buffer[i] = '\0';
	}

	return 1;
}

int32_t getFileSize(const char * name)
{
	int32_t fSize = 0;
	char c;

	ifstream myFile(name, ios::binary); //opening file
	if(myFile.is_open())
	{
		myFile.seekg(0, myFile.end);
		fSize = myFile.tellg(); 
		myFile.close();
		return fSize; //returning current position (should be the end of the file, i.e. file size)
	}
	else
	{
		return -1; //indicates file failed to open
	}
}


