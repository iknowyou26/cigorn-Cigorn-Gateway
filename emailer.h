/* 
 * File:   emailer.h
 * Author: john
 *
 * Created on December 18, 2010, 1:30 PM
 */

#ifndef EMAILER_H
#define	EMAILER_H

#include <string>
#include <queue>
#include "TCPsocket.h"
#include <vector>

using namespace std;

struct emailcontent{
    string subjectline;  // text to put on the subject line
    string to;           // email address we send to
    string from;
    string content;
};

struct Exchange{
    char CorS;        // c=client s=server
    int code;
    string message;
    double time;
};

enum  smtp_states{
    smtp_idle,
    smtp_callserver,
    smtp_waitconnect,
    smtp_sendhelo,
    smtp_sendauth,
    smtp_sendlogin,
    smtp_sendmailto,
    smtp_send_content,
    smtp_senddot,
    smtp_done,
    smtp_finished,
    smtp_timeout,
    smtp_fail_wait,
    smtp_restartwaiting,
    smtp_restart

};

#define smtp_con_timeout  10        // The maximum number of seconds for the SMTP server to respond to a connect request
#define smtp_ex_timeout   4         // The maximum number of seconds for the SMTP server to respond to exchanges of messages
#define MAX_ELOG_LEN      40        // number of chars to store in the error log if we get an error in the exchang

class emailer{
public:
    emailer();
    emailer(const emailer& orig);
    virtual ~emailer();

    bool ProcessEmails(void);
    bool call_server(string);
    int GetResponse(void);
    int SendMessage(string);
    int LogComment(string);
    bool Enabled(void);

    string  response_code(int);
    string base64_encode(const std::string& );
    string base64_decode(std::string const&);


    string server;      // the IP address or url of the email server
    string username;    // the user name to log into the email server
    string password;    // the password to use when we log into the server
    string subject;     // text to put on the subject line of every message from this server    int    portnum;  // tthe port number to connect through
    string mailfrom;    // the email address our emails will be from
    string mailto;      // the email address to send to
    int smtptimeout;    // the number of seconds before we time out

    int CountSent;      // number of emails sent
    int CountFailed;    // number we could not sent
    int CountTimout;    // numberof times we start sending, but fail to finish.

    int portnum;        // the email port number to use

    tcpnet MySocket;  // the socket object to communicate to the smtp mail server with
    vector <Exchange> ServerResponses;   // a list of all responses from the server
    Exchange LastResponse;               // the last response we got from the server
    Exchange LastSent;                   // the last thing we sent to the server

    smtp_states mystate;

    emailcontent CurrentEmail;
    queue <emailcontent> emails;    // queue of outbound emails
    
private:
    string challenge;


};

#endif	/* EMAIL_H */

