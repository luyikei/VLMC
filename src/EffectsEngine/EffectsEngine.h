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
#include "Effect.h"

namespace Workflow
{
    class   Frame;
}

class   FilterInstance;
class   MixerInstance;

class   QTime;
class   QXmlStreamWriter;

#include <QObject>
#include <QStringList>
#include <QHash>
#include <QUuid>
#include <QMetaType>

class   QSettings;

class   EffectsEngine : public QObject, public Singleton<EffectsEngine>
{
    Q_OBJECT

    public:
        struct      EffectHelper
        {
            EffectHelper( EffectInstance *_effect, qint64 _start = 0, qint64 _end = -1,
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

            EffectInstance  *effect;
            qint64          start;
            qint64          end;
            QUuid           uuid;
        };

        typedef QList<EffectHelper*>            EffectList;
        static const quint32                    MaxFramesForMixer = 3;

        Effect*             effect( const QString& name );
        const QStringList&  effects( Effect::Type type ) const;
        bool                loadEffect( const QString& fileName );
        void                loadEffects();

        static void     initEffects( const EffectList &effects, quint32 width, quint32 height );
        //Filters methods:
        static quint32  *applyFilters( const EffectList &effects,
                                  const Workflow::Frame *frame, qint64 currentFrame, double time );
        static void     saveFilters( const EffectList &effects, QXmlStreamWriter &project );

        //Mixers methods:
        static EffectHelper     *getMixer( const EffectList &mixers, qint64 currentFrame );

    private:
        EffectsEngine();
        ~EffectsEngine();
        void        browseDirectory( const QString& path );

    private:
        QHash<QString, Effect*> m_effects;
        QList<QStringList>      m_names;
        QSettings               *m_cache;
        QTime                   *m_time;

    signals:
        void        effectAdded( Effect*, const QString& name, Effect::Type );
    friend class    Singleton<EffectsEngine>;
};

Q_DECLARE_METATYPE(EffectsEngine::EffectHelper*)

#endif // EFFECTSENGINE_H
