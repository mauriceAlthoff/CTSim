/*****************************************************************************
**      Name:         pol.h
**      Purpose:      Header file for Problem Oriented Language Class
**      Programmer:   Kevin Rosenberg
**      Date Started: 1984
**
**  This is part of the CTSim program
**  Copyright (c) 1983-2009 Kevin Rosenberg
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License (version 2) as
**  published by the Free Software Foundation.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
******************************************************************************/
#ifndef __H_POL
#define __H_POL

#include "hashtable.h"
#include <stack>

class POL {

  public:

    // codes for pol_usefile
    enum {
      P_USE_STR = 1,            // use string as input source
      P_USE_FILE,                 // use file as input source
    };


    POL();
    ~POL();

    void init ();
    void addSkipWord (const char* const w);
    void addSkipChar (int c);
    void addKeyword (const char* const str, int code);
    bool readWord (const char *search, int nlet);
    bool readUserToken (char *str, int *code);
    bool readString (char *str);
    bool readInteger (int *n, int typecode, bool boundcode, int bb1, int bb2);
    bool readFloat (double *n, double typecode, bool boundcode, double bb1, double bb2);
    bool skipTokens ();
    void reader ();
    bool readText (char *str, int lim);
    void usefile (int source, const char *fn);
    void closefile ();
    int lookchar ();
    int inchar ();
    void ungetch (int c);
    int get_inputline (FILE *fp);
    void set_inputline (const char* const line);


  enum {
    MAXTOK = 200,               // maximum length of a token
      MAXLINE = 1024,       // maximum line length
      MAXIDENT = 20,
      MAXSKIPWORD = 20,
      MAXSKIPCHAR = 20,
      MIN_INT = -2000000000,
      MAX_INT =  2000000000,
  };

  // token types
  enum {
    TT_STRING = 1,  // string token
      TT_INT,         // integer token
      TT_REAL,                  // floating point token
      TT_ALPHA,                 // alphabetic token
      TT_ALPNUM,                // alphanumeric token
      TT_NUMALPHA,
      TT_SPECLCHAR,
      TT_EOF,         // end of file reached
      TT_ERROR,       // error in token, caused by call to wrong type of token reader
      TT_BLANK,        // white space token.  pol_tok() skips these
      TT_USERTOK,     // user defined token
  };


private:

  // codes for pol_int and pol_float
  // if in reject catagory, get new number from terminal
  enum {
    P_FLTINT = 1,        // get a real or integer number
      P_BFLTINT,     // get a real or integer number, clip against bounds
      P_CBFLTINT,        // get real or int, reject if outside bounds
      P_FLT,               // get a real number
      P_BFLT,        // get a real, clip against bounds
      P_CBFLT,       // get a floating, reject if outside bounds
      P_INT,         // get a integer number
      P_BINT,        // get a integer, clip against bounds
      P_CBINT,       // get a integer, reject if outside bounds
  };

#define LETTER   'a'
#define DIGIT    '0'


//  typedef std::map<std::string,int> KeywordCodeList;

  struct token_st {
    int ready;                          // TRUE if token is ready
  //  std::string tokstr;       // token string
    char tokstr[MAXTOK+1];
    int type;                             // type of token 'TT_'
    int code;                             // holds code for user defined tokens
    double fnum;                        // real value of token
    int inum;                       // integer value of token
  };
  typedef struct token_st TOKEN;
  struct token_st token;                                // current token

  // Tables words stored with install() & found with lookup()
  KeywordCodeHashTable skiptable;               // words to ignore and skip
  KeywordCodeHashTable cmdtable;                // pol parameter commands
  KeywordCodeHashTable usertable;               // user defined symbols

  struct metachar {
    char eoc;           /* end of command character */
    char str;           /* string delimiter */
    char com;           /* comment character */
    char cmd;           /* pol parameter command character */
    char prg;           /* program load character */
    char con;           /* continuation across newline character */
    char out;           /* character that delimits output to terminal */
    char ter;           /* character indicates insertion of input from terminal */
    char inb;           /* input from graphics device */
  } meta;


  char m_szSkipChars [MAXSKIPCHAR]; // characters to skip
  bool m_bTrace;
  bool m_bNewlineIsEOC;

  struct KeywordCodeList {
    const char *keyword;
    int  code;
  };

  static const struct KeywordCodeList cmdlist[];
  static const unsigned int NUMCMD;

  // Internal codes for pol commands
  enum {
    PC_EOC = 1,
      PC_STR,
      PC_COM,
      PC_CMD,
      PC_PRG,
      PC_CON,
      PC_OUT,
      PC_TER,
      PC_INB,
      PC_NL_EOC,
      PC_NL_NEOC,
      PC_TRON,
      PC_TROFF,
      PC_FILE,
      PC_DUMP,
  };

  enum {
    INPUT_STREAM = 1,
    INPUT_FILE,
    INPUT_STRING,
  };

  int m_iInputType;
  std::istream* m_pInputStream;
  FILE* m_pInputFile;
  char* m_pszInputString;

  enum {
    MAXFILE = 8,
  };

  int currentf;         /* pointer to current fp */
  FILE *filep[MAXFILE];         /* == NULL for string input */
  char *fname[MAXFILE];         /* pointer to filename */

  char inputline[MAXLINE];              /* current input line */
  int lineptr;                  /* current position in inputline */

  std::stack<int> m_stackPushBackInput;

  bool skipSingleToken (char term[]);
  int tok (struct token_st *token);
  void dumptok (struct token_st *token);


  int getpol_tok (struct token_st *token);
  int getcmd ();
  int gettok (TOKEN *tok);
  void getblank (char *s, int toksiz);
  int getalpha (char *s, int toksiz);
  void getquote (char *qs, int toksiz);
  void getescape (char *s, int delim, int toksiz);
  int getnumber (char str[], int strsize, double *fnum, int *inum);
  void eatline ();
  int type (int c);
  int getch (FILE *fp);

};

#endif

