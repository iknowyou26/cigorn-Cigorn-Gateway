/**
 * @file cigorn_quick_compiler.cpp
 * @author Rodrigo Bondoc
 * @date August 2, 2019, 4:30PM
 * @section desc Description
 * This program is designed for making debugging easier on the command line. It will take a single source file that has been modified
 * and will compile it. Afterwords, it will link the newly created .o object file and link it with the others, yielding an updated
 * executable binary file. 
 * @section revs Revisions
 * This section will keep track of revisions made to this file
 * @subsection v10 Version 1.0
 * [08/02/19] Initial release of this file
 * @subsection v11 Version 1.1
 * [08/15/19] Changed output executable name from "cigorn" to "cigorngateway"
 */

#include <iostream>
#include <string>
#include <stdlib.h>		// Used for system() calls, exit(), and EXIT_FAILURE
#include <unistd.h>		// Used for chdir()

using std::cout;
using std::endl;
using std::string;

int main(int argc, char* argv[]) {

	if(argc != 2) {
		if(argc < 2) {
			cout << "Not enough arguments. ";
		}
		else {
			cout << "Too many arguments. ";
		}
		cout << "Usage: ./cqc modified_file.cpp" << endl;
		exit(EXIT_FAILURE);
	}

	// ***** Change working directory to Cigornv4.0.1-minimal *****
	std::string dir = "/home/engineer/working/Cigornv4.0.1-minimal";
	chdir(dir.c_str());

	string command, file = dir + "/" + argv[1];

	if(file.find(".c") != -1) {
		if(file.substr(file.size() - 2) != ".c") {	// File is a .cpp file
			command = "g++ -c " + file;
		}
		else {						// File is a .h file
			command = "gcc -c " + file;
		}
	}
	else {
		cout << "Invalid file format detected. Only .c and .cpp files may be passed into the compiler." << endl;
		exit(EXIT_FAILURE);
	}

	cout << command << endl;
	if(system(command.c_str()) != -1) {
		cout << "Compilation successful." << endl;
	}

	string linkCommand = "g++ -lcrypt -I/usr/include/postgresql -L/usr/lib/postgresql/9.6/lib -lpq -pthread -o cigorn *.o";

	if(system("rm cigorn_quick_compiler.o") != -1) {
		cout << "cigorn_quick_compiler.o was removed before linking." << endl;
	}
	if(system("rm cigorn_compiler.o") != -1) {
		cout << "cigorn_compiler.o was removed before linking." << endl;
	}

	if(system(linkCommand.c_str()) != -1) {
		cout << "Linking successful." << endl;
	}

	return 0;
}
