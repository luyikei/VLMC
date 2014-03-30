/*****************************************************************************
 * AutomaticBackup.h: Handles the project automatic backup & associated settings
 *****************************************************************************
 * Copyright (C) 2008-2014 VideoLAN
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

#ifndef AUTOMATICBACKUP_H
#define AUTOMATICBACKUP_H

class QTimer;

class ProjectManager;
class Settings;

#include <QObject>

class AutomaticBackup : public QObject
{
    Q_OBJECT

    public:
        explicit AutomaticBackup(Settings* vlmcSettings, QObject *parent = 0);
        virtual ~AutomaticBackup();
        void    setProject( ProjectManager* projectManager );

    private:
        Settings*   m_vlmcSettings;
        QTimer*     m_timer;

    private slots:
        void            automaticSaveEnabledChanged( const QVariant& enabled );
        void            automaticSaveIntervalChanged( const QVariant& interval );

};

#endif // AUTOMATICBACKUP_H
