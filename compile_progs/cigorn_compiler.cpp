/**
 * @file cigorn_compiler.cpp
 * @author Rodrigo Bondoc
 * @date August 2, 2019, 4:30PM
 * @section desc Description
 * This program compiles all the necessary files required for Cigorn.
 * @section revs Revisions
 * This section will keep track of revisions made to this file
 * @subsection v10 Version 1.0
 * [08/02/19] Initial release of this file
 * @subsection v11 Version 1.1
 * [08/15/19] Changed output executable name from "cigorn" to "cigorngateway"
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>	// Used for system() calls, exit() NULL, and EXIT_FAILURE
#include <stdio.h>	// Used for printf()
#include <unistd.h>	// Used for chdir()

using std::cout;
using std::endl;

void createFileList(std::string fileName) {

	std::string command;

	// ***** Check if file exists *****
	std::fstream fileStream;
	fileStream.open(fileName.c_str());
	if(!fileStream.fail()) {		// True if file already exists
		fileStream.close();
		command = "rm " + fileName;
		system(command.c_str());
		cout << "Old " << fileName << " deleted." << endl;
	}					// Now, file does not exist anymore

	std::string ls = "ls -R ";					// Recursively list all files
	std::string fileRedirect = " > " + fileName;

	command = ls + fileRedirect;

	system(command.c_str());
}

std::vector<std::string> getListOfCFiles(std::string fileName) {

	std::vector<std::string> fileList;

	std::ifstream in(fileName.c_str());

	if(!in) {
		cout << "Error opening: " << fileName << endl;
		exit(EXIT_FAILURE);
	}

	std::string str;
	while(std::getline(in, str)) {
		if(str.find(".") != -1) {		// File could be a valid file name
			if(str.find("/") == -1) {	// File is not a directory
				if(str.find(".cpp") != -1) {
					fileList.push_back(str);
				}
				else if(str.find(".c") != -1) {		// Could be a valid c file
					if( (str.find(".crt") == -1) && (str.find(".data") == -1) && (str.find(".css") == -1) ) {
						fileList.push_back(str);	// Filtered invalid file extensions
					}
				}
			}
		}
	}

	in.close();

	return fileList;
}

std::vector<std::string> getFilePaths(std::vector<std::string>& fileList) {
	std::string command, str;

	// ***** Check if file exists *****
	std::fstream fileStream;
	fileStream.open("fileLocations.txt");
	if(!fileStream.fail()) {		// True if the file already exists
		fileStream.close();
		command = "rm fileLocations.txt";
		system(command.c_str());
		cout << "Old fileLocations.txt deleted" << endl;
	}

	std::vector<std::string> filePaths;
	std::vector<std::string>::iterator it;

	for(it = fileList.begin(); it != fileList.end(); it++) {
		str = *it;
		command = "find ./ -name " + str + " >> fileLocations.txt";		// Append locations to file
		system(command.c_str());
	}

	std::ifstream in("fileLocations.txt");
	if(!in) {
		cout << "Error in opening: fileLocations.txt" << endl;
		exit(EXIT_FAILURE);
	}

	while(std::getline(in, str)) {			// Extract file locations
		filePaths.push_back(str);
	}

	return filePaths;
}

std::vector<std::string> getListOfObjectFiles(std::string fileName) {

	std::string command, str;

	// ***** Check if file exists *****
	std::fstream fileStream;
	fileStream.open(fileName.c_str());
	if(!fileStream.fail()) {		// True if the file already exists
		fileStream.close();
		command = "rm " + fileName;
		system(command.c_str());
		cout << "Old " << fileName << " deleted" << endl;
	}

	command = "ls > " + fileName;
	system(command.c_str());

	std::ifstream in(fileName.c_str());
	if(!in) {
		cout << "Error in opening: " << fileName << endl;
		exit(EXIT_FAILURE);
	}


	std::vector<std::string> objectList;
	while(std::getline(in, str)) {
		if(str.substr(str.size()-2) == ".o") {
			objectList.push_back(str);
		}
	}

	return objectList;
}


int main() {

	// ***** Change working directory to Cigornv4.0.1-minimal *****
	std::string dir = "/home/engineer/working/Cigornv4.0.1-minimal";
	chdir(dir.c_str());


	std::string fileName = "files.txt";

	createFileList(fileName);

	std::vector<std::string> fileList = getListOfCFiles(fileName);	// Returns list of .c and .cpp files
	std::vector<std::string> filePaths = getFilePaths(fileList);	// Return the full path of the .c and .cpp files
	std::vector<std::string>::iterator it;				// vector<string> iterator object


	// ***** BEGIN SOURCE CODE COMPILATION SECTION *****
	std::string path, command, gcc, gpp;
	for(it = filePaths.begin(); it != filePaths.end(); it++) {
		path = *it;

		gpp = "g++ -c " + path;
		gcc = "gcc -c " + path;

		if(path.find(".cpp") != -1) {	// File is a C++ file, use g++
			command = gpp;
		}
		else {				// File is a C file, use gcc
			command = gcc;
		}

		cout << "------" << endl;
//		cout << "Compiling: " << path.substr(path.find_last_of("/")+1) << endl;		// Only has file name

		printf("%c[%dmCompiling: %s\n", 0x1B, 96, path.c_str());		// Includes full path (colored text)
		printf("%c[%dm%s\n", 0x1B, 37, command.c_str());
//		cout << "Compiling: " << path << endl;					// Includes full path (white text)
//		cout << command << endl;

		system(command.c_str());		// Execute compile command
	}
	// ***** END SOURCE CODE COMPLICATION SECTION *****

	// Return list of .o files to link in the current dir
//	std::vector<std::string> objectList = getListOfObjectFiles("objectFiles.txt");		// Easier to use *.o when linking


	if(system("rm cigorn_compiler.o") != -1) {			// Remove object file before linking CIGORN files
		cout << endl << "Removed cigorn_compiler.o" << endl;
	}

	if(system("rm cigorn_quick_compiler.o") != -1) {			// Remove object file before linking CIGORN files
		cout << endl << "Removed cigorn_quick_compiler.o" << endl;
	}

	// ***** BEGIN OBJECT FILE LINKING SECTION *****

	/*
	 * Need to link libraries:
	 * 1. crypt.h		Used for cypher.h -- An encryption program		-lcrypt
	 * 2. libpq-fe.h	Used for accessing postgresql accessing functions	-I/usr/include... and -L/usr/lib/...
	 * 3. pthread		Used for POSIX threads					-pthread
	 */
	std::string linkCommand = "g++ -lcrypt -I/usr/include/postgresql -L/usr/lib/postgresql/9.6/lib -lpq -pthread -o cigorn *.o";

//	for(it = objectList.begin(); it != objectList.end(); it++) {	// Form long command string that lists all .o files
//		str = *it;
//		linkCommand.append(str + " ");
//	}

	cout << "\n" << "---------- BEGIN LINKING ----------" << endl;
	int success = system(linkCommand.c_str());
	// ***** END OBJECT FILE LINKING SECTION *****'

	if(success != -1) {
		printf("%c[%dmLinking successful.\n", 0x1B, 95);		// Colored text
//		cout << "Linking successful." << endl;				// White text
	}

//	system("rm fileLocations.txt");
//	system("rm files.txt");
//	system("rm objectFiles.txt");

	return 0;
}
