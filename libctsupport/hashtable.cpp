/*****************************************************************************
** FILE IDENTIFICATION
**
**   Hash Table Class
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


KeywordCodeEntry::KeywordCodeEntry (const char* const pszKeyword, int iCode)
    : m_iCode (iCode), m_pNext(NULL)
{
   int nLength = strlen (pszKeyword);
   char* pszCopy = new char [ nLength + 1];
   for (int i = 0; i < nLength; i++)
     pszCopy[i] = tolower (pszKeyword[i]);
   pszCopy[nLength] = 0;

   m_strKeyword = pszCopy;

   delete pszCopy;
}


bool
KeywordCodeEntry::matchesKeyword (const char* const pszCompare) const
{
    int nLength = strlen (pszCompare);
    char* pszCopy = new char [ nLength + 1];
    for (int i = 0; i < nLength; i++)
      pszCopy[i] = tolower (pszCompare[i]);
    pszCopy[nLength] = 0;

    bool bMatch = false;
    if (m_strKeyword.compare (pszCompare) == 0)
      bMatch = true;

    delete pszCopy;

    return bMatch;
}


// inittable (table)
//    clear symbol table

void
KeywordCodeHashTable::initTable ()
{
        int i;

        for (i = 0; i < HASHSIZE; i++)
            m_hashTable[i] = NULL;
}

// freetable (table)
//      free all memory allocated to table, then clear table

void
KeywordCodeHashTable::freeTable ()
{
        int i;
        KeywordCodeEntry *p, *np;

        for (i = 0; i < HASHSIZE; i++) {
            np = m_hashTable [i];
            while (np != NULL) {
                    p = np->getNext();
                    delete np;
                    np = p;
      }
        }
        initTable ();
}


// form hash value of string s
int
KeywordCodeHashTable::hash (const char* s)
{
        int hashval = 0;

  while (*s != EOS) {
            hashval += tolower(*s);
      s++;
  }

        return (hashval % HASHSIZE);
}


/* Look for s in hash table */
KeywordCodeEntry *
KeywordCodeHashTable::lookup (const char* const pszLookup)
{
    KeywordCodeEntry *found = NULL;
    for (KeywordCodeEntry* np = m_hashTable[ hash( pszLookup ) ]; np != NULL; np = np->getNext())
            if (np->matchesKeyword (pszLookup)) {
              found = np;               // found it
              break;
        }

  return (found);
}

void
KeywordCodeHashTable::installKeywordCode (const char* const pszKeyword, int iCode)
{
    KeywordCodeEntry *np = lookup (pszKeyword);

    if (np == NULL) {       // not found
            np = new KeywordCodeEntry (pszKeyword, iCode);
            int hashval = hash (np->getKeyword());
        np->setNext (m_hashTable[ hashval ]);
            m_hashTable[hashval] = np;
    } else                                      // already defined
            np->setCode (iCode);
}
