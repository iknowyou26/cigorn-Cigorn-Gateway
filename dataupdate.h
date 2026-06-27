/* 
 * File:   dataupdate.h
 * Author: john
 *
 * Created on October 21, 2010, 6:16 AM
 */

#ifndef _DATAUPDATE_H
#define	_DATAUPDATE_H

using namespace std;


struct dbdata{
    int index;
    std::string fieldname;
    std::string data;
};

typedef map<int, dbdata> updateinfo;


class dataupdate {
public:
    dataupdate(char*);
    dataupdate(std::string);
    dataupdate(const dataupdate& orig);
    virtual ~dataupdate();

    void putupdate(int, std::string, std::string);
    void putupdate(std::string, std::string, std::string);
    void putupdate(char*, char*, std::string);

    updateinfo updatelist;
    std::string tablename;

private:

};

#endif	/* _DATAUPDATE_H */

