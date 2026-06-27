
// **************************************************************
//  The function calls to TinyXML have been known to cause the 
//  program to crash.  We need to wrap them in try/catch and
//  add more bounds checking.
// **************************************************************

#include <iostream>
#include <string>

#include "DeviceList.h"
#include <string.h>   // Required by strcpy()
#include <stdlib.h>   // Required by malloc()
#include <stdio.h>
#include <sys/socket.h>

#include <limits>

#include "Cigorn.h"     // Our application-specific constants
#include "GlobalVar.h"
#include "ourstructures.h"
#include "tinyxml/tinystr.h"   //http://www.grinninglizard.com/tinyxml/
#include "tinyxml/tinyxml.h"
#include "xmlFunctions.h"
#include "xmlDefs.h"
#include "CommThread.h"
#include "dataupdate.h"

// Local prototypes

using namespace std;

// Prototype the local functions
string Attribute(TiXmlElement* , string);
string Attribute(TiXmlElement* , int );
string AttributeName(TiXmlElement* , int );
string ElementValue(TiXmlElement*);

//  Make a WD table update message updating WD between A and B
string MakeWDtableUpdateXML(int A, int B){

    TiXmlDocument doc;
    TiXmlComment *comment;
    TiXmlElement *msg;
    TiXmlElement *fieldname;
    TiXmlElement *fielddata;
    string value;

    string s;
    int i;
    char c[255];


    TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );

    doc.LinkEndChild( decl );

    // Create the root element
    TiXmlElement *root = new TiXmlElement(xmlCIGORN);
    doc.LinkEndChild(root);

    // Add a comment
    comment = new TiXmlComment();
    s = " WD Table information from " + Application + " ";
    comment->SetValue(s.c_str());
    root->LinkEndChild(comment);

    // First put in a time stamp to show when this document is created
    TiXmlElement * cxn = new TiXmlElement(xmlTimeStamp);
    root->LinkEndChild( cxn );
    cxn->SetAttribute("time", LocalTime().c_str());
    cxn->SetAttribute("date", LocalDate().c_str());

    // Store the software version just in case some day we care to know
    cxn = new TiXmlElement(xmlSoftVersion);
    root->LinkEndChild( cxn );
    cxn->SetAttribute("major", REV_MAJOR);
    cxn->SetAttribute("minor", REV_MINOR);

    // Build the SiteProperties Elements
    TiXmlElement *msgs = new TiXmlElement(xmlSiteProperties);
    root->LinkEndChild(msgs);

        // Put the name of the site in the message
        string key = xmlSiteName;
        value= Me.MyName;
        msg = new TiXmlElement(key.c_str());
        msg->LinkEndChild(new TiXmlText(value.c_str()));
        msgs->LinkEndChild(msg);

        // Tell them if we are a master site
        key = xmlIsMaster;
        value = BoolToString(Me.IsChief);
        msg = new TiXmlElement(key.c_str());
        msg->LinkEndChild(new TiXmlText(value.c_str()));
        msgs->LinkEndChild(msg);

    TiXmlElement *dtable = new TiXmlElement(xmlTableUpdate);
    root->LinkEndChild(dtable);

    dtable->SetAttribute(xmlTableNameAttrib, dtWD->tablename.c_str());

    // tell them the ID range that this update covers
    cxn = new TiXmlElement(xmlIDrange);
    dtable->LinkEndChild( cxn );
    cxn->SetAttribute(xmlFirstID,intToString(A).c_str());
    cxn->SetAttribute(xmlLastID, intToString(B).c_str());


    // loop through and make an element for each WD in our table
    for (dtWD->dit = dtWD->rows.begin(); dtWD->dit != dtWD->rows.end(); dtWD->dit++){
       i = dtWD->GetIntItem(dtWD->dit->first, fld_ID);
       if (( i>= A) && (i <= B)){
           // Build the WD column Elements
            TiXmlElement *datarow = new TiXmlElement(xmlRowUpdate);
            to_cstring (c, i, 12);   // convert the index to a c string
            datarow->SetAttribute(xmlIndexAttrib, c);
            dtable->LinkEndChild(datarow);

                // list the columns that we will send out data for updates of off-site tables in other gateways.
                TiXmlElement *tablecol= new TiXmlElement(xmlTableColumn);  // this is column data for a row update in a table
                value= dtWD->GetItem(dtWD->dit->first, fld_enabled);
                tablecol->SetAttribute(fld_enabled,value.c_str());
                datarow->LinkEndChild(tablecol);

                tablecol = new TiXmlElement(xmlTableColumn); // this is column data for a row update in a table
                value= dtWD->GetItem(dtWD->dit->first, fld_system );
                tablecol->SetAttribute(fld_system ,value.c_str());
                datarow->LinkEndChild(tablecol);
       }


    }


    doc.SaveFile("WDupdate.xml");
    TiXmlPrinter printer;
    printer.SetIndent( "    " );

    doc.Accept( &printer );
    string xmltext = printer.CStr();

    return xmltext;
}

