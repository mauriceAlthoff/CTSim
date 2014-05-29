/*****************************************************************************
** FILE IDENTIFICATION
**
**   POL - Problem Oriented Language
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

#include "ct.h"
#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include "ctsupport.h"
#include "pol.h"


const struct POL::KeywordCodeList POL::cmdlist[] = {
  {     "eoc",  PC_EOC,},
  {     "str",  PC_STR,},
  {     "com",  PC_COM,},
  {     "cmd",  PC_CMD,},
  {     "prg",  PC_PRG,},
  {     "con",  PC_CON,},
  {     "out",  PC_OUT,},
  {     "ter",  PC_TER,},
  {     "inb",  PC_INB,},

  {     "nl_eoc",PC_NL_EOC,},
  {     "nl_neoc", PC_NL_NEOC,},
  {     "tron",  PC_TRON,},
  {     "troff", PC_TROFF,},
  {     "file",  PC_FILE,},
  {     "dump",  PC_DUMP,},
};

const unsigned int POL::NUMCMD = (sizeof(POL::cmdlist) / sizeof (struct POL::KeywordCodeList));


POL::POL()
{
  currentf = -1;
  m_bTrace = false;
  init();
}

POL::~POL()
{
}

void
POL::init ()
{
  meta.eoc    = SEMICOL;
  meta.str    = DQUOTE;
  meta.com    = SHARP;
  meta.cmd    = EXCLAM;
  meta.prg    = ATSIGN;
  meta.con    = AMPERSAND;
  meta.out    = DOLLAR;
  meta.ter    = PERCENT;
  meta.inb    = LBRACK;

  m_bNewlineIsEOC = true;
  m_szSkipChars[0] = EOS;


  for (unsigned int i = 0; i < NUMCMD; i++)
    cmdtable.installKeywordCode (cmdlist[i].keyword, cmdlist[i].code);

  token.ready = false;          // no token read yet
}


/* addSkipWord (w)
*
* char *w - word for pol to ignore and skip over in input
*
* tok() compares all tokens to words given to this routine. If it finds it,
* it will immediately read another token.
*/

void
POL::addSkipWord (const char* const w)
{
  skiptable.installKeywordCode (w, 0);
}

/* skpchar (s)
*
* skip all characters that appear in string s
*/
void
POL::addSkipChar (int c)
{
  int n = strlen (m_szSkipChars);
  if (n < MAXSKIPCHAR) {
    m_szSkipChars[n] = c;
    m_szSkipChars[n+1] = 0;
  }
}

// installKeyword (str, code)
//
// char *str - token string to install
// int code  - code to return for token
//
// tok() looks for these user defined tokens.  If it finds one,
// it stores the tokens code in the token structure and returns TT_USERTOK
void
POL::addKeyword (const char* const str, int code)
{
  usertable.installKeywordCode (str, code);
}

/* get_word - matches tokens on a letter by letter basis
*
* char *search - string to search for
* int  nlet     - maximum number of chars to search for match
*/

bool
POL::readWord (const char *search, int nlet)
{
  tok (&token);
  if (m_bTrace)
    sys_error (ERR_TRACE, "POL matching current token %s against word %s\n", token.tokstr, search);

  if (strncasecmp (search, token.tokstr, nlet) == 0) {
    dumptok (&token);
    return (true);
  } else
    return (false);
}

/* usertok (str,code)
*       see if current token is a user defined token set with install()
*
*    char *str - token string as read from input
*    int *code - returned code for user defined symbol
*    return value - true if current token has been user defined
*                    false if current token is not user defined
*/
bool
POL::readUserToken (char *str, int *code)
{
  tok (&token);

  if (m_bTrace)
    sys_error (ERR_TRACE, "POL checking if current token '%s' is user defined\n", token.tokstr);

  if (token.type == TT_USERTOK) {
    *code = token.code;
    strcpy (str, token.tokstr);
    dumptok (&token);
    return (true);
  } else {
    *code = 0;
    return (false);
  }
}

/* isstring (s) - returns true if current token is a string
*
* char *s - pointer to place to store token string
*/

bool
POL::readString (char *str)
{
  tok (&token);

  if (token.type == TT_STRING) {
    strcpy (str, token.tokstr);
    dumptok (&token);
    return (true);
  } else
    return (false);
}

