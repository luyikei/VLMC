/*****************************************************************************
 * ProjectCallbacks.h: Defines some project related callback interfaces
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

#ifndef PROJECTCALLBACKS_H
#define PROJECTCALLBACKS_H

#include <QString>

class   IProjectUiCb
{
public:
    enum SaveMode
    {
        Save,       // Save the project
        Discard,    // Discard it
        Cancel      // Don't do anything
    };

    virtual ~IProjectUiCb() {}

    /**
     * @brief getProjectFile    Ask the user where to save a new project
     * @param defaultPath       A default project location, if any.
     * @return The selected project file
     */
    virtual QString    getProjectFileDestination( const QString& defaultPath ) = 0;

    /**
     * @brief shouldLoadBackupFile
     * @return True if the user wants to load the backup file.
     */
    virtual bool    shouldLoadBackupFile() = 0;

    /**
     * @brief shouldDeleteOutdatedBackupFile
     * @return True if the user wants to delete this backup file
     */
    virtual bool    shouldDeleteOutdatedBackupFile() = 0;
};

#endif // PROJECTCALLBACKS_H