string MakeSiteIdentifierXML(void){

    TiXmlDocument doc;
    TiXmlElement *msg;
    TiXmlComment *comment;
    string s;
    TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );

    doc.LinkEndChild( decl );

    // Create the root element
    TiXmlElement *root = new TiXmlElement(xmlCIGORN);
    doc.LinkEndChild( root );

    // Add a comment
    comment = new TiXmlComment();
    s = " Information from " + Application + " ";
    comment->SetValue(s.c_str());
    root->LinkEndChild( comment );

    // First put in a time stamp to show when this document is created
    TiXmlElement * cxn = new TiXmlElement(xmlTimeStamp);
    root->LinkEndChild( cxn );
    cxn->SetAttribute("time", LocalTime().c_str());
    cxn->SetAttribute("date", LocalDate().c_str());

    // Store the software version just in case some day we care to know
    cxn = new TiXmlElement(xmlSoftVersion);
    root->LinkEndChild( cxn );
    cxn->SetAttribute("major", REV_MAJOR);
    cxn->SetAttribute("minor", REV_MINOR);
    cxn->SetAttribute("build", REV_BUILD);


    // Build the SiteProperties Elements
    TiXmlElement *msgs = new TiXmlElement(xmlSiteProperties);
    root->LinkEndChild(msgs);

    // Put the name of the site in the message
    string key = xmlSiteName;
    string value= Me.MyName;
    msg = new TiXmlElement(key.c_str());
    msg->LinkEndChild(new TiXmlText(value.c_str()));
    msgs->LinkEndChild(msg);

    // Tell them if we are a master site
    key = xmlIsMaster;
    value = BoolToString(Me.IsChief);
    msg = new TiXmlElement(key.c_str());
    msg->LinkEndChild(new TiXmlText(value.c_str()));
    msgs->LinkEndChild(msg);

    // Tell them our role in the system (primary or backup)
    key = xmlRole;
    if (Me.gaterole == Primary)
        value = xmlRolePrimary;
    else
        value = xmlRoleBackup;
    msg = new TiXmlElement(key.c_str());
    msg->LinkEndChild(new TiXmlText(value.c_str()));
    msgs->LinkEndChild(msg);

    // Tell them if we modified our DB or recently re-read it
    key = xmlDBmodified;
    value = intToString(Me.DBmodifyflag);
    msg = new TiXmlElement(key.c_str());
    msg->LinkEndChild(new TiXmlText(value.c_str()));
    msgs->LinkEndChild(msg);

    // Tell them last time our DB tables were changed
    key = xmlIsActive;
    value = BoolToString(Me.IsActive);
    msg = new TiXmlElement(key.c_str());
    msg->LinkEndChild(new TiXmlText(value.c_str()));
    msgs->LinkEndChild(msg);


    // Send some statistics to let the hot-standby site know we are OK
    cxn = new TiXmlElement(xmlSiteStatistics);
    root->LinkEndChild( cxn );
    cxn->SetAttribute(xmlSocketCount , intToString(OurDevices.getConnectedSocketCount()).c_str() );
    cxn->SetAttribute(xmlMsgInCount , intToString(OurDevices.getMessagesIn()).c_str() );
    cxn->SetAttribute(xmlMsgOutCount , intToString(OurDevices.getMessagesOut()).c_str() );

    // Done building XML structure
    doc.SaveFile("MySite.xml");
    TiXmlPrinter printer;
    printer.SetIndent( "    " );

    doc.Accept( &printer );
    string xmltext = printer.CStr();

    return xmltext;
}


string MakePrimaryCommand(string CommandText, string Destination){

    TiXmlDocument doc;
    TiXmlElement *msg;
    TiXmlComment *comment;
    string s;
    TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );

    doc.LinkEndChild( decl );

    // Create the root element
    TiXmlElement *root = new TiXmlElement(xmlCOMMAND);
    doc.LinkEndChild( root );

    // Add a comment
    comment = new TiXmlComment();
    s = " Command to Primary";
    comment->SetValue(s.c_str());
    root->LinkEndChild( comment );

    // First put in a time stamp to show when this document is created
    TiXmlElement * cxn = new TiXmlElement(xmlTimeStamp);
    root->LinkEndChild( cxn );
    cxn->SetAttribute("time", LocalTime().c_str());
    cxn->SetAttribute("date", LocalDate().c_str());

    // Say who this message is for
    cxn = new TiXmlElement(xmlDestination);
    root->LinkEndChild( cxn );
    cxn->SetAttribute("name", Destination.c_str());

    // Store the software version just in case some day we care to know
    cxn = new TiXmlElement(xmlSoftVersion);
    root->LinkEndChild( cxn );
    cxn->SetAttribute("major", REV_MAJOR);
    cxn->SetAttribute("minor", REV_MINOR);
    cxn->SetAttribute("build", REV_BUILD);

    // Build the SiteProperties Elements
    TiXmlElement *msgs = new TiXmlElement(xmlSiteProperties);
    root->LinkEndChild(msgs);

    // Put the name of the site in the message
    string key = xmlSiteName;
    string value= Me.MyName;
    msg = new TiXmlElement(key.c_str());
    msg->LinkEndChild(new TiXmlText(value.c_str()));
    msgs->LinkEndChild(msg);

    // Tell them our role in the system (primary or backup)
    key = xmlRole;
    if (Me.gaterole == Primary)
        value = xmlRolePrimary;
    else
        value = xmlRoleBackup;
    msg = new TiXmlElement(key.c_str());
    msg->LinkEndChild(new TiXmlText(value.c_str()));
    msgs->LinkEndChild(msg);



    // Build the list of query items
    TiXmlElement *msg2 = new TiXmlElement(xmlCommands);
    root->LinkEndChild(msg2);

    // Put the command into the the message
    key = xmlCommandItem;

    value= CommandText;
    msg = new TiXmlElement(key.c_str());
    msg->LinkEndChild(new TiXmlText(value.c_str()));
    msg2->LinkEndChild(msg);

    // Done building XML structure
    doc.SaveFile("MySite.xml");
    TiXmlPrinter printer;
    printer.SetIndent( "    " );

    doc.Accept( &printer );
    string xmltext = printer.CStr();

    return xmltext;
}



