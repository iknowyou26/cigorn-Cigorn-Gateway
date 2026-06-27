/* 
 * File:   xmlFunctions.h
 * Author: john
 *
 * Created on October 6, 2010, 8:15 PM
 */

#ifndef _XMLFUNCTIONS_H
#define	_XMLFUNCTIONS_H

#include "xmlDefs.h"
#include "tinyxml/tinystr.h"   //http://www.grinninglizard.com/tinyxml/
#include "tinyxml/tinyxml.h"

using namespace std;

string MakeSiteIdentifierXML(void);
string MakeWDtableUpdateXML(int , int );
string findroot(char *, int, int);
string closetag(string );
string maketag(string );
void ProcessInterSiteXML(BinaryEntry&, CigornSite&);
void ProcessXMLcommand(BinaryEntry&, CigornIntersite&);
string MakeSecureRequestXML(string, string);
string MakePrimaryCommand(string, string);
void GotInterSiteXML(TiXmlDocument&, CigornSite&);
void GotSecureQuery(TiXmlDocument&, CigornIntersite&);
void GotSecureReply(TiXmlDocument& , CigornIntersite&);
void GotCommand(TiXmlDocument&, CigornIntersite&);

std::string GetCigornMessageType(BinaryEntry&);
void ProcessSecureXML(BinaryEntry& msg, CigornIntersite&);
string MakeSecureResponseXML(XMLnodeList, string);
string ElementTag(TiXmlElement*);

#endif	/* _XMLFUNCTIONS_H */

