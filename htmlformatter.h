/* 
 * File:   htmlformatter.h
 * Author: john
 *
 * Created on December 23, 2010, 11:11 PM
 */

#ifndef HTMLFORMATTER_H
#define	HTMLFORMATTER_H

#include <string>
#include <vector>

using namespace std;

#define     tag_para        "p"
#define     tag_h1          "h1"
#define     tag_h2          "h2"
#define     tag_h3          "h3"
#define     tag_bold        "b"
#define     tag_link        "a"
#define     tag_image       "img"
#define     tag_emphasis    "em"
#define     tag_strong      "strong"
#define     tag_linebreak   "br"
#define     tag_table       "table"
#define     tag_form        "form"
#define     tag_caption     "caption"
#define     tag_thread      "thread"
#define     tag_tablerow    "tr"
#define     tag_tableheader "th"
#define     tag_tablebody   "tbody"
#define     tag_tabledata   "td"
#define     tag_small       "small"
#define     tag_italic      "i"
#define     tag_head        "head"
#define     tag_body        "body"
#define     tag_meta        "meta"
#define     tag_title       "title"
#define     tag_font        "font"
#define     tag_first       "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">"
#define     tag_center      "center"
#define     tag_txt_fixed  "kbd"    // keyboard text. Fixed width.

#define     COLOR_BLACK    "black"
#define     COLOR_BLUE     "blue"


// Some HTM shorcuts
#define     html_endl       "<p>"
#define     html_linebrk    "<br />"

struct tabledata2{
    string col1;
    string col2;
    string col3;
    string col4;
    string col5;
    string col6;
};


class htmlformatter {
public:
    htmlformatter();
    htmlformatter(const htmlformatter& orig);
    virtual ~htmlformatter();

    string htmlformat(string , string );
    string htmlDataInputFormat(string, string);
    string TagOpen(string );
    string TagOpen(string, string);
    string TagOpenClose(string, string);
    string FormOpen(string);
    string TagClose(string );
    string TableToHTML(void);
    string TableToHTML(string);
    string TableParmsToHTML(string);
    string FormatedText(string, int , string);   //size and color
    string FormatedText(string, int, string, string);

    void ToTable(string, string);
    void ToTable(string, string, string);
    void ToTable(string, string, string, string);
    void ToTable(string, string, string, string, string);
    void ToTable(string, string, string, string, string, string);

    vector <tabledata2> mytable;   // array of strings to make into an HTML table
    int NumOfCols;

private:

};

#endif	/* HTMLFORMATTER_H */

