/*****************************************************************************
** FILE IDENTIFICATION
**
**      Name:         timer.h
**      Purpose:      Header file for Timer class
**      Author:       Kevin Rosenberg
**      Date Started: Sep 2000
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

#ifndef _TIMER_H
#define _TIMER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef MSVC
#include <sys/timeb.h>
#endif

class Timer
{
 public:
    Timer (void)
        { m_timeStart = ttime(); }

    virtual ~Timer (void)
        {}

    virtual double timerEnd (void)
      {
        m_timeEnd = ttime();
        m_timeElapsed = m_timeEnd - m_timeStart;

        return (m_timeElapsed);
      }

    virtual void timerReport (const char* const msg) const
      {
                std::cout << msg << ": " << m_timeElapsed << " seconds" << std::endl;
      }

    virtual double timerEndAndReport (const char* const msg)
      {
        double t = timerEnd ();
        timerReport (msg);
        return (t);
      }

    double getTimeElapsed (void) const
        { return m_timeElapsed; }

 protected:
    double m_timeStart;
    double m_timeEnd;
    double m_timeElapsed;

    double ttime(void) const
        {
#ifdef HAVE_GETTIMEOFDAY
            struct timeval now;
            if (gettimeofday (&now, NULL))
                return 0;

            return (now.tv_sec + static_cast<double>(now.tv_usec) / 1000000.);
#elif defined(MSVC)
                struct _timeb now;
                _ftime (&now);
                return (now.time + static_cast<double>(now.millitm) / 1000.);
#else
            return 0;
#endif
        }
};


#ifdef HAVE_MPI

#include "mpi++.h"

class TimerMPI : public Timer
{
 public:
    TimerMPI (MPI::Intracomm& comm)
        : m_comm(comm)
      {
          m_timeStart = MPI::Wtime();
      }

    virtual ~TimerMPI (void)
      {}

    virtual double timerEnd (void)
      {
        m_timeEnd = MPI::Wtime();
        m_timeElapsed = m_timeEnd - m_timeStart;

        return (m_timeElapsed);
      }

    virtual void timerReport (const char* const msg)
      {
          if (m_comm.Get_rank() == 0)
                  std::cout << msg << ": " << m_timeElapsed << " seconds" << std::endl;
      }

    virtual double timerEndAndReport (const char* const msg)
      {
        double t = timerEnd ();
        timerReport (msg);
        return (t);
      }

    virtual void timerReportAllProcesses (const char* const msg)
      {
          timerReport (msg);
      }

 protected:
    MPI::Intracomm& m_comm;
};

class TimerCollectiveMPI : public TimerMPI
{
 public:
    TimerCollectiveMPI (MPI::Intracomm& comm)
        : TimerMPI::TimerMPI (comm)
      {
        m_comm.Barrier();
        m_timeStart = MPI::Wtime();
      }

    virtual ~TimerCollectiveMPI (void)
      {}

    virtual double timerEnd (void)
      {
        m_timeEnd = MPI::Wtime();
        m_timeElapsed = m_timeEnd - m_timeStart;
        m_comm.Reduce (&m_timeElapsed, &m_timeMin, 1, MPI::DOUBLE, MPI::MIN, 0);
        m_comm.Reduce (&m_timeElapsed, &m_timeMax, 1, MPI::DOUBLE, MPI::MAX, 0);

        return (m_timeElapsed);
      }

    virtual double timerEndAndReport (const char* const msg)
      {
        double t = timerEnd ();
        timerReport (msg);
        return (t);
      }

    virtual void timerReport (const char* const msg)
      {
        if (m_comm.Get_rank() == 0)
                std::cout << msg << " " << "Minimum=" << m_timeMin << ", Maximum=" << m_timeMax << " seconds" << std::endl;
      }

    virtual void timerReportAllProcesses (const char* const msg)
      {
                std::cout << msg << ": " << "Minimum=" << m_timeMin << ", Maximum=" << m_timeMax << " seconds (Rank " << m_comm.Get_rank() << ")" << std::endl;
      }

 private:
    double m_timeMin;
    double m_timeMax;
};
#endif


#endif  // _TIMER_H


