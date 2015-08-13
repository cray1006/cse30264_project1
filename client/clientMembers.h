//Christopher Ray
//Curt Freeland
//CSE 30264
//26 September 2014

//clientMembers.h
//Header file for myftp.  

/*This contains the prototypes for various functions used in myftp.cpp.*/

#ifndef CLIENTMEMBERS_H
#define CLIENTMEMBERS_H

#include <iostream>
#include <iomanip>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <cstdio>
#include <netdb.h>
#include <cstdint>
#include <string>
#include <mhash.h>
#include <sys/time.h>

using namespace std;

int openClient(const char * name, const char * port); //function for opening client socket

int saveFile(const char * name, char * buffer, int32_t fileSize, int i); //function for saving file to memory

int prepFile(char *& buffer, const char * name, int32_t fileSize, int option); //generates hash or output buffer

int32_t getFileSize(const char * name); //returns size of file



#endif
