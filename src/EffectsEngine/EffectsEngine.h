/*****************************************************************************
 * EffectsEngine.h: Manage the effects plugins.
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <beauze.h@gmail.com>
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

#ifndef EFFECTSENGINE_H
#define EFFECTSENGINE_H

#include "Singleton.hpp"

class   QTime;

#include "Effect.h"
#include "MainWorkflow.h"

class   FilterInstance;
class   MixerInstance;

#include <QObject>
#include <QHash>

class   QSettings;

class   EffectsEngine : public QObject, public Singleton<EffectsEngine>
{
    Q_OBJECT

    public:
        template <typename T>
        struct      EffectHelper
        {
            EffectHelper( T *_effect, qint64 _start = 0, qint64 _end = -1,
                          const QString& _uuid = QString() ) :
                effect( _effect ),
                start( _start ),
                end( _end )
            {
                if ( _uuid.isNull() == true )
                    uuid = QUuid::createUuid();
                else
                    uuid = _uuid;
            }

            T*          effect;
            qint64      start;
            qint64      end;
            QUuid       uuid;
        };
        typedef EffectHelper<FilterInstance>    FilterHelper;
        typedef QList<FilterHelper*>            FilterList;
        typedef EffectHelper<MixerInstance>     MixerHelper;
        typedef QList<MixerHelper*>             MixerList;

        Effect*     effect( const QString& name );
        bool        loadEffect( const QString& fileName );
        void        browseDirectory( const QString& path );

        static void applyFilters( const FilterList &effects,
                                  Workflow::Frame *frame, qint64 currentFrame, double time );
        static void saveFilters( const FilterList &effects, QXmlStreamWriter &project );
        static void initFilters( const FilterList &effects, quint32 width, quint32 height );

    private:
        EffectsEngine();
        ~EffectsEngine();

        QHash<QString, Effect*> m_effects;
        QSettings               *m_cache;
        QTime                   *m_time;

    signals:
        void        effectAdded( Effect*, const QString& name, Effect::Type );
    friend class    Singleton<EffectsEngine>;
};

#endif // EFFECTSENGINE_H