string MakeSecureRequestXML(string ReqText, string Destination){

    TiXmlDocument doc;
    TiXmlElement *msg;
    TiXmlComment *comment;
    string s;
    TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );

    doc.LinkEndChild( decl );

    // Create the root element
    TiXmlElement *root = new TiXmlElement(xmlSECUREQRY);
    doc.LinkEndChild( root );

    // Add a comment
    comment = new TiXmlComment();
    s = " Secure Query Request";
    comment->SetValue(s.c_str());
    root->LinkEndChild( comment );

    // First put in a time stamp to show when this document is created
    TiXmlElement * cxn = new TiXmlElement(xmlTimeStamp);
    root->LinkEndChild( cxn );
    cxn->SetAttribute("time", LocalTime().c_str());
    cxn->SetAttribute("date", LocalDate().c_str());

    // Say who this message is for
    cxn = new TiXmlElement(xmlDestination);
    root->LinkEndChild( cxn );
    cxn->SetAttribute("name", Destination.c_str());
    
    // Store the software version just in case some day we care to know
    cxn = new TiXmlElement(xmlSoftVersion);
    root->LinkEndChild( cxn );
    cxn->SetAttribute("major", REV_MAJOR);
    cxn->SetAttribute("minor", REV_MINOR);
    cxn->SetAttribute("build", REV_BUILD);

    // Build the SiteProperties Elements
    TiXmlElement *msgs = new TiXmlElement(xmlSiteProperties);
    root->LinkEndChild(msgs);

    // Put the name of the site in the message
    string key = xmlSiteName;
    string value= Me.MyName;
    msg = new TiXmlElement(key.c_str());
    msg->LinkEndChild(new TiXmlText(value.c_str()));
    msgs->LinkEndChild(msg);

 // Tell them if we are a master site
    key = xmlPublicKey;
    value = Me.PublicKey.c_str();
    msg = new TiXmlElement(key.c_str());
    msg->LinkEndChild(new TiXmlText(value.c_str()));
    msgs->LinkEndChild(msg);

    // Build the list of query items
    TiXmlElement *msg2 = new TiXmlElement(xmlQueries);
    root->LinkEndChild(msg2);

    // Put the name of the site in the message
    key = xmlQuerieItem;

    value= ReqText;
    msg = new TiXmlElement(key.c_str());
    msg->LinkEndChild(new TiXmlText(value.c_str()));
    msg2->LinkEndChild(msg);



    // Done building XML structure
    doc.SaveFile("MySite.xml");
    TiXmlPrinter printer;
    printer.SetIndent( "    " );

    doc.Accept( &printer );
    string xmltext = printer.CStr();

    return xmltext;
}


string MakeSecureResponseXML(XMLnodeList TheNodes, string ToSite){

    TiXmlDocument doc;
    TiXmlElement *msg;
    TiXmlComment *comment;
    string s;
    int i;

    if ((ToSite.size() == 0) || (TheNodes.size() == 0))
        return "";

    TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );

    doc.LinkEndChild( decl );

    // Create the root element
    TiXmlElement *root = new TiXmlElement(xmlSECUREREPLY );
    doc.LinkEndChild( root );

    // Add a comment
    comment = new TiXmlComment();
    s = " Secure Query Response";
    comment->SetValue(s.c_str());
    root->LinkEndChild( comment );

    // First put in a time stamp to show when this document is created
    TiXmlElement * cxn = new TiXmlElement(xmlTimeStamp);
    root->LinkEndChild( cxn );
    cxn->SetAttribute("time", LocalTime().c_str());
    cxn->SetAttribute("date", LocalDate().c_str());

    // Build the SiteProperties Elements
    TiXmlElement *msgs = new TiXmlElement(xmlSiteProperties);
    root->LinkEndChild(msgs);

    // Put the name of the site in the message
    string key = xmlSiteName;
    string value= Me.MyName;
    msg = new TiXmlElement(key.c_str());
    msg->LinkEndChild(new TiXmlText(value.c_str()));
    msgs->LinkEndChild(msg);

    // Build the list of query items
    TiXmlElement *msg2 = new TiXmlElement(xmlSQResponses);
    root->LinkEndChild(msg2);

    // Always put the name of the site we are sending this response to SR_REQ_DESTINATION

    msg = new TiXmlElement(SR_REQ_DESTINATION);
    msg->LinkEndChild(new TiXmlText(ToSite.c_str()));
    msg2->LinkEndChild(msg);

    for (i=0; i< TheNodes.size(); i++){
        // Put the responses into the response node
        // cout << " response" << TheNodes[i].NodeName << endl;
        key = TheNodes[i].NodeName;
        value= TheNodes[i].NodeValue;
        msg = new TiXmlElement(key.c_str());
        msg->LinkEndChild(new TiXmlText(value.c_str()));
        msg2->LinkEndChild(msg);
    }


    // Done building XML structure
    doc.SaveFile("MyResponse.xml");
    TiXmlPrinter printer;
    printer.SetIndent( "    " );

    doc.Accept( &printer );
    string xmltext = printer.CStr();

    return xmltext;
}


std::string GetCigornMessageType(BinaryEntry& msg){
    TiXmlDocument doc;
    TiXmlElement* pElem;
    TiXmlHandle hDoc(&doc);
    string RootElement = "";
    CigornIntersite CA;

    doc.Parse(msg.data, 0,TIXML_DEFAULT_ENCODING);
    pElem = hDoc.FirstChildElement().Element();

    // should always have a valid root but handle gracefully if it does
    if (!pElem) 
        return "";

    RootElement = pElem->Value();  // Is equal to CIGORN if this is from another site

    if ((RootElement.find(xmlCIGORN ) >= 0) && (RootElement.find(xmlCIGORN ) < 3)){
         return xmlCIGORN;
        //cout << "Parsing Intersite" << endl;
    }
    if ((RootElement.find(xmlSECUREQRY) >= 0) && (RootElement.find(xmlSECUREQRY) < 3)){
        return xmlSECUREQRY;
    }
    // #define xmlSQResponses      "SQResponse"
    if ((RootElement.find(xmlSECUREREPLY ) >= 0) && (RootElement.find(xmlSECUREREPLY) < 3)){
        return xmlSECUREREPLY ;
    }
    // #define xmlSQResponses      "SQResponse"
    if ((RootElement.find(xmlCOMMAND) >= 0) && (RootElement.find(xmlCOMMAND) < 3)){
        return xmlCOMMAND;
    }


    return "";
}

void ProcessInterSiteXML(BinaryEntry& msg, CigornSite& SM){
    TiXmlDocument doc;
    TiXmlElement* pElem;
    TiXmlHandle hDoc(&doc);
    string RootElement = "";
    CigornIntersite CA;

    doc.Parse(msg.data, 0,TIXML_DEFAULT_ENCODING);
    pElem = hDoc.FirstChildElement().Element();

    // should always have a valid root but handle gracefully if it does
    if (!pElem) return;

    RootElement = pElem->Value();  // Is equal to CIGORN if this is from another site

    if ((RootElement.find(xmlCIGORN ) >= 0) && (RootElement.find(xmlCIGORN ) < 3)){
        GotInterSiteXML(doc, SM);
        //cout << "Parsing Intersite" << endl;
    }
    if ((RootElement.find(xmlSECUREQRY) >= 0) && (RootElement.find(xmlSECUREQRY) < 3)){
        GotSecureQuery(doc, CA);
        if (CA.MessageType.size() > 0 ){
            // valid message in
        }
    }

    if ((RootElement.find(xmlCOMMAND) >= 0) && (RootElement.find(xmlCOMMAND) < 3)){
        GotCommand(doc, CA);
        if (CA.MessageType.size() > 0 ){
            // valid message in

        }
    }


}


