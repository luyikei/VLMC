/*****************************************************************************
 * Clip.h : Represents a basic container for media informations.
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzee-Luyssen <hugo@vlmc.org>
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

/** \file
  * This file contains the Clip class declaration/definition.
  * It's used by the timeline in order to represent a subset of a media
  */

#ifndef CLIP_H__
# define CLIP_H__

#include <QObject>
#include <QUuid>

#include "Media.h"


class Media;

class   Clip : public QObject
{
    Q_OBJECT

    public:
        static const int DefaultFPS;
        /**
         *  \brief  Construct the base clip for a Media.
         *
         *  This clip will represent the whole media as a Clip.
         *  \param  parent  The media to represent.
         *  \param  uuid    A forced unique id. If not given, a new unique id will be generated.
         */
        Clip( Media *parent, const QString& uuid = QString() );
        /**
         *  \brief  Constructs a Clip that is a subpart of a Media.
         *
         *  \param  parent  The media to represent.
         *  \param  begin   The clip beginning (in frames, from the parent's beginning)
         *  \param  end     The end, in frames, from the parent's beginning. If not given,
         *                  the end of the parent will be used.
         *  \param  uuid    A unique identifier. If not given, one will be generated.
         */
        Clip( Media *parent, qint64 begin, qint64 end = -1, const QString &uuid = QString() );
        /**
         *  \brief  Clones a Clip, potentially with a new begin and end.
         *
         *  \param  creator The clip to clone.
         *  \param  begin   The clip beginning (in frames, from the parent's beginning).
         *                  If not given, 0 is assumed.
         *  \param  end     The end, in frames, from the parent's beginning. If not given,
         *                  the end of the parent will be used.
         */
        Clip( Clip *creator, qint64 begin = -1, qint64 end = -1 );
        virtual ~Clip();

        qint64              begin() const;
        qint64              end() const;

        void                setBegin( qint64 begin, bool updateMax = false );
        void                setEnd( qint64 end, bool updateMax = false );
        void                setBoundaries( qint64 newBegin, qint64 newEnd, bool updateMax = false );

        /**
            \return         Returns the clip length in frame.
        */
        qint64              length() const;

        /**
            \return         Returns the clip length in seconds.
        */
        qint64              lengthSecond() const;

        /**
            \return         Returns the Media that the clip was basep uppon.
        */
        Media*              getParent();

        /**
            \brief          Returns an unique Uuid for this clip (which is NOT the
                            parent's Uuid).

            \return         The Clip's Uuid as a QUuid
        */
        const QUuid         &uuid() const;
        void                setUuid( const QUuid &uuid );

        const QStringList   &metaTags() const;
        void                setMetaTags( const QStringList &tags );
        bool                matchMetaTag( const QString &tag ) const;

        const QString       &notes() const;
        void                setNotes( const QString &notes );

        qint64              maxBegin() const;
        qint64              maxEnd() const;

        void                computeLength();

        bool                isRootClip() const;
        Clip*               rootClip();

    private:
        Media               *m_parent;
        /**
         *  \brief  This represents the beginning of the Clip in frames, from the
         *          beginning of the parent Media.
         */
        qint64              m_begin;
        /**
         *  \brief  This represents the end of the Clip in frames, from the
         *          beginning of the parent Media.
         */
        qint64              m_end;

        /**
         *  \brief  The length in frames
         */
        qint64              m_length;
        /**
         *  \brief  The length in seconds (Be carreful, VLC uses MILLIseconds)
         */
        qint64              m_lengthSeconds;
        /**
         *  The Clip's timeline UUID. Used to identify the Clip in the
         *  timeline, as a unique object, even if this clip is present more than
         *  once.
         */
        QUuid               m_uuid;
        QStringList         m_metaTags;
        QString             m_notes;

        /**
         *  This is used for the resize. The clip won't be abble to be resized beyond this value.
         *  ie this clip won't start before m_maxBegin.
         */
        qint64              m_maxBegin;

        /**
         *  This is used for the resize. The clip won't be abble to be resized beyond this value
         *  ie this clip won't end before m_maxEnd.
         */
        qint64              m_maxEnd;

        /**
         *  \brief          Return the root clip.
         *
         *  The root clip is the base clip for the parent media.
         */
        Clip*               m_rootClip;

    signals:
        void                lengthUpdated();
};

#endif //CLIP_H__
