/*****************************************************************************
 * EffectHelper: Contains informations about effects
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
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

#ifndef EFFECTHELPER_H
#define EFFECTHELPER_H

class   EffectInstance;
class   EffectUser;

#include "Helper.h"

#include <QObject>
#include <QUuid>
#include <QMetaType>

class   EffectHelper : public Workflow::Helper
{
    Q_OBJECT

    public:
        EffectHelper( EffectInstance *effectInstance, qint64 begin = 0, qint64 end = -1,
                      const QString& uuid = QString() );

        EffectInstance          *effectInstance();
        const EffectInstance    *effectInstance() const;
        EffectUser              *target();
        void                    setTarget( EffectUser *target );

    private:
        EffectInstance          *m_effectInstance;
        EffectUser              *m_target;
};

Q_DECLARE_METATYPE( EffectHelper* );

#endif // EFFECTHELPER_H