void ProcessSecureXML(BinaryEntry& msg, CigornIntersite& CA){
    TiXmlDocument doc;
    TiXmlElement* pElem;
    TiXmlHandle hDoc(&doc);
    string RootElement = "";


    doc.Parse(msg.data, 0,TIXML_DEFAULT_ENCODING);
    pElem = hDoc.FirstChildElement().Element();

    // should always have a valid root but handle gracefully if it does
    if (!pElem) return;

    RootElement = pElem->Value();  // Is equal to CIGORN if this is from another site

    if ((RootElement.find(xmlSECUREQRY) >= 0) && (RootElement.find(xmlSECUREQRY) < 3)){
        GotSecureQuery(doc, CA);
        if (CA.MessageType.size() > 0 ){
            // valid message in

        }
    }

    if ((RootElement.find(xmlSECUREREPLY) >= 0) && (RootElement.find(xmlSECUREREPLY) < 3)){
        GotSecureReply(doc, CA);
        //cout << "Got Secure Answer.  Nodes:" << CA.XMLnodes.size() << endl;
        if (CA.MessageType.size() > 0 ){
            // valid message in

        }
    }

}

void ProcessXMLcommand(BinaryEntry& msg, CigornIntersite& CA){
    TiXmlDocument doc;
    TiXmlElement* pElem;
    TiXmlHandle hDoc(&doc);
    string RootElement = "";

    doc.Parse(msg.data, 0,TIXML_DEFAULT_ENCODING);
    pElem = hDoc.FirstChildElement().Element();
    // should always have a valid root but handle gracefully if it does
    if (!pElem) return;

    GotCommand(doc, CA); // Load the XML into our Intersite structure
    if (CA.MessageType.size() > 0 ){
        // valid message in

    }

}

// Got a command XML structure
void GotCommand(TiXmlDocument& doc, CigornIntersite& SiteMessage){
        string m_name;
        string el_name;
        string XinTime = "";
        string XinDate = "";
        string XinSiteName = "";
        string DestName = "";
        stringstream ssout;
        XMLvector xv;

        bool XinIsMaster = false;
        WDupdateList NewUpdates;

        int count = 0;

        //cout << "Site Message In" << endl;

	TiXmlHandle hDoc(&doc);
        TiXmlElement* root = doc.RootElement( );
	TiXmlElement* pElem;
	TiXmlElement* pElem1;
	TiXmlHandle hRoot(0);
        TiXmlNode* pNode;
        TiXmlNode* pWdNode;
        TiXmlText *pText;


        pElem = hDoc.FirstChildElement().Element();

        // should always have a valid root but handle gracefully if it does
        if (!pElem) return;

        m_name = pElem->Value();  // Is equal to CIGORN if this is from another site

        // save this for later
        hRoot = TiXmlHandle(pElem);
        //cout << "Element: " << m_name << endl;

        SiteMessage.MessageType = "";    // blank if no message is parsed.

        if (m_name == xmlCOMMAND){
            CoutM2(ssout) << "Cigorn CMD in. ";
            // Traverse children of root, populating the list
            // Initialize the structure to hold site information
            SiteMessage.ready = false;
            SiteMessage.sitedate ="";
            SiteMessage.sitename="";
            SiteMessage.sitetime = "" ;         // the time clock of the remote gateway
            SiteMessage.timein = time(NULL);    // the time this message came in.
            SiteMessage.MessageType = xmlCOMMAND;
            SiteMessage.XMLnodes.clear();
            for ( pElem = root->FirstChildElement( ); pElem;  pElem = pElem->NextSiblingElement( ) )
            {
               if (pElem){
                  //cout << pElem->Value() << endl;
                  m_name = pElem->Value();
                  //cout << "M " << m_name<< endl;
                  if (m_name == xmlTimeStamp){
                      // The timestamp element telling us when the site sent the XML message
                      XinTime = Attribute(pElem,xmlTime);
                      XinDate = Attribute(pElem,xmlDate);
                      SiteMessage.sitetime = XinTime;
                      SiteMessage.sitedate = XinDate;
                  }
                  if (m_name == xmlDestination){
                      // The destination for this command
                      DestName  = Attribute(pElem,xmlName);
                  }

                  if (m_name == xmlSiteProperties){
                      // The name and information about the SITE that sent this message
                      pNode = pElem->FirstChild(xmlSiteName);
                      //pChild = pNode->FirstChildElement();
                      if (pNode != NULL){
                        //cout << "Child " << pNode->Value() << "  " << XinSiteName << endl;  //xmlSiteName
                        // XinSiteName = pNode->FirstChild()->ToText();
                        if (pNode->FirstChild() != NULL)
                            XinSiteName = pNode->FirstChild()->Value();
                        else
                            XinSiteName = "";
                        SiteMessage.sitename = XinSiteName;
                        CoutM2(ssout) << "  From:" << XinSiteName << "  ";
                      }
                      pNode = pElem->FirstChild(xmlIsMaster );
                      //pChild = pNode->FirstChildElement();
                      if (pNode != NULL){
                        if (pNode->FirstChild() != NULL)
                            XinIsMaster = IsStringTrue(pNode->FirstChild()->Value());
                        //SiteMessage.ismaster = XinIsMaster;
                        //cout << "Child " << pNode->Value() << "  " << XinIsMaster << endl;  //xmlSiteName
                      }
                  }
                  if (m_name == xmlPublicKey){
                      // The name and information about the SITE that sent this message
                      pNode = pElem->FirstChild(xmlSiteName);
                      //pChild = pNode->FirstChildElement();
                      if (pNode != NULL){
                        //cout << "Child " << pNode->Value() << "  " << XinSiteName << endl;  //xmlSiteName
                        // XinSiteName = pNode->FirstChild()->ToText();
                        if (pNode->FirstChild() != NULL){
                            xv.NodeName = m_name;
                            xv.NodeValue =  pNode->FirstChild()->Value();
                            SiteMessage.XMLnodes.insert(SiteMessage.XMLnodes.end(), xv );
                        }
                      }
                  }
                  // xmlQueries.  Something else requestd us to send them our DB password in a secure XML package
                  if (m_name == xmlCommands){
                      // The name and information about the SITE that sent this message
                      //m_name = pElem->FirstChild()->Value();
                      //m_name = pElem->FirstChild()->;
                      for ( pElem1 = pElem->FirstChildElement( ); pElem1;  pElem1 = pElem1->NextSiblingElement( ) ){
                          if (pElem1){
                            el_name = pElem1->FirstChild()->Value();  // the text "DBpassword"  Secure Query Item name
                            xv.NodeName = pElem1->Value();
                            xv.NodeValue =  el_name;
                            SiteMessage.XMLnodes.insert(SiteMessage.XMLnodes.end(), xv );
                            CoutM2(ssout) << " " << xv.NodeValue;
                          }
                      }
                  }
               }// for (pElem...

            }
            SiteMessage.ready = true;  // we parsed it, and it is ready to be used.

          }
        CoutM2(ssout) << "  Cmd Count:" << SiteMessage.XMLnodes.size() << " To:" << DestName;

        if (IsSameText(DestName, Me.MyName) == false){
            SiteMessage.ready = false;     // throw this out.  not for us.
            SiteMessage.MessageType = "";
            SiteMessage.XMLnodes.clear();
            CoutM2(ssout) << "  (Ignored)";
        }

        if (ssout.str().size()>0 )
            ssout << endl;
        CoutM2(ssout);
        MyCLI.Display(&ssout);   // output the string to the Command-Line user and then erase it.

}