/* integer - test for an integer
*
* int *n:       returned integer value
* int typecode = TT_INT if accept only integer values
*               = TT_REAL if accept both real and integer values
* int boundcode= true if force to lie between boundries
*               = false can take any value it likes
* int bb1:      lower bound
* int bb2:      upper bound
*/
bool
POL::readInteger (int *n, int typecode, bool boundcode, int bb1, int bb2)
{
  tok (&token);

  if (m_bTrace)
    sys_error (ERR_TRACE, "POL checking if current token %s is an integer\n", token.tokstr);

  if (token.type == TT_INT || token.type == TT_REAL) {
           if (boundcode) {
       if (token.inum < bb1)
         *n = bb1;
       else if (token.inum > bb2)
         *n = bb2;
       else
         *n = token.inum;
            } else
        *n = token.inum;
      dumptok (&token);
      return (true);
  }
  *n = 0;
  return (false);
}

bool
POL::readFloat (double *n, double typecode, bool boundcode, double bb1, double bb2)
{
  tok (&token);

  if (m_bTrace)
    sys_error (ERR_TRACE, "POL checking if current token %s is an floating point number\n", token.tokstr);

  if (token.type == TT_INT || token.type == TT_REAL) {
           if (boundcode) {
       if (token.fnum < bb1)
         *n = bb1;
       else if (token.fnum > bb2)
         *n = bb2;
       else
         *n = token.fnum;
            } else
        *n = token.fnum;
      dumptok (&token);
      return (true);
  }
  *n = 0.0;
  return (false);
}

/*----------------------------------------------------------------------*/
/* skip() - skip over any token except for end of command sequence      */
/*                                                                      */
/*              returns true if succesful skip                          */
/*              returns false if already at end of command or EOF       */
/*----------------------------------------------------------------------*/

bool
POL::skipTokens()
{
  char term[5];         /* string of characters not to skip */

  term[0] = meta.eoc;
  if (m_bNewlineIsEOC) {
    term[1] = NEWLINE;
    term[2] = EOS;
  } else
    term[1] = EOS;

  return (skipSingleToken (term));
}

void
POL::reader()
{
  while (skipTokens())
    ;

  dumptok (&token);             /* skip end of command token */
}

/* skiptok (term) - skip a token unless the first character of a token is
*                   in the string of terminators, term.
* char *term - string of termination characters, don't skip these characters
*               skiptok() also does NOT skip TT_EOF
* returns (true) if succesful skip of a token
* returns (false) if didn't skip, read termination character or TT_EOF
*/

bool
POL::skipSingleToken (char term[])
{
  tok (&token);

  if (token.type == TT_EOF
    || (token.type == TT_SPECLCHAR && strchr(term, token.tokstr[0]) != NULL))
    return (false);
  else {
    dumptok (&token);
    return (true);
  }
}

int
POL::tok (struct token_st *token)
{
  if (token->ready == false)
    getpol_tok(token);
  else
    if (token->type == TT_EOF && lookchar() != EOF)
      getpol_tok(token);
    return (token->type);
}

void
POL::dumptok (struct token_st *token)
{
  if (token->ready == false)
    getpol_tok(token);
  token->ready = false;
}

int
POL::getpol_tok (struct token_st *token)
{
  KeywordCodeEntry* sym;

  token->ready = false;
nexttok:
  gettok (token);

  if (token->type == TT_BLANK)
    goto nexttok;
  if (token->type == TT_SPECLCHAR) {
    if (strchr(m_szSkipChars, token->tokstr[0]) != NULL)
      goto nexttok;
    if (token->tokstr[0] == NEWLINE)
      goto nexttok;
    if (token->tokstr[0] == meta.cmd) {
      getcmd();
      goto nexttok;
    }
    if (token->tokstr[0] == meta.com) {         /* skip comment */
      eatline ();
      goto nexttok;
    }
    if (token->tokstr[0] == meta.out) {
      getescape(token->tokstr, meta.out, MAXTOK);
      fputs (token->tokstr, stderr);
      goto nexttok;
    }
    if (token->tokstr[0] == meta.con) {         /* continuation across NEWLINE */
      while (lookchar() == BLANK || lookchar() == TAB)
        inchar();
      if (lookchar() == NEWLINE)
        inchar();
    }
    if (token->tokstr[0] == meta.ter) {         /* get input from terminal */
      usefile (P_USE_FILE, "");
      tok (token);
      closefile();
      return (token->type);
    }
  }

  /* look for filler words */

  if (skiptable.lookup (token->tokstr) != NULL) /* ignore words in skip table */
    goto nexttok;

  /* look for user defined symbols */

  if ((sym = usertable.lookup (token->tokstr)) != NULL) {
    token->type = TT_USERTOK;
    token->code = sym->getCode();
  } else
    token->code = 0;

  if (m_bTrace)
    sys_error (ERR_TRACE, "POL read token '%s', type = %d\n", token->tokstr, token->type);

  return (token->type);
}


