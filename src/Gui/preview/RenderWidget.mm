/*****************************************************************************
 * RenderWidget.mm: A NSView Vout render widget for Mac OS
 *****************************************************************************
 * Copyright (C) 2008-2011 VideoLAN
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

#include <QtGlobal>
#if defined( Q_OS_MAC )

#include "RenderWidget.h"
#include <QPalette>
#include <QColor>
#include <QVBoxLayout>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import <AppKit/NSView.h>

@interface VLCNSView : NSView
- ( void ) setStretchesVideo : ( BOOL ) value;
- ( BOOL ) stretchesVideo;
- ( void ) addVoutSubview:( NSView * ) aView;
- ( void ) removeVoutSubview:( NSView * ) aView;
@property BOOL stretchesVideo;
@end

@implementation VLCNSView
@synthesize stretchesVideo = stretchesVideo;
- ( id ) initWithFrame:( NSRect ) frameRect
{
    if ((self = [super initWithFrame:frameRect]) == nil){ return nil; }
    return self;
}
- ( void ) dealloc
{
    [super dealloc];
}
- ( void ) setStretchesVideo : ( BOOL ) value
{
    stretchesVideo = value;
}
- ( BOOL ) stretchesVideo
{
    return stretchesVideo;
}
- ( void ) addVoutSubview:( NSView * ) aView
{
    [aView setFrame:[self bounds]];
    [self addSubview:aView];
    [aView setAutoresizingMask:NSViewHeightSizable | NSViewWidthSizable];
}
- (void) removeVoutSubview:(NSView *)aView {}
@end

RenderWidget::RenderWidget( QWidget *parent ) :
    QWidget( parent )
{
    m_video = [[VLCNSView alloc] init];
    m_container = new QMacCocoaViewContainer( m_video, this );
    m_container->setAutoFillBackground( true );

    QPalette videoPalette = m_container->palette();
    videoPalette.setColor( QPalette::Window, QColor( Qt::black ) );
    m_container->setPalette( videoPalette );

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget( m_container );
    setLayout( layout );
}

/* winId should return pointer to the NSView, m_video */
NativeNSViewRef
RenderWidget::id() const
{
    return m_video;
}

void
RenderWidget::release()
{
    [m_video release];
}

#endif
