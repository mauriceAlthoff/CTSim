/*****************************************************************************
** FILE IDENTIFICATION
**
**   Name:          mpiworld.h
**   Purpose:       MPIWorld classes
**   Programmer:    Kevin Rosenberg
**   Date Started:  June 6, 2000
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

#include <mpi++.h>
#include <vector.h>
#include <string>

class MPIWorld
{
 public:
    MPIWorld (int& argc, char* const *& argv);

    void setTotalWorkUnits (int totalUnits);

    int getRank (void) const
        { return m_myRank; }

    int getNumProcessors (void) const
        { return m_nProcessors; }

    int getStartWorkUnit (int rank) const
        { return m_vStartWorkUnit [rank]; }

    int getEndWorkUnit (int rank) const
      { return m_vEndWorkUnit [rank]; }

    int getLocalWorkUnits (int rank) const
        { return m_vLocalWorkUnits [rank]; }

    int getMyStartWorkUnit (void) const
        { return m_vStartWorkUnit [m_myRank]; }

    int getMyEndWorkUnit (void) const
        { return m_vEndWorkUnit [m_myRank]; }

    int getMyLocalWorkUnits (void) const
        { return m_vLocalWorkUnits [m_myRank]; }

    MPI::Intracomm& getComm()
      { return m_comm; }

    void BcastString (string& str);

private:
    int m_myRank;
    int m_nProcessors;
    vector<int> m_vLocalWorkUnits;
    vector<int> m_vStartWorkUnit;
    vector<int> m_vEndWorkUnit;
    MPI::Intracomm m_comm;
};

