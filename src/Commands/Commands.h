/*****************************************************************************
 * Commands.h: Contains all the implementation of VLMC commands.
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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

#include "Workflow/MainWorkflow.h"

class   Clip;
class   EffectUser;

namespace Commands
{
#ifdef WITH_GUI
    class       Generic : public QObject, public QUndoCommand
#else
    class       Generic : public QObject
#endif
    {
        Q_OBJECT

        public:
            Generic();
            virtual void    internalRedo() = 0;
            virtual void    internalUndo() = 0;
            void            redo();
            void            undo();
            bool            isValid() const;
#ifndef WITH_GUI
            void            setText( const QString& text ) ;
            QString         text() const;
#endif
        private:
            bool            m_valid;
            QString         m_text;
        protected slots:
            virtual void    retranslate() = 0;
            void            invalidate();
        signals:
            void            invalidated();
    };

    void        trigger( Generic* command );

    namespace   Clip
    {
        class   Add : public Generic
        {
            Q_OBJECT

            public:
                Add( ::Clip* clip, TrackWorkflow* tw, qint64 pos );
                virtual ~Add();
                virtual void    internalRedo();
                virtual void    internalUndo();
                virtual void    retranslate();
            private:
                ::Clip                      *m_clip;
                TrackWorkflow               *m_trackWorkflow;
                qint64                      m_pos;
        };

        class   Move : public Generic
        {
            Q_OBJECT

            public:
                Move( TrackWorkflow *oldTrack, TrackWorkflow *newTrack,
                        ::Clip *clip, qint64 newPos );
                virtual void    internalRedo();
                virtual void    internalUndo();
                virtual void    retranslate();

            private:
                TrackWorkflow   *m_oldTrack;
                TrackWorkflow   *m_newTrack;
                ::Clip          *m_clip;
                qint64          m_newPos;
                qint64          m_oldPos;
        };

        class   Remove : public Generic
        {
            Q_OBJECT

            public:
                Remove( ::Clip* clip, TrackWorkflow* tw );
                virtual void internalRedo();
                virtual void internalUndo();
                virtual void    retranslate();

            private:
                ::Clip                      *m_clip;
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
        class   Resize : public Generic
        {
            Q_OBJECT

            public:
                Resize( TrackWorkflow* tw, ::Clip* clip,
                            qint64 newBegin, qint64 newEnd, qint64 newPos );
                virtual void    internalRedo();
                virtual void    internalUndo();
                virtual void    retranslate();

            private:
                TrackWorkflow*              m_trackWorkflow;
                ::Clip*                     m_clip;
                qint64                      m_newBegin;
                qint64                      m_newEnd;
                qint64                      m_oldBegin;
                qint64                      m_oldEnd;
                qint64                      m_newPos;
                qint64                      m_oldPos;
        };

        class   Split : public Generic
        {
            Q_OBJECT

            public:
                Split( TrackWorkflow *tw, ::Clip* toSplit, qint64 newClipPos,
                           qint64 newClipBegin );
                ~Split();
                virtual void    internalRedo();
                virtual void    internalUndo();
                virtual void    retranslate();
            private:
                TrackWorkflow               *m_trackWorkflow;
                ::Clip*                     m_toSplit;
                ::Clip*                     m_newClip;
                qint64                      m_newClipPos;
                qint64                      m_newClipBegin;
                qint64                      m_oldEnd;
        };
    }
    namespace   Effect
    {
        class   Add : public Generic
        {
            Q_OBJECT

            public:
                Add( EffectHelper *helper, EffectUser *target );
                virtual void    internalRedo();
                virtual void    internalUndo();
                virtual void    retranslate();
            private:
                EffectHelper    *m_helper;
                EffectUser      *m_target;
        };

        class   Move : public Generic
        {
            Q_OBJECT

            public:
                Move( EffectHelper *helper, EffectUser *old, EffectUser *newUser, qint64 pos );
                virtual void    internalRedo();
                virtual void    internalUndo();
                virtual void    retranslate();
            private:
                EffectHelper    *m_helper;
                EffectUser      *m_old;
                EffectUser      *m_new;
                qint64          m_oldPos;
                qint64          m_newPos;
                qint64          m_newEnd;
                qint64          m_oldEnd;
        };

        class   Resize : public Generic
        {
            Q_OBJECT

            public:
                Resize( EffectUser *target, EffectHelper *helper, qint64 newBegin, qint64 newEnd );
                virtual void        internalRedo();
                virtual void        internalUndo();
                virtual void        retranslate();
            private:
                EffectUser          *m_target;
                EffectHelper        *m_helper;
                qint64              m_newBegin;
                qint64              m_newEnd;
                qint64              m_oldBegin;
                qint64              m_oldEnd;
        };

        class   Remove : public Generic
        {
            Q_OBJECT

            public:
                Remove( EffectHelper *helper, EffectUser *user );
                virtual void    internalRedo();
                virtual void    internalUndo();
                virtual void    retranslate();
            private:
                EffectHelper    *m_helper;
                EffectUser      *m_user;
        };
    }
}

#endif // COMMANDS_H