int
POL::getcmd()
{
  int tt, found;
  char str[MAXTOK+1];
  KeywordCodeEntry *cmd;
  TOKEN tok;

  tt = getalpha (str, MAXTOK);
  if (tt == TT_ERROR) {
    sys_error (ERR_WARNING, "Error in POL parameter command");
    reader();
    return(false);
  }
  if ((cmd = cmdtable.lookup (str)) == NULL) {
    sys_error  (ERR_WARNING, "POL: Unrecognized command %s", cmd);
    reader();
    return (false);
  } else {
    found = false;
    switch (cmd->getCode()) {
    case PC_TRON:
                    m_bTrace = true;
        found = true;
        break;
    case PC_TROFF:
                    m_bTrace = false;
        found = true;
        break;
    case PC_FILE:
                    found = true;
        tt = gettok (&tok);
        usefile (P_USE_FILE, tok.tokstr);
        break;
    case PC_NL_EOC:
                    found = true;
        m_bNewlineIsEOC = true;
        break;
    case PC_NL_NEOC:
                    found = true;
        m_bNewlineIsEOC = false;
        break;
    case PC_DUMP:
                    found = true;
        printf("eoc = %c  str = %c  com = %c  cmd = %c  prg = %c\n",
          meta.eoc, meta.str, meta.com, meta.cmd, meta.prg);
        printf("con = %c  out = %c  ter = %c  inb = %c\n",
          meta.con, meta.out, meta.ter, meta.inb);
        break;
    }
    if (found == false) {
      tt = gettok (&tok);
      if (tt != TT_SPECLCHAR) {
        sys_error (ERR_SEVERE, "POL: Illegal command character");
        return (false);
      }
      switch(cmd->getCode()) {
                    case PC_EOC:
          meta.eoc = tok.tokstr[0];
          break;
        case PC_STR:
          meta.str = tok.tokstr[0];
          break;
        case PC_COM:
          meta.com = tok.tokstr[0];
          break;
        case PC_CMD:
          meta.cmd = tok.tokstr[0];
          break;
        case PC_PRG:
          meta.prg = tok.tokstr[0];
          break;
        case PC_CON:
          meta.con = tok.tokstr[0];
          break;
        case PC_OUT:
          meta.out = tok.tokstr[0];
          break;
        case PC_TER:
          meta.ter = tok.tokstr[0];
          break;
        case PC_INB:
          meta.inb = tok.tokstr[0];
          break;
        default:
          printf("command not implemented\n");
          break;
      }                         /* switch (tok->type) */
    }                                   /* if (found == false) */
    reader();                   /* clean up command */
  }                                     /* if legal command */

  return (true);
}


int
POL::gettok (TOKEN *tok)
{
  int c, toktype;
  int inum;
  double fnum;
  int toksiz = MAXTOK;          /* maximum length of token string */

  while ((c = inchar()) == BLANK || c == TAB)
    ;
  ungetch (c);

  c = lookchar();
  toktype = type(c);

  fnum = 0.0;
  inum = 0;

  if (c == BLANK || c == TAB) {                 /* skip white space */
    getblank(tok->tokstr, toksiz);
    toktype = TT_BLANK;
  } else if (toktype == LETTER) {
    toktype = getalpha (tok->tokstr, toksiz);
  } else if (c == meta.str) {                   /* quoted string */
    getquote (tok->tokstr, toksiz);
    toktype = TT_STRING;
  } else if (type(c) == DIGIT || c == PLUS || c == HYPHEN || c == PERIOD) {
    toktype = getnumber (tok->tokstr, toksiz, &fnum, &inum);
  } else if (c == EOF) {
    tok->tokstr[0] = EOS;
    toktype = TT_EOF;
  } else {
    c = inchar();
    tok->tokstr[0] = c;
    tok->tokstr[1] = EOS;
    toktype = TT_SPECLCHAR;
  }

  tok->type = toktype;
  tok->ready = true;
  if (tok->type == TT_REAL || tok->type == TT_INT) {
    tok->fnum = fnum;
    tok->inum = inum;
  } else {
    tok->fnum = 0.0;
    tok->inum = 0;
  }

  return (toktype);
}


void
POL::getblank (char *s, int toksiz)
{
  int c;

  while ((c = inchar()) == BLANK || c == TAB)
    ;
  ungetch(c);

  s[0] = BLANK;
  s[1] = EOS;
}


