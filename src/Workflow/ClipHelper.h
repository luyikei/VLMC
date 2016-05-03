/*****************************************************************************
 * ClipHelper.h: Contains information about a Clip in the workflow
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

#ifndef CLIPHELPER_H
#define CLIPHELPER_H

class   Clip;
class   ClipWorkflow;

#include "Helper.h"

#include <QObject>
#include <QUuid>

class   ClipHelper : public Workflow::Helper
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

        ClipHelper( Clip* clip, qint64 begin = -1, qint64 end = -1,
                    const QString& uuid = QString() );

        Clip*       clip()
        {
            return m_clip;
        }
        const Clip* clip() const
        {
            return m_clip;
        }
        void        setBegin( qint64 begin );
        void        setEnd( qint64 end );
        void        setBoundaries( qint64 begin, qint64 end );
        ClipWorkflow    *clipWorkflow();
        void            setClipWorkflow( ClipWorkflow* cw );

        Formats         formats() const;
        void            setFormats( Formats formats );

    private:
        Clip*           m_clip;
        ClipWorkflow*   m_clipWorkflow;

        Formats         m_formats;

    private slots:
        void        clipDestroyed();
};

Q_DECLARE_OPERATORS_FOR_FLAGS( ClipHelper::Formats )

#endif // CLIPHELPER_H
