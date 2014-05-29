/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          mpiworld.cpp
**   Purpose:       MPI Support class
**   Programmer:    Kevin Rosenberg
**   Date Started:  June 2000
**
**  This is part of the CTSim program
**  Copyright (C) 1983-2009 Kevin Rosenberg
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

#include <mpi++.h>
#include <mpiworld.h>


MPIWorld::MPIWorld (int& argc, char* const *& argv)
{
  MPI::Init (argc, const_cast<char**&>(argv));
  m_comm = MPI::COMM_WORLD.Dup();
  m_nProcessors = m_comm.Get_size();
  m_myRank = m_comm.Get_rank();
  m_vLocalWorkUnits.reserve (m_nProcessors);
  m_vStartWorkUnit.reserve (m_nProcessors);
  m_vEndWorkUnit.reserve (m_nProcessors);
}


void
MPIWorld::setTotalWorkUnits(int totalWorkUnits)
{
  if (m_nProcessors < 1)
      return;

  int baseLocalWorkUnits = totalWorkUnits / m_nProcessors;
  int remainderWorkUnits = totalWorkUnits % m_nProcessors;

  int currWorkUnits = 0;
  for (int iProc = 0; iProc < m_nProcessors; iProc++) {
    m_vLocalWorkUnits[iProc] = baseLocalWorkUnits;
    if (iProc < remainderWorkUnits)
      m_vLocalWorkUnits[iProc]++;

    m_vStartWorkUnit[iProc] = currWorkUnits;
    m_vEndWorkUnit[iProc] = m_vStartWorkUnit[iProc] + m_vLocalWorkUnits[iProc]  - 1;

    currWorkUnits += m_vLocalWorkUnits[iProc];
  }

}

void
MPIWorld::BcastString (string& str)
{
  int len;

  if (m_myRank == 0)
    len = str.length();
  m_comm.Bcast (&len, 1, MPI::INT, 0);
  char buf [ len + 1];

  if (m_myRank == 0)
    strcpy (buf, str.c_str());

  m_comm.Bcast (buf, len + 1, MPI::CHAR, 0);

  if (m_myRank > 0)
    str = buf;
}