int
POL::getalpha (char *s, int toksiz)
{
  int i, chartype, alphatype;

  if (type(lookchar()) != LETTER) {
    s[0] = EOS;
    return (TT_ERROR);
  }

  alphatype = TT_ALPHA;
  for (i = 0; i < toksiz; i++) {                /* get alphanumeric token */
    s[i] = inchar();
    chartype = type (s[i]);
    if (chartype != LETTER && chartype != DIGIT)
      break;
    if (chartype == DIGIT)
      alphatype = TT_ALPNUM;
  }
  ungetch(s[i]);

  if (i >= toksiz)
    sys_error (ERR_SEVERE, "POL token too long.");

  s[i] = EOS;                   /* terminate token */
  return (alphatype);
}


/* getquote - get quoted string from file */
/* have already gotten delimiter in qs[0] */
void
POL::getquote (char *qs, int toksiz)
{
  int delim;

  delim = inchar();                     /* char = delimiter */
  getescape(qs, delim, toksiz);
}


void
POL::getescape (        /* reads up to delim */
                char *s,
                int delim,
                int toksiz
                )
{
  int i, c;

  for (i = 0; (c = inchar()) != delim; i++) {
    if (c == NEWLINE) {
      sys_error (ERR_WARNING, "Missing closing delimiter.");
      break;
    }
    if (i >= toksiz) {
      sys_error (ERR_SEVERE, "string too long.");
      break;
    }
    if (c == EOF) {
      ungetch(c);
      sys_error (ERR_SEVERE, "end of file inside quotation");
      break;
    } else if (c == BSLASH) {   /* escape character */
      s[i++] = c;
      c = inchar();             /* get escaped character */
    }
    s[i] = c;
  }
  s[i] = EOS;
}


bool
POL::readText (char *str, int lim)
{
  int c;
  while ((c = inchar()) == BLANK || c == TAB)
    ;
  ungetch (c);
  if (c == EOF) {
    str[0] = 0;
    return false;
  }

  int i;
  for (i = 0; i < lim && (c = inchar()) != EOF && c != NEWLINE; i++)
    str[i] = c;
  ungetch (c);
  str[i] = 0;

  return true;
}

//----------------------------------------------
// Get a number for gettok()
//----------------------------------------------

int
POL::getnumber
(
 char str[],                            /* string to return token in */
 int strsize,                           /* maximum length of token string */
 double *fnum,                          /* floating point value of number read */
 int *inum                              /* integer value of number read */
 )
{
  int sp = 0;
  double sign = 1.0;
  bool isSigned = false;                /* true if number prefixed by '+' or '-' */
  *fnum = 0.0;
  *inum = 0;
  str[0] = EOS;

  int c = inchar();
  if (c == HYPHEN) {
    str[sp++] = c;
    isSigned = true;
    sign = -1.0;
  } else if (c == PLUS) {
    str[sp++] = c;
    isSigned = true;
    sign = 1.0;
  } else if (c == PERIOD) {
    if (type(lookchar()) != DIGIT) {
      str[0] = PERIOD;
      str[1] = EOS;
      return (TT_SPECLCHAR);
    } else
      ungetch (PERIOD);
  } else if (type(c) != DIGIT) {
    ungetch (c);
    return (TT_ERROR);
  } else
    ungetch (c);

  if (isSigned) {
    c = lookchar();
    if (c == PERIOD) {
      inchar();         /* get period */
      c = lookchar();           /* look at character past period */
      ungetch (PERIOD); /* put back period */
      if (type(c) != DIGIT) {
        str[sp] = EOS;
        return (TT_SPECLCHAR);
      }
    } else if (type (c) != DIGIT) {
      str[sp] = EOS;
      return (TT_SPECLCHAR);
    }
  }

  double whole = 0.0;
  while (type(c = inchar()) == DIGIT) {
    if (sp < strsize)
      str[sp++] = c;
    whole = 10.0 * whole + (c - '0');
  }
  ungetch (c);          /* put back non-numeric character */

  if (c != PERIOD && tolower(c) != 'e') {
    str[sp] = EOS;
    *fnum = whole * sign;
    if (*fnum < MIN_INT)
      *inum = MIN_INT;
    else if (*fnum > MAX_INT)
      *inum = MAX_INT;
    else
      *inum = (int) *fnum;
    return (TT_INT);
  }

  if (lookchar() == PERIOD) {
    inchar();
    if (sp < strsize)
      str[sp++] = PERIOD;
  }

  double frac = 0.0;
  double powerof10 = 10.0;

  while (type(c = inchar()) == DIGIT) {
    if (sp < strsize)
      str[sp++] = c;
    frac += (double) (c - '0') / powerof10;
    powerof10 *= 10.0;
  }
  ungetch (c);

  double exp = 0.0;
  double expsign = 1.0;
  c = inchar();
  if (tolower(c) != 'e')
    ungetch (c);
  else {
    if (sp < strsize)
      str[sp++] = c;
    if ((c = inchar()) == PLUS) {
      if (sp < strsize)
        str[sp++] = c;
      expsign = 1.0;
    } else if (c == HYPHEN) {
      if (sp < strsize)
        str[sp++] = c;
      expsign = -1.0;
    } else if (type(c) != DIGIT) {
      --sp;                             /* erase 'e' */
      ungetch (c);
      ungetch ('e');
      goto getnumexit;
    } else
      ungetch(c);

    exp = 0;
    while (type(c = inchar()) == DIGIT) {
      if (sp < strsize)
        str[sp++] = c;
      exp = 10 * exp + (c - '0');
    }
    ungetch (c);
  }

getnumexit:
  str[sp] = EOS;
  *fnum = sign * (whole + frac) * pow (10.0, expsign * exp);
  if (*fnum < MIN_INT)
    *inum = MIN_INT;
  else if (*fnum > MAX_INT)
    *inum = MAX_INT;
  else
    *inum = (int) *fnum;
  return (TT_REAL);
}

