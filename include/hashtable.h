/* FILE IDENTIFICATION
**
**      File Name:      hashtable.h
**      Author:         Kevin Rosenberg
**      Purpose:        Header file for hash table library
**      Date Started:   Dec. 2000
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

#ifndef HASHTABLE_H
#define HASHTABLE_H


class KeywordCodeEntry
{
private:
  std::string m_strKeyword;
  int m_iCode;
  class KeywordCodeEntry *m_pNext;

  public:

    KeywordCodeEntry (const char* const pszKeyword, int iCode);

    const char* const getKeyword() const
    { return m_strKeyword.c_str(); }

    bool matchesKeyword (const char* const pszMatch) const;

    int getCode () const
    { return m_iCode; }

    void setCode (int iCode)
    { m_iCode = iCode; }

    void setNext (KeywordCodeEntry* pNext)
    { m_pNext = pNext; }

    KeywordCodeEntry* getNext ()
    { return m_pNext; }
};


class KeywordCodeHashTable {
public:
  enum {
    HASHSIZE = 100,
  };

  KeywordCodeHashTable()
  { initTable(); }

  ~KeywordCodeHashTable()
  { freeTable(); }

  void installKeywordCode (const char* const pszKeyword, int iCode);
  KeywordCodeEntry* lookup (const char* const pszKeyword);

private:
  KeywordCodeEntry* m_hashTable[HASHSIZE];

  int hash (const char* s);
  void initTable ();
  void freeTable ();
};

#endif