// An secure XML parameter request came in from another site
void GotSecureQuery(TiXmlDocument& doc, CigornIntersite& SiteMessage){
        string m_name;
        string el_name;
        string XinTime = "";
        string XinDate = "";
        string XinSiteName = "";
        string S="";
        string DestName = "";
        stringstream ssout;
        XMLvector xv;

        bool XinIsMaster = false;
        WDupdateList NewUpdates;

        int count = 0;

        //cout << "Site Message In" << endl;

	TiXmlHandle hDoc(&doc);
        TiXmlElement* root = doc.RootElement( );
	TiXmlElement* pElem;
	TiXmlElement* pElem1;
	TiXmlHandle hRoot(0);
        TiXmlNode* pNode;
        TiXmlNode* pWdNode;
        TiXmlText *pText;


        pElem = hDoc.FirstChildElement().Element();

        // should always have a valid root but handle gracefully if it does
        if (!pElem) return;

        m_name = pElem->Value();  // Is equal to CIGORN if this is from another site

        // save this for later
        hRoot = TiXmlHandle(pElem);
        //cout << "Element: " << m_name << endl;

        SiteMessage.MessageType = "";    // blank if no message is parsed.

        if (m_name == xmlSECUREQRY){
            CoutM2(ssout) << "Cigorn SQ in. ";
            // Traverse children of root, populating the list
            // Initialize the structure to hold site information
            SiteMessage.ready = false;
            SiteMessage.sitedate ="";
            SiteMessage.sitename="";
            SiteMessage.sitetime = "" ;         // the time clock of the remote gateway
            SiteMessage.timein = time(NULL);    // the time this message came in.
            SiteMessage.MessageType = xmlSECUREQRY;
            SiteMessage.XMLnodes.clear();
            for ( pElem = root->FirstChildElement( ); pElem;  pElem = pElem->NextSiblingElement( ) )
            {
               if (pElem){
                  //cout << pElem->Value() << endl;
                  m_name = pElem->Value();
                  //cout << "M " << m_name<< endl;
                  if (m_name == xmlTimeStamp){
                      // The timestamp element telling us when the site sent the XML message
                      XinTime = Attribute(pElem,xmlTime);
                      XinDate = Attribute(pElem,xmlDate);
                      SiteMessage.sitetime = XinTime;
                      SiteMessage.sitedate = XinDate;
                  }
                  if (m_name == xmlDestination){
                      // The destination for this query
                      DestName  = Attribute(pElem,xmlName);
                  }

                  if (m_name == xmlSiteProperties){
                      // The name and information about the SITE that sent this message
                      pNode = pElem->FirstChild(xmlSiteName);
                      //pChild = pNode->FirstChildElement();
                      if (pNode != NULL){
                        //cout << "Child " << pNode->Value() << "  " << XinSiteName << endl;  //xmlSiteName
                        // XinSiteName = pNode->FirstChild()->ToText();
                        if (pNode->FirstChild() != NULL)
                            XinSiteName = pNode->FirstChild()->Value();
                        else
                            XinSiteName = "";
                        SiteMessage.sitename = XinSiteName;
                        CoutM2(ssout) << "  From:" << XinSiteName << "  ";
                      }
                      pNode = pElem->FirstChild(xmlIsMaster );
                      //pChild = pNode->FirstChildElement();
                      if (pNode != NULL){
                        if (pNode->FirstChild() != NULL)
                            XinIsMaster = IsStringTrue(pNode->FirstChild()->Value());
                        //SiteMessage.ismaster = XinIsMaster;
                        //cout << "Child " << pNode->Value() << "  " << XinIsMaster << endl;  //xmlSiteName
                      }
                  }
                  if (m_name == xmlPublicKey){
                      // The name and information about the SITE that sent this message
                      pNode = pElem->FirstChild(xmlSiteName);
                      //pChild = pNode->FirstChildElement();
                      if (pNode != NULL){
                        //cout << "Child " << pNode->Value() << "  " << XinSiteName << endl;  //xmlSiteName
                        // XinSiteName = pNode->FirstChild()->ToText();
                        if (pNode->FirstChild() != NULL){
                            xv.NodeName = m_name;
                            xv.NodeValue =  pNode->FirstChild()->Value();
                            SiteMessage.XMLnodes.insert(SiteMessage.XMLnodes.end(), xv );
                        }
                      }
                  }
                  // xmlQueries.  Something else requestd us to send them our DB password in a secure XML package
                  if (m_name == xmlQueries){
                      // The name and information about the SITE that sent this message
                      //m_name = pElem->FirstChild()->Value();
                      //m_name = pElem->FirstChild()->;
                      for ( pElem1 = pElem->FirstChildElement( ); pElem1;  pElem1 = pElem1->NextSiblingElement( ) ){
                          if (pElem1){
                            el_name = pElem1->FirstChild()->Value();  // the text "DBpassword"  Secure Query Item name
                            xv.NodeName = pElem1->Value();
                            xv.NodeValue =  el_name;
                            SiteMessage.XMLnodes.insert(SiteMessage.XMLnodes.end(), xv );
                            CoutM2(ssout) << " " << xv.NodeValue;
                          }
                      }
                  }


               }// for (pElem...

            }
            SiteMessage.ready = true;  // we parsed it, and it is ready to be used.

          }
        CoutM2(ssout) << "  Query Count:" << SiteMessage.XMLnodes.size() << " TO:" << DestName;

        if (IsSameText(DestName, Me.MyName) == false){
            SiteMessage.ready = false;     // throw this out.  not for us.
            SiteMessage.MessageType = "";
            SiteMessage.XMLnodes.clear();
            CoutM2(ssout) << "  (Ignored)";
        }
        
        if (ssout.str().size()>0 )
            ssout << endl;
        CoutM2(ssout);
        MyCLI.Display(&ssout);   // output the string to the Command-Line user and then erase it.

}