void
POL::eatline ()
{
  char term [2];

  term[0] = NEWLINE;
  term[1] = EOS;
  skipSingleToken (term);
}

// return type of ASCII character
int
POL::type (int c)
{
  if (isalpha(c) || c == UNDERLIN)
    return (LETTER);
  else if (isdigit(c))
    return (DIGIT);
  else
    return (c);
}


//----------------------------------------------------------------------
//                              POL INPUT
//----------------------------------------------------------------------


/* usefile - set source of POL input
*
*    int source - source of input
*                  P_USE_STR  - have POL use strings as input
*                  P_USE_FILE - use file.  filename is in str
*
*/

void
POL::usefile (int source, const char *fn)
{
  FILE *fp;

  ++currentf;
  if (currentf >= MAXFILE) {
    --currentf;
    sys_error (ERR_SEVERE, "files nested too deeply");
    return;
  }

  while (! m_stackPushBackInput.empty())
    m_stackPushBackInput.pop();

  if (source == P_USE_STR) {
    filep[currentf] = NULL;
  } else if (source == P_USE_FILE) {
    if (fn == NULL || strlen(fn) == 0) {
      fp = stdin;
    } else if ((fp = fopen(fn, "r")) == NULL) {
      --currentf;
      sys_error (ERR_SEVERE, "can't open file");
      return;
    }
    filep[currentf] = fp;
    fname[currentf] = strdup (fn);
  }
}

void
POL::closefile()
{
  if (currentf >= 0) {
    if (filep[currentf] != NULL)
      fclose (filep[currentf]);
    --currentf;
  }
}

/*-----------------------------*/
/* Lowest Level Input Routines */
/*-----------------------------*/


int
POL::lookchar()
{
  int c;

  c = inchar();
  ungetch (c);
  return (c);
}

int
POL::inchar()
{
  int c = 0;

  if (currentf < 0)
    return (EOF);

  while (currentf >= 0 && (c = getch(filep[currentf])) == EOF && filep[currentf] != NULL) {
    closefile ();
  }

  return (c);
}

/*--------------------------------------------------------------*/
/* getch - get a (possibly pushed back) character               */
/*         if fp == NULL, then get character from inputline     */
/*--------------------------------------------------------------*/

int
POL::getch (FILE *fp)
{
  int c;
  if (m_stackPushBackInput.size() > 0) {
    c = m_stackPushBackInput.top();
    m_stackPushBackInput.pop();
    return c;
  }

  if (fp == NULL) {
    if ((c = inputline[lineptr]) == EOS)
      return (EOF);
    else {
      ++lineptr;
      return (c);
    }
  } else
    c = fgetc(fp);

  return (c);
}

// push character back on input
void
POL::ungetch (int c)
{
  m_stackPushBackInput.push (c);
}


int
POL::get_inputline (FILE *fp)
{
  while (! m_stackPushBackInput.empty())
    m_stackPushBackInput.pop();

  lineptr = 0;
  if (fgets (inputline, MAXLINE, fp) == NULL)
    return (EOF);
  else
    return (OK);
}

void
POL::set_inputline (const char* const line)
{
  while (! m_stackPushBackInput.empty())
    m_stackPushBackInput.pop();

  strncpy (inputline, line, MAXLINE);
  lineptr = 0;
}
