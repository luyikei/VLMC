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
        Clip( QSharedPointer<Media> parent, qint64 begin = 0, qint64 end = Backend::IInput::EndOfMedia, const QString &uuid = QStringLiteral() );
        /**
         *  \brief  Clones a Clip, potentially with a new begin and end.
         *
         *  \param  creator The clip to clone.
         *  \param  begin   The clip beginning (in frames, from the parent's beginning).
         *                  If not given, 0 is assumed.
         *  \param  end     The end, in frames, from the parent's beginning. If not given,
         *                  the end of the parent will be used.
         */
        Clip( Clip *creator, qint64 begin = -1, qint64 end = Backend::IInput::EndOfParent, const QString& uuid = QStringLiteral() );
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

        Clip                *parent();
        const Clip          *parent() const;

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

        bool                isRootClip() const;
        Clip*               rootClip();

        /**
         *  \brief          Clear all the clip subclips recursively.
         */
        void                clear();

        bool                addSubclip( Clip* clip );

        /**
         *  \return         true if the clip is a child of the given clip uuid
         */
        bool                isChild( const QUuid& uuid ) const;

        QVariant            toVariant() const;
        QVariant            toVariantFull() const;

        Formats             formats() const;
        void                setFormats( Formats formats );

        Backend::IInput* input();

        static Clip*        fromVariant( const QVariant& v );

    private:
        static Clip*        fromVariant( const QVariant& v, Clip* parent );
        void                loadVariant(const QVariantMap& v );

    private:
        QSharedPointer<Media>               m_media;
        std::unique_ptr<Backend::IInput>    m_input;

        QStringList         m_metaTags;
        QString             m_notes;

        /**
         *  \brief          Return the root clip.
         *
         *  The root clip is the base clip for the parent media.
         */
        Clip*               m_rootClip;

        QHash<QUuid, Clip*> m_subclips;

        Clip*               m_parent;

        QUuid               m_linkedClipUuid;
        bool                m_isLinked;

        Formats             m_formats;

    private slots:
        void                mediaMetadataUpdated();

    signals:
        /**
         *  \brief          Act just like QObject::destroyed(), but before the clip deletion.
         */
        void                unloaded( Clip* );

        /**
         *  \brief          This signal should be emitted to tell a new sublip have been added
         *  \param Clip     The newly added subclip
         */
        void    subclipAdded( Clip* );
        /**
         *  \brief This signal should be emiteted when a subclip has been removed
         *  This signal pass a QUuid as the clip may be deleted when the signal reaches its
         *  slot.
         *  \param uuid The removed clip uuid
         */
        void    subclipRemoved( const QUuid& );
};

Q_DECLARE_OPERATORS_FOR_FLAGS( Clip::Formats )

#endif //CLIP_H__
