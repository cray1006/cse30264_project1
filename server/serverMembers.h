//Christopher Ray
//Curt Freeland
//CSE 30264
//26 September 2014

//serverMembers.h
//Header file for myftpd.  

/*This contains the prototypes for various functions used in myftpd.cpp.*/

#ifndef SERVERMEMBERS_H
#define SERVERMEMBERS_H

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

int heyListen(const char * port); //function for creating and binding a socket

int getDirSize(FILE * myFile); //function for getting size of directory listing

int getDirListing(int32_t fileSize, char *& buffer, FILE * myFile); //function for putting directory listing into output buffer

int32_t getFileSize(char * name); //returns the size of the indicated file

int prepFile(char *& buffer, const char * name, int32_t fileSize, int option); //generates MD5 hash or output buffer

int16_t readyCheck(const char * name); //checking if file can be written to

int saveFile(const char * name, char * buffer, int32_t fileSize, int i); //saving file to memory

#endif
