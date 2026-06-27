/* 
 * File:   pipe.h
 * Author: john
 *
 * Created on January 18, 2011, 11:04 PM
 */

#ifndef PIPER_H
#define	PIPER_H

#include "BinaryEntry.h"
#include "CommandLine.h"

#define MAXPIPEPARMS        5
#define CigornPrompt    "Cigorn>"
#define MAXPIPETIME         15         // maximum time we will keep a pipe open (seconds)
#define MAXIDLETIME         10         // pipe gets closed if idle for more than this number of seconds

enum PipeStates{
    pipe_Idle,
    pipe_WaitCmd,
    pipe_Request,
    pipe_Piping,
    pipe_Done
};

class piper{

public:
    piper();
    piper(const piper& orig);
    virtual ~piper();

    void  DataFromTerminal(BinaryEntry&);
    bool  PipeBinary(BinaryEntry&);
    void  StoreText(BinaryEntry&, std::string );
    std::string Prompt(void);
    void NewCommand(std::string);
    bool CheckForNewInput(void);

    std::string PipeTo;     // The device designator for the thing we pipe data to
    int pipetoindex;             // The integer index to the device designator
    std::string PipeFrom;   // The terminal or other interface that originates the pipe connection
    bool PipingNow;
    double StartTime;

    std::string px[MAXPIPEPARMS];  // string paramters
    std::string PX[MAXPIPEPARMS];  // all uppercase parameters
    int pi[MAXPIPEPARMS];          // Integer values of the parameters

    PipeStates MyState;

    CommandLine CLI;

private:

};

#endif	/* PIPE_H */