// A secure XML reply came in from another gateway
void GotSecureReply(TiXmlDocument& doc, CigornIntersite& SiteMessage){
        string m_name;
        string el_name;
        string XinTime = "";
        string XinDate = "";
        string XinSiteName = "";
        string S="";
        XMLvector xv;

        bool XinIsMaster = false;
        
	TiXmlHandle hDoc(&doc);
        TiXmlElement* root = doc.RootElement( );
	TiXmlElement* pElem;
	TiXmlElement* pElem1;
	TiXmlHandle hRoot(0);
        TiXmlNode* pNode;

        pElem = hDoc.FirstChildElement().Element();

        // should always have a valid root but handle gracefully if it does
        if (!pElem) return;

        m_name = pElem->Value();  // Is equal to CIGORN if this is from another site

        // save this for later
        hRoot = TiXmlHandle(pElem);

        SiteMessage.MessageType = "";    // blank if no message is parsed.

        if (m_name == xmlSECUREREPLY){
            //ssout << "Cigorn SR in. ";
            // Traverse children of root, populating the list
            // Initialize the structure to hold site information
            SiteMessage.ready = false;
            SiteMessage.sitedate ="";
            SiteMessage.sitename="";
            SiteMessage.sitetime = "" ;         // the time clock of the remote gateway
            SiteMessage.timein = time(NULL);    // the time this message came in.
            SiteMessage.MessageType = xmlSECUREQRY;
            SiteMessage.XMLnodes.clear();
            for ( pElem = root->FirstChildElement( ); pElem;  pElem = pElem->NextSiblingElement( ) )
            {
               if (pElem){
                  m_name = pElem->Value();
                  if (m_name == xmlTimeStamp){
                      // The timestamp element telling us when the site sent the XML message
                      XinTime = Attribute(pElem,xmlTime);
                      XinDate = Attribute(pElem,xmlDate);
                      SiteMessage.sitetime = XinTime;
                      SiteMessage.sitedate = XinDate;
                  }

                  if (m_name == xmlSiteProperties){
                      // The name and information about the SITE that sent this message
                      pNode = pElem->FirstChild(xmlSiteName);
                      //pChild = pNode->FirstChildElement();
                      if (pNode != NULL){
                        // XinSiteName = pNode->FirstChild()->ToText();
                        if (pNode->FirstChild() != NULL)
                            XinSiteName = pNode->FirstChild()->Value();
                        else
                            XinSiteName = "";
                        SiteMessage.sitename = XinSiteName;
                      }
                      pNode = pElem->FirstChild(xmlIsMaster );
                      //pChild = pNode->FirstChildElement();
                      if (pNode != NULL){
                        if (pNode->FirstChild() != NULL)
                            XinIsMaster = IsStringTrue(pNode->FirstChild()->Value());
                      }
                  }
                  if (m_name == xmlPublicKey){
                      // The name and information about the SITE that sent this message
                      pNode = pElem->FirstChild(xmlSiteName);
                      //pChild = pNode->FirstChildElement();
                      if (pNode != NULL){
                        // XinSiteName = pNode->FirstChild()->ToText();
                        if (pNode->FirstChild() != NULL){
                            xv.NodeName = m_name;
                            xv.NodeValue =  pNode->FirstChild()->Value();
                            SiteMessage.XMLnodes.insert(SiteMessage.XMLnodes.end(), xv );
                        }
                      }
                  }
                  // xmlQueries.  Something else requestd us to send them our DB password in a secure XML package
                  if (m_name == xmlSQResponses){
                      // The name and information about the SITE that sent this message
                      for ( pElem1 = pElem->FirstChildElement( ); pElem1;  pElem1 = pElem1->NextSiblingElement( ) ){
                          if (pElem1){
                            //el_name = pElem1->FirstChild()->Value();  // the text "DBpassword"  Secure Query Item name
                            xv.NodeName = ElementTag(pElem1);
                            xv.NodeValue =  ElementValue(pElem1);
                            SiteMessage.XMLnodes.insert(SiteMessage.XMLnodes.end(), xv );
                          }
                      }
                  }
               }// for (pElem...
            }
            SiteMessage.ready = true;  // we parsed it, and it is ready to be used.
          }
  
}



