/*****************************************************************************
 * Clip.h : Represents a basic container for media informations.
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
 *          Yikei Lu    <luyikei.qmltu@gmail.com>
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

#include "Workflow/Helper.h"
#include <QHash>
#include <QSharedPointer>
#include <QStringList>
#include <QUuid>
#include <QXmlStreamWriter>
#include "Backend/IInput.h"

#include <memory>

class   Media;

class   Clip : public Workflow::Helper
{
    Q_OBJECT

    public:
        enum    Format
        {
            None          = 0,
            Audio         = 1 << 0,
            Video         = 1 << 1,
        };
        Q_DECLARE_FLAGS( Formats, Format )

        static const int DefaultFPS;
        /**
         *  \brief  Constructs a Clip that is a subpart of a Media.
         *
         *  \param  parent  The media to represent.
         *  \param  begin   The clip beginning (in frames, from the parent's beginning)
         *  \param  end     The end, in frames, from the parent's beginning. If not given,
         *                  the end of the parent will be used.
         *  \param  uuid    A unique identifier. If not given, one will be generated.
         */
        Clip( QSharedPointer<Media> parent, qint64 begin = 0, qint64 end = Backend::IInput::EndOfMedia, const QUuid &uuid = QStringLiteral() );

        virtual ~Clip();

        /**
            \return         Returns the clip length in seconds.
        */
        qint64              lengthSecond() const;

        /**
            \return         Returns the Media that the clip was basep uppon.
        */
        QSharedPointer<Media>       media();
        QSharedPointer<const Media> media() const;

        /**
            \brief          Returns an unique Uuid for this clip (which is NOT the
                            parent's Uuid).

            \return         The Clip's Uuid as a QUuid
        */
        const QUuid         &uuid() const override;
        void                setUuid( const QUuid &uuid );

        virtual qint64      begin() const override;
        virtual qint64      end() const override;
        virtual void        setBegin( qint64 begin ) override;
        virtual void        setEnd( qint64 end ) override;
        virtual qint64      length() const override;
        virtual void        setBoundaries( qint64 begin, qint64 end ) override;

        void                setLinkedClipUuid( const QUuid& uuid );
        const QUuid&        linkedClipUuid() const;

        void                setLinked( bool isLinked );
        bool                isLinked() const;

        const QStringList   &metaTags() const;
        void                setMetaTags( const QStringList &tags );
        bool                matchMetaTag( const QString &tag ) const;

        const QString       &notes() const;
        void                setNotes( const QString &notes );

        QVariant            toVariant() const;

        Formats             formats() const;
        void                setFormats( Formats formats );

        Backend::IInput* input();

        //FIXME: This shouldn't be represented in the Library
        void                loadFilters(const QVariantMap& v );

    private:
        QSharedPointer<Media>               m_media;
        std::unique_ptr<Backend::IInput>    m_input;

        QStringList         m_metaTags;
        QString             m_notes;

        QUuid               m_linkedClipUuid;
        bool                m_isLinked;

        Formats             m_formats;

    signals:
        /**
         *  \brief          Act just like QObject::destroyed(), but before the clip deletion.
         */
        void                unloaded( Clip* );
};

Q_DECLARE_OPERATORS_FOR_FLAGS( Clip::Formats )

#endif //CLIP_H__
