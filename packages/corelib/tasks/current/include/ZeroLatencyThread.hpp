/***************************************************************************
 tag: Peter Soetens  Wed Apr 17 16:01:36 CEST 2002  ZeroTimeThread.h 

                       ZeroLatencyThread.h -  description
                          -------------------
   begin                : Wed April 17 2002
   copyright            : (C) 2002 Peter Soetens
   email                : peter.soetens@mech.kuleuven.ac.be

   ***************************************************************************
   *                                                                         *
   *   This program is free software; you can redistribute it and/or modify  *
   *   it under the terms of the GNU General Public License as published by  *
   *   the Free Software Foundation; either version 2 of the License, or     *
   *   (at your option) any later version.                                   *
   *                                                                         *
   ***************************************************************************/



#ifndef ZEROLATENCYTHREAD_HPP
#define ZEROLATENCYTHREAD_HPP

#include "TaskExecution.hpp"


namespace ORO_CoreLib
{

    class TaskPreemptible;
    /**
     * This thread is the preemptible realtime thread in the
     * Orocos system. It is assumed to perform its work over a 
     * low (zero) latency timespan.
     * It Uses the Singleton pattern, since there will be only one.
     *
     * @see ComponentThreaded, ZeroTimeThread
     */
    class ZeroLatencyThread
        : public TaskExecution
    {
    public:
        static ZeroLatencyThread* Instance();

        /**
         * Releases the ZeroLatencyThread
         * Reference counting might aid in making this call safe
         *
         * @return true on success, false on failure
         */
        static bool Release();

        /**
         * Destructor
         */
        virtual ~ZeroLatencyThread();

        /**
         * Add a TaskPreemptible which is handled each n nanoseconds
         * 
         * @param t The task to handle each n nanoseconds
         * @param n handle every n nanoseconds
         */
        bool taskAdd( TaskPreemptible* t, const nsecs n );

        /**
         * Remove a TaskPreemptible from handleing
         *
         * @post <t> is no longer handled by this thread
         */
        void taskRemove( TaskPreemptible* t );
    protected:

        /**
         * Constructor
         */
        ZeroLatencyThread();

    private:

        /**
         * Our only instance of the ZeroLatencyThread
         */
        static ZeroLatencyThread* _instance;

    };
} // namespace ORO_CoreLib

#endif
