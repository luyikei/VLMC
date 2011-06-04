/*****************************************************************************
 * AudioSpectrumDrawer.cpp: Audio Spectrum Drawing Widget
 *****************************************************************************
 * Copyright (C) 2008-2011 VideoLAN
 *
 * Authors: Geoffroy Lacarriere <geoffroylaca@gmail.com>
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

#include "AudioSpectrumDrawer.h"

AudioSpectrumDrawer::AudioSpectrumDrawer( QPainter* painter, int height, int width, QList<int>* audioValueList )
    : m_painter( painter ), m_height( height ), m_width( width ), m_audioValueList( audioValueList )
{
    m_painter->setRenderHint( QPainter::Antialiasing, true );
    m_painter->setPen( QPen( QColor( 79, 106, 25 ), 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin ) );
}

void
AudioSpectrumDrawer::run()
{
    qreal max = 0;
    for( int i = 0; i < m_audioValueList->count(); i++ )
        if ( m_audioValueList->at(i) > max )
            max = m_audioValueList->at(i);

    for( int x = 0; x < m_audioValueList->count(); x++ )
    {
        if ( x <= m_height )
        {
            qreal y = ( (qreal)m_audioValueList->at(x) / max ) * 500;
            y -= 365;
            m_path.lineTo( x, y );
        }
    }
    m_painter->drawPath( m_path );
}