// An XML message from another site came in
void GotInterSiteXML(TiXmlDocument& doc, CigornSite& SiteMessage){
        string m_name;
        string el_name;
        string XinTime = "";
        string XinDate = "";
        string XinSiteName = "";
        string XinNodeVal = "";
        string TableName = "";
        string Index="";
        string ColumnName="";
        string ColumnVal="";
        string S="";
        stringstream ssout;

        bool XinIsMaster = false;
        bool XinIsActive = false;
        int IDl = 0;
        int IDh = 0;
        int idx;
        WDupdateList NewUpdates;

        int count = 0;

 
	TiXmlHandle hDoc(&doc);
        TiXmlElement* root = doc.RootElement( );
	TiXmlElement* pElem;
	TiXmlElement* pRowElem;
	TiXmlElement* pColElem;
	TiXmlElement* pWdElem;
	TiXmlHandle hRoot(0);
        TiXmlNode* pNode;
        TiXmlNode* pWdNode;
        TiXmlText *pText;


        pElem = hDoc.FirstChildElement().Element();

        // should always have a valid root but handle gracefully if it does
        if (!pElem) return;

        m_name = pElem->Value();  // Is equal to CIGORN if this is from another site

        // save this for later
        hRoot = TiXmlHandle(pElem);
        //cout << "Element::: " << m_name << endl;

        CoutM2(ssout) << "Cigorn XML in.  Root: "<< m_name << endl;

        if (m_name == xmlCIGORN){
            // Traverse children of root, populating the list
            // Initialize the structure to hold site information
            SiteMessage.valid = true;    // flag a new cigorn site message came in
            SiteMessage.ready = false;
            SiteMessage.sitedate ="";
            SiteMessage.description="";
            SiteMessage.ischief = false;
            SiteMessage.isactive = false;
            SiteMessage.msgincount = 0;
            SiteMessage.msgoutcount = 0;
            SiteMessage.sitename="";
            SiteMessage.socketcount = 0;
            SiteMessage.sitetime = "" ;         // the time clock of the remote gateway
            SiteMessage.timein = time(NULL);    // the time this message came in.
            SiteMessage.ver_build = 0;
            SiteMessage.ver_major = 0;
            SiteMessage.ver_minor = 0;

            for ( pElem = root->FirstChildElement( ); pElem;  pElem = pElem->NextSiblingElement( ) )
            {
               if (pElem){
                  //cout << pElem->Value() << endl;
                  m_name = pElem->Value();

                  if (m_name == xmlTimeStamp){
                      // The timestamp element telling us when the site sent the XML message
                      XinTime = Attribute(pElem,xmlTime);
                      XinDate = Attribute(pElem,xmlDate);
                      SiteMessage.sitetime = XinTime;
                      SiteMessage.sitedate = XinDate;
                  }
  
                  // See if this element is a data table update. Must come from a Master Site.
                  if ((m_name == xmlTableUpdate) && (XinIsMaster)){
                      // a data table update from a Master Site. What is the table name?
                      TableName = Attribute(pElem, xmlTableNameAttrib);    // name of the table that it wants to update
                      //cout << "Update table:" << TableName << endl;
                      if (TableName.size() > 0){
                          // It is a name, so loop through and read in the updates for the rows.
                          for ( pRowElem = pElem->FirstChildElement( );pRowElem;  pRowElem = pRowElem->NextSiblingElement( ) )
                          {// see if the index looks valid. xmlTableColumn
                           if (pRowElem){
                              // cout << pElem->Value() << endl;
                              el_name = pRowElem->Value();

                              if (el_name == xmlIDrange){
                                    pRowElem->QueryIntAttribute(xmlFirstID, &IDl);    // returns 0 if found, 1 if not found
                                    pRowElem->QueryIntAttribute(xmlLastID, &IDh);     // returns 0 if found, 1 if not found
                                    //cout << "WD table updates. ID " << IDl << " to " << IDh << endl;  //xmlSiteName
                              }



                              if (el_name == xmlRowUpdate){
                                  // We have col data for this row. Get it
                                  Index = pRowElem->FirstAttribute()->Value();
                                  if (StringIsInteger(Index)){
                                      idx = StringToInt(Index);
                                      // we got an index for this row. Now loop throught the columns and get the fields that should be updated. pColElem
                                      for ( pColElem = pRowElem->FirstChildElement( );pColElem;  pColElem = pColElem->NextSiblingElement( ) )
                                      {// see if the index looks valid
                                          if(pColElem){
                                              el_name = pColElem->Value();
                                              if (el_name == xmlTableColumn){
                                                  // Here is an elemet with the data for a particualr field in the table
                                                  if (pColElem->FirstAttribute()){
                                                       //ColumnName = pColElem->FirstAttribute();
                                                       ColumnVal = Attribute(pColElem,1);  // get the first attribute
                                                       ColumnName = AttributeName(pColElem,1);  // and its name
                                                       if ((ColumnVal.size() > 0 ) && (ColumnName.size()>0 ) && (idx >0 )){
                                                           // seems to be valid data.
                                                           dtWD->StoreMSupdate(idx, ColumnName, ColumnVal);
                                                       }
                                                  }
                                                 
                                              }
                                          }
                                         // cout << "Row:" << Index << " Col:" << ColumnName << " " << ColumnVal <<endl;

                                        }


                                  }
                              }
                           }// if pRowElem
                          }


                      }
                  }
                  if (m_name == xmlSiteStatistics){
                      // The name and information about the SITE that sent this message
                      S = Attribute(pElem,xmlSocketCount);
                      SiteMessage.socketcount = StringToInt(S);
                      S = Attribute(pElem,xmlMsgInCount);
                      SiteMessage.msgincount = StringToInt(S);
                      S = Attribute(pElem,xmlMsgOutCount);
                      SiteMessage.msgoutcount = StringToInt(S);
                      S = Attribute(pElem,"abc");
                  }
                  if (m_name == xmlSiteProperties){
                      // The name and information about the SITE that sent this message
                      pNode = pElem->FirstChild(xmlSiteName);
                      //pChild = pNode->FirstChildElement();
                      if (pNode != NULL){
                        //cout << "Child " << pNode->Value() << "  " << XinSiteName << endl;  //xmlSiteName
                        // XinSiteName = pNode->FirstChild()->ToText();
                        if (pNode->FirstChild() != NULL)
                            XinSiteName = pNode->FirstChild()->Value();
                        else
                            XinSiteName = "";
                        SiteMessage.sitename = XinSiteName;
                      }
                      pNode = pElem->FirstChild(xmlIsMaster );
                      //pChild = pNode->FirstChildElement();
                      if (pNode != NULL){
                        if (pNode->FirstChild() != NULL)
                            XinIsMaster = IsStringTrue(pNode->FirstChild()->Value());
                        SiteMessage.ischief = XinIsMaster;
                        //cout << "Child " << pNode->Value() << "  " << XinIsMaster << endl;  //xmlSiteName
                      }
                      pNode = pElem->FirstChild(xmlIsActive);
                      //pChild = pNode->FirstChildElement();
                      if (pNode != NULL){
                        if (pNode->FirstChild() != NULL)
                            XinIsActive = IsStringTrue(pNode->FirstChild()->Value());
                        SiteMessage.isactive = XinIsActive;
                        //cout << "Child " << pNode->Value() << "  " << XinIsMaster << endl;  //xmlSiteName
                      }


                      pNode = pElem->FirstChild(xmlDBmodified );
                      //pChild = pNode->FirstChildElement();
                      if (pNode != NULL){
                        if (pNode->FirstChild() != NULL)
                            XinIsMaster = IsStringTrue(pNode->FirstChild()->Value());
                        SiteMessage.DBmodFlag = XinIsMaster;
                        //cout << "Child " << pNode->Value() << "  " << XinIsMaster << endl;  //xmlSiteName
                      }


                  }
                  if (m_name == xmlWirelessDevices){
                        // This structure is an update of WD parameters from a master site.
                        int t = pElem->Type();
                        dataupdate wdUpdates(WDEVICE);
                        string newdata = "";
                        switch (t)
                        {
                        case TiXmlNode::TINYXML_DOCUMENT:
                            break;
                        case TiXmlNode::TINYXML_ELEMENT:
                            TiXmlNode * pChild;
                            for ( pChild = pElem->FirstChild(); pChild != 0; pChild = pChild->NextSibling()){
                                // Loop through the list of WDs and get thier settings.
                                if(pChild){
                                    string Val = pChild->Value();
                                    //cout << Val << "|" << endl;
                                    if (Val == xmlIDrange){
                                        pChild->ToElement()->QueryIntAttribute(xmlFirstID, &IDl);    // returns 0 if found, 1 if not found
                                        pChild->ToElement()->QueryIntAttribute(xmlLastID, &IDh);     // returns 0 if found, 1 if not found
                                        //cout << "WD Table Updates. ID " << IDl << " to " << IDh << endl;  //xmlSiteName
                                    }

                                    if (Val == xmlWDdevice){
                                        WDupdate ThisUpdate;
                                        //Attribute(pElem,xmlDate);

                                        ThisUpdate.Enabled = IsStringTrue(Attribute(pElem,xmlWDenabled));
                                        //ThisUpdate.Enabled = IsStringTrue(pChild->ToElement()->Attribute(xmlWDenabled));
                                        pChild->ToElement()->QueryIntAttribute(xmlWDid, &ThisUpdate.ID);   // returns 0 if found, 1 if not found
                                        pChild->ToElement()->QueryIntAttribute(xmlWDsystem, &ThisUpdate.System);   // returns 0 if found, 1 if not found
                                        if (XinIsMaster){
                                            // this information came from a Cigorn Master site, so we need to update our DB with it.
                                            NewUpdates[ThisUpdate.ID]= ThisUpdate;  // save the update in the list of updates
                                            wdUpdates.putupdate(xmlWDid, xmlWDsystem,newdata) ;
                                        }
                                    }
                                }
                            }
                            if ((NewUpdates.size() > 0 ) && (XinIsMaster == true) && (Me.IsChief == false)){
                                // We were given new updates from a Master to store to the database
                                UpdateWDtable(NewUpdates, IDl, IDh);
                            }
                            break;
                        case TiXmlNode::TINYXML_COMMENT:
                            break;
                        case TiXmlNode::TINYXML_UNKNOWN:
                            break;
                        case TiXmlNode::TINYXML_TEXT:
                            pText = pElem->ToText();
                            break;
                        case TiXmlNode::TINYXML_DECLARATION:
                            break;
                        default:
                            break;
                        }

                    }

               }// for (pElem...

            }
            SiteMessage.ready = true;  // we parsed it, and it is ready to be used.

          }

        MyCLI.Display(&ssout);   // output the string to the Command-Line user and then erase it.

}

