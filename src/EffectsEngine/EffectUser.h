/*****************************************************************************
 * EffectUser.h: Handles effects list and application
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

#ifndef EFFECTUSER_H
#define EFFECTUSER_H

#include "EffectsEngine.h"

#include <QObject>
#include <QList>
#include <QXmlStreamWriter>

class   QDomElement;
class   QReadWriteLock;

class EffectUser : public QObject
{
    Q_OBJECT

    public:
        /**
         *  \brief      Add an effect to the TrackWorkflow
         *
         *  \param      effect  The effect instance. Can be either mixer or filter.
         */
        EffectHelper                    *addEffect( Effect *effect, qint64 start = 0, qint64 end = -1 );
        void                            addEffect( EffectHelper *effect );
        void                            moveEffect( EffectHelper *helper, qint64 newPos );
        void                            removeEffect( EffectHelper *helper );
        const EffectsEngine::EffectList &effects( Effect::Type type ) const;
        void                            removeEffect( Effect::Type type, qint32 idx );
        void                            swapFilters( qint32 idx, qint32 idx2 );
        qint32                          count( Effect::Type type ) const;
        void                            cleanEffects();

    protected:
        EffectUser();
        virtual ~EffectUser();
        void                            initFilters();
        void                            initMixers();

        //Filters:
        quint32                         *applyFilters( const Workflow::Frame *frame,
                                                       qint64 currentFrame, double time );
        void                            loadEffects( const QDomElement &project );
        void                            saveFilters( QXmlStreamWriter &project ) const;

        //Mixers methods:
        EffectHelper                    *getMixer( qint64 currentFrame );

    protected:
        /**
         *  \brief  Will be equal to true if a render has been started, even if it paused.
         *
         *  If this is true, when an effect is loaded, it has to be initialized
         *  immediately,
         */
        bool                                    m_isRendering;
        quint32                                 m_width;
        quint32                                 m_height;

        QReadWriteLock                          *m_effectsLock;
        EffectsEngine::EffectList               m_mixers;
        EffectsEngine::EffectList               m_filters;

    signals:
        void                                    effectAdded( EffectHelper *helper, qint64 pos );
        void                                    effectMoved( const QUuid&, qint64 newPos );
        void                                    effectRemoved( const QUuid& );
};

#endif // EFFECTUSER_H
