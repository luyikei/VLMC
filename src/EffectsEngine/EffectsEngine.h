/*****************************************************************************
 * EffectsEngine.h: Manage the effects plugins.
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

#ifndef EFFECTSENGINE_H
#define EFFECTSENGINE_H


namespace Workflow
{
    class   Frame;
}

class   EffectHelper;
class   FilterInstance;
class   MixerInstance;

class   QTime;
class   QXmlStreamWriter;

#include <QObject>
#include <QStringList>
#include <QHash>
#include <QUuid>
#include <QMetaType>

#include "EffectsEngine/Effect.h"

class   QSettings;

class   EffectsEngine : public QObject
{
    Q_OBJECT

    public:
        typedef QList<EffectHelper*>    EffectList;
        static const quint32            MaxFramesForMixer = 3;

        EffectsEngine();
        ~EffectsEngine();

        Effect*             effect( const QString& name );
        const QStringList&  effects( Effect::Type type ) const;
        bool                loadEffect( const QString& fileName );
        void                loadEffects();

    private:
        void        browseDirectory( const QString& path );

    private:
        QHash<QString, Effect*> m_effects;
        QList<QStringList>      m_names;
        QSettings               *m_cache;
        QTime                   *m_time;

    signals:
        void        effectAdded( Effect*, const QString& name, Effect::Type );
};

#endif // EFFECTSENGINE_H