//l_name = pElem1->FirstChild()->Value();
string ElementValue(TiXmlElement* pElem){


    if (pElem->NoChildren())
        return "";

    const char *p = pElem->FirstChild()->Value();


    // NULL test and return value if not null
    if (p)
        return ToString(p);
    else
        return "";
}


//l_name = pElem1->FirstChild()->Value();
string ElementTag(TiXmlElement* pElem){

    
    const char *p = pElem->Value();

    // NULL test and return value if not null
    if (p)
        return ToString(p);
    else
        return "";
}


string Attribute(TiXmlElement* pElem, int i){

    const char *p = pElem->FirstAttribute()->Value();

    // NULL test and return value if not null
    if (p){
        return ToString(p);
    }
    else
        return "";
}

string AttributeName(TiXmlElement* pElem, int i){

    const char *p = pElem->FirstAttribute()->Name();

    // NULL test and return value if not null
    if (p){
        return ToString(p);
    }
    else
        return "";
}
string Attribute(TiXmlElement* pElem, string attr){
    if (attr.size()==0)
        return "";

    const char *p = pElem->Attribute(attr.c_str());

    // NULL test and return value if not null
    if (p){
        return ToString(p);
    }
    else
        return "";
}

// give a tag s, create the closing tag string.
string closetag(string s){
    string r="";
    int i;
    for (i=0; i < s.size(); i++){
        if (s[i] == '<')
            r = r + "</";
        else
            r = r + s[i];
    }
    return r;
}

// crate a tag <xx>
string maketag(string s){
    string r;
    r = "<" + s + ">";
    return r;
}


// Find the tag for a node and return it as a string
string findroot(char *buff, int max, int node){

    int i = 0;
    int nodecount = 0;
    bool endofnode = true;  // we have not yet found a node

    string s = "";

    while(i < max){
        if (buff[i] == '<'){
            nodecount++;
            endofnode = false;
        }
        if (nodecount == node){
            // This is the root node
            s = s + buff[i];
        }
        if (buff[i] == '>'){
            endofnode = true;
        }

        if ((endofnode == true) && (nodecount == node))
            return s;

        i++;
    }

    s = ""; // didn't find a node.
    return s;
}
