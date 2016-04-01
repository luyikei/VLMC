/*****************************************************************************
 * RenderWidget: Vout render widget
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Rohit Yadav <rohityadav89@gmail.com>
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

#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QtGlobal>
#include <QWidget>

#if defined( Q_OS_MAC )
#include <QMacCocoaViewContainer>

#ifdef __OBJC__
#define ADD_COCOA_NATIVE_REF(CocoaClass) \
    @class CocoaClass; \
    typedef CocoaClass *Native##CocoaClass##Ref
#else
#define ADD_COCOA_NATIVE_REF(CocoaClass) typedef void *Native##CocoaClass##Ref
#endif

ADD_COCOA_NATIVE_REF(NSView);
#endif

class RenderWidget : public QWidget
{
    Q_OBJECT

public:
#if defined( Q_OS_MAC )
    RenderWidget( QWidget* parent = nullptr );
    NativeNSViewRef id() const;
    void release();

private:
    NativeNSViewRef          m_video;
    QMacCocoaViewContainer*  m_container;
#else
    RenderWidget( QWidget* parent = nullptr )
        : QWidget (parent) {}
    WId id() const { return winId(); }
#endif
};

#endif // RENDERWIDGET_H
