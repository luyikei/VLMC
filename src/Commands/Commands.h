/*****************************************************************************
 * Commands.h: Contains all the implementation of VLMC commands.
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzee-Luyssen <hugo@vlmc.org>
 *          Ludovic Fauvet <etix@l0cal.com>
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

#ifndef COMMANDS_H
#define COMMANDS_H

#include "config.h"
#ifdef WITH_GUI
# include <QUndoCommand>
#endif
#include <QObject>
#include <QVector>
#include "MainWorkflow.h"
#include "Types.h"

class   Clip;
class   ClipHelper;
class   EffectUser;

#define NEW_COMMAND(x)      class   x : public Generic

namespace Commands
{
#ifdef WITH_GUI
    class       Generic : public QUndoCommand
#else
    class       Generic
#endif
    {
        public:
            virtual void    redo() = 0;
            virtual void    undo() = 0;
#ifndef WITH_GUI
            virtual void    setText( const QString& ) {}
#endif
    };

#ifdef WITH_GUI
    void        trigger( QUndoCommand* command );
#else
    void        trigger( Generic* command );
#endif

    namespace   Clip
    {
        NEW_COMMAND( Add )
        {
            public:
                Add( ClipHelper* ch, TrackWorkflow* tw, qint64 pos );
                virtual ~Add();
                virtual void    redo();
                virtual void    undo();
            private:
                ClipHelper                  *m_clipHelper;
                TrackWorkflow               *m_trackWorkflow;
                qint64                      m_pos;
        };

        NEW_COMMAND( Move )
        {
            public:
                Move( TrackWorkflow *oldTrack, TrackWorkflow *newTrack,
                        ClipHelper *clipHelper, qint64 newPos );
                virtual void    redo();
                virtual void    undo();

            private:
                TrackWorkflow   *m_oldTrack;
                TrackWorkflow   *m_newTrack;
                ClipHelper      *m_clipHelper;
                qint64          m_newPos;
                qint64          m_oldPos;
        };

        NEW_COMMAND( Remove )
        {
            public:
                Remove( ClipHelper* clip, TrackWorkflow* tw );
                virtual void redo();
                virtual void undo();

            private:
                ClipHelper                  *m_clipHelper;
                TrackWorkflow               *m_trackWorkflow;
                qint64                      m_pos;
        };

        /**
         *  \brief  This command is used to resize a clip.
         *  \param  newBegin: The clip's new beginning
         *  \param  newEnd: The clip's new end
         *  \param  newPos: if the clip was resized from the beginning, it is moved
         *                  so we have to know its new position
        */
        NEW_COMMAND( Resize )
        {
            public:
                Resize( TrackWorkflow* tw, ClipHelper* clipHelper,
                            qint64 newBegin, qint64 newEnd, qint64 newPos );
                virtual void    redo();
                virtual void    undo();
            private:
                TrackWorkflow*              m_trackWorkflow;
                ClipHelper*                 m_clipHelper;
                qint64                      m_newBegin;
                qint64                      m_newEnd;
                qint64                      m_oldBegin;
                qint64                      m_oldEnd;
                qint64                      m_newPos;
                qint64                      m_oldPos;
        };

        NEW_COMMAND( Split )
        {
            public:
                Split( TrackWorkflow *tw, ClipHelper* toSplit, qint64 newClipPos,
                           qint64 newClipBegin );
                ~Split();
                virtual void    redo();
                virtual void    undo();
            private:
                TrackWorkflow               *m_trackWorkflow;
                ClipHelper*                 m_toSplit;
                ClipHelper*                 m_newClip;
                qint64                      m_newClipPos;
                qint64                      m_newClipBegin;
                qint64                      m_oldEnd;
        };
    }
    namespace   Effect
    {
        NEW_COMMAND( Add )
        {
            public:
                Add( EffectHelper *helper, EffectUser *target );
                virtual void    redo();
                virtual void    undo();
            private:
                EffectHelper    *m_helper;
                EffectUser      *m_target;
        };

        NEW_COMMAND( Move )
        {
            public:
                Move( EffectHelper *helper, EffectUser *old, EffectUser *newUser, qint64 pos );
                virtual void    redo();
                virtual void    undo();
            private:
                EffectHelper    *m_helper;
                EffectUser      *m_old;
                EffectUser      *m_new;
                qint64          m_oldPos;
                qint64          m_newPos;
        };

        NEW_COMMAND( Resize )
        {
            public:
                Resize( EffectHelper *helper, qint64 newBegin, qint64 newEnd );
                virtual void        redo();
                virtual void        undo();
            private:
                EffectHelper        *m_helper;
                qint64              m_newBegin;
                qint64              m_newEnd;
                qint64              m_oldBegin;
                qint64              m_oldEnd;
        };
    }
}

#endif // COMMANDS_H
