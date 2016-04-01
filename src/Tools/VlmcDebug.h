/*****************************************************************************
 * DebugHelper.cpp: Debug helpers for VLMC
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

#ifndef DEBUGHELPER_H
#define DEBUGHELPER_H

#include <QDebug>
#include <QThread>
#include <QTime>

inline QDebug operator<<( QDebug& qdbg, const std::string& str )
{
    qdbg << str.c_str();
    return qdbg;
}

inline QDebug vlmcDebug()
{
    return (qDebug().nospace() << '[' << qPrintable(QTime::currentTime().toString("hh:mm:ss.zzz")) << "] T #" << QThread::currentThreadId() << " D:").space();
}

inline QDebug vlmcWarning()
{
    return (qWarning().nospace() << '[' << qPrintable(QTime::currentTime().toString("hh:mm:ss.zzz")) << "] T #" << QThread::currentThreadId() << " W:").space();
}

inline QDebug vlmcCritical()
{
    return (qCritical().nospace() << '[' << qPrintable(QTime::currentTime().toString("hh:mm:ss.zzz")) << "] T #" << QThread::currentThreadId() << " C:").space();
}

inline void vlmcFatal(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    char* str = nullptr;
    if (vasprintf(&str, msg, args) < 0)
    {
        qFatal("[%s] T #%p F: %s", qPrintable(QTime::currentTime().toString("hh:mm:ss.zzz")), reinterpret_cast<void*>(QThread::currentThreadId()), "<Failed to generate message. Have fun>");
    }
    else
    {
        qFatal("[%s] T #%p F: %s", qPrintable(QTime::currentTime().toString("hh:mm:ss.zzz")), reinterpret_cast<void*>(QThread::currentThreadId()), str);
        free(str);
    }
    va_end(args);
}


#endif // DEBUGHELPER_H
