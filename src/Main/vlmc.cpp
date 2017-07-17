/*****************************************************************************
 * vlmc.cpp: VLMC launcher for unix
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Main/Core.h"
#include "Workflow/Types.h"
#include "Tools/VlmcDebug.h"
#include "Project/Project.h"

#include <QMetaType>
#include <QTextStream>
#include <QVariant>

#include <sys/wait.h>
#include <unistd.h>


int VLMCmain( int , char** );

#if defined(WITH_CRASHHANDLER) && defined(Q_OS_UNIX)

#ifdef HAVE_GUI
    #ifdef WITH_CRASHHANDLER_GUI
        #include "Gui/widgets/CrashHandler.h"
    #endif
#else
    #include "ProjectManager.h"
#endif

void
signalHandler( int sig )
{
    signal( sig, SIG_DFL );

    Core::instance()->project()->emergencyBackup();

    #ifdef WITH_CRASHHANDLER_GUI
        CrashHandler* ch = new CrashHandler( sig );
        ::exit( ch->exec() );
    #else
        ::exit( 1 );
    #endif
}
#endif

int
main( int argc, char **argv )
{
    #ifdef WITH_CRASHHANDLER
        while( true )
        {
            pid_t pid = fork();
            if( pid < 0 )
                vlmcFatal("Can't fork to launch VLMC. Exiting.");

            /* We're in the crash handler process */
            if( pid != 0 )
            {
                int status;

                wait( &status );
                if( WIFEXITED(status) )
                {
                    int ret = WEXITSTATUS( status );
                    if ( ret == 2 )
                        continue ;
                    else
                        return ret ;
                }
                else
                {
                    vlmcCritical() << "Unhandled crash.";
                    return 1;
                }
            }
            else /* We're actually in the program */
            {
                signal( SIGSEGV, signalHandler );
                signal( SIGFPE, signalHandler );
                signal( SIGABRT, signalHandler );
                signal( SIGILL, signalHandler );
                break; /* Run it */
            }
        }
    #endif

    return VLMCmain( argc, argv );
}
