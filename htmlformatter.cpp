/* 
 * File:   htmlformatter.cpp
 * Author: john
 * 
 * Created on December 23, 2010, 11:11 PM
 */

#include "htmlformatter.h"

#include <string>
#include <queue>
#include <vector>
#include <sstream>

using namespace std;

htmlformatter::htmlformatter() {
    NumOfCols = 0;
}

htmlformatter::htmlformatter(const htmlformatter& orig) {
}

htmlformatter::~htmlformatter() {
}


string htmlformatter::htmlformat(string s, string tag) {

    s = "<" + tag + ">" + s + "</" + tag + ">";
    return s;
}

string htmlformatter::htmlDataInputFormat(string s, string tag){

    s = "<td bgcolor=\"#f0f0f0\"><input type=\"TEXT\" name=\"ABC\" value=\"" + s + "\" maxlength=\"31\"></td>";
    return s;

}

string htmlformatter::TagOpen(string tag) {
    string s;
    s = "<" + tag + ">";
    return s;
}

string htmlformatter::TagOpen(string tag, string attr) {
    string s;
    s = "<" + tag + " " + attr + ">";
    return s;
}

string htmlformatter::TagOpenClose(string tag, string attr) {
    string s;
    s = "<" + tag + " " + attr + "/>";
    return s;
}

string htmlformatter::FormOpen(string formtitle) {
    
    std::stringstream ss;
    ss << "<form  METHOD=POST ACTION=\"" << formtitle << "\" >" << endl;
    return ss.str();
}

string htmlformatter::TagClose(string tag) {
    string s;
    s = "</" + tag + ">";
    return s;
}

// Store two strings in the mytable for later conversin to HTML table
void htmlformatter::ToTable(string c1, string c2) {
    tabledata2 T2;

    T2.col1 = c1;
    T2.col2 = c2;
    T2.col3 = "";
    T2.col4 = "";
    T2.col5 = "";
    T2.col6 = "";

    mytable.insert(mytable.end(),T2);
    NumOfCols = 2;
    return;

}

// Store two strings in the mytable for later conversin to HTML table
void htmlformatter::ToTable(string c1, string c2, string c3) {
    tabledata2 T2;

    T2.col1 = c1;
    T2.col2 = c2;
    T2.col3 = c3;
    T2.col4 = "";
    T2.col5 = "";
    T2.col6 = "";

    mytable.insert(mytable.end(),T2);
    NumOfCols = 3;
    return;
}

// Store two strings in the mytable for later conversin to HTML table
void htmlformatter::ToTable(string c1, string c2, string c3, string c4) {
    tabledata2 T2;

    T2.col1 = c1;
    T2.col2 = c2;
    T2.col3 = c3;
    T2.col4 = c4;
    T2.col5 = "";
    T2.col6 = "";

    mytable.insert(mytable.end(),T2);
    NumOfCols = 4;
    return;
}

// Store two strings in the mytable for later conversin to HTML table
void htmlformatter::ToTable(string c1, string c2, string c3, string c4, string c5) {
    tabledata2 T2;

    T2.col1 = c1;
    T2.col2 = c2;
    T2.col3 = c3;
    T2.col4 = c4;
    T2.col5 = c5;
    T2.col6 = "";

    mytable.insert(mytable.end(),T2);
    NumOfCols = 5;
    return;
}

// Store two strings in the mytable for later conversin to HTML table
void htmlformatter::ToTable(string c1, string c2, string c3, string c4, string c5, string c6) {
    tabledata2 T2;

    T2.col1 = c1;
    T2.col2 = c2;
    T2.col3 = c3;
    T2.col4 = c4;
    T2.col5 = c5;
    T2.col6 = c6;

    mytable.insert(mytable.end(),T2);
    NumOfCols = 6;
    return;
}

// Create a table with the format of the text
string htmlformatter::TableToHTML(string format) {
    // The table data
    std::stringstream ss;
    string s;
    int row;
    htmlformatter fm;

    ss << TagOpen(tag_tablebody);

    // loop through the rows
    for (row=0; row < mytable.size(); row++){
        if (format.size() > 1)
            ss << fm.TagOpen(format);
        ss << TagOpen(tag_tablerow);
        s = htmlformat(mytable[row].col1, format);
        ss << htmlformat(s, tag_tabledata);
        s = htmlformat(mytable[row].col2, format);
        ss << htmlformat(s, tag_tabledata);
        if (NumOfCols > 2){
            s = htmlformat(mytable[row].col3, format);
            ss << htmlformat(s, tag_tabledata);
        }
        if (NumOfCols > 3){
            s = htmlformat(mytable[row].col4, format);
            ss << htmlformat(s, tag_tabledata);
        }
        if (NumOfCols > 4){
            s = htmlformat(mytable[row].col5, format);
            ss << htmlformat(s, tag_tabledata);
        }
        if (NumOfCols > 5){
            s = htmlformat(mytable[row].col6, format);
            ss << htmlformat(s, tag_tabledata);
        }
        ss << endl;
        ss << TagClose(tag_tablerow);
        if (format.size() > 1)
            ss << fm.TagClose(format);

    }
    ss << TagClose(tag_tablebody);
    s = ss.str();
    return s;
}


// Create a table with the format of the text
string htmlformatter::TableParmsToHTML(string format) {
    // The table data
    std::stringstream ss;
    string s;
    int row;
    htmlformatter fm;

    ss << "<table cellpadding = \"2\" width=\"500\" >";

    // loop through the rows
    for (row=0; row < mytable.size(); row++){
        if (format.size() > 1)
            ss << fm.TagOpen(format);
        ss << TagOpen(tag_tablerow);
        s = htmlformat(mytable[row].col1, format);
        ss << htmlformat(s, tag_tabledata);
        
        //s = htmlformat(mytable[row].col2, "a");
        //ss << htmlformat(s, tag_tabledata);
        ss << htmlDataInputFormat(mytable[row].col2, "X");

        if (NumOfCols > 2){
            s = htmlformat(mytable[row].col3, format);
            ss << htmlformat(s, tag_tabledata);
        }
        if (NumOfCols > 3){
            s = htmlformat(mytable[row].col4, format);
            ss << htmlformat(s, tag_tabledata);
        }
        if (NumOfCols > 4){
            s = htmlformat(mytable[row].col5, format);
            ss << htmlformat(s, tag_tabledata);
        }
        if (NumOfCols > 5){
            s = htmlformat(mytable[row].col6, format);
            ss << htmlformat(s, tag_tabledata);
        }
        ss << endl;
        ss << TagClose(tag_tablerow);
        if (format.size() > 1)
            ss << fm.TagClose(format);

    }
    ss << TagClose(tag_tablebody);
    s = ss.str();
    return s;
}


string htmlformatter::TableToHTML(void) {
    // The table data
    return TableToHTML("");
}

// "size=\"1\" color=\"red\""
string htmlformatter::FormatedText(string s, int size, string color){
       std::stringstream ss;
       string st;
       ss << "<font size=\"" << size << "\" color=\"" << color << "\">";
       ss << s;
       ss << "</font>" << endl;
       st = ss.str();
       return st;
}

// "size=\"1\" color=\"red\""
string htmlformatter::FormatedText(string Text, int size, string color, string font1){
       std::stringstream ss;
       string st;
       ss << "<font size=\"" << size << "\" color=\"" << color << "\">";
       if (font1.size() > 0)
           ss << "<" << font1 << ">";
       ss << Text;  // the text to display
       if (font1.size() > 0)
           ss << "</" << font1 << ">";

       ss << "</font>" << endl;
       st = ss.str();
       return st;
}
