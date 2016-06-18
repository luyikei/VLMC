/*****************************************************************************
 * Types.h: Workflow related types.
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

#ifndef TYPES_H
#define TYPES_H

#include <qglobal.h>

namespace   Workflow
{
    // This is constrained by frei0r
    const quint32   Depth = 4;

    /**
     *  \enum   Represents the potential Track types.
     */
    enum    TrackType
    {
        VideoTrack, ///< Represents a video track
        AudioTrack, ///< Represents an audio track
        NbTrackType, ///< Used to know how many types we have
    };
}

namespace   Vlmc
{
    /**
     *  \enum   Used to know which part required a change of rendered frame.
     *          The main use of this enum is to avoid infinite information propagation
     *          such as the timeline informing itself that the frame as changed, which
     *          would cause a signal to be emmited to inform every other part that the
     *          rendered frame has changed, and so on.
     */
    enum    FrameChangedReason
    {
        Renderer, ///< Used by the WorkflowRenderer
        /**
         *  \brief      Used by the timeline cursor.
         *  \warning    The timeline cursor is not the timeline ruler
         */
        TimelineCursor,
        PreviewCursor, ///< Used by the preview widget, when using the time cursor.
        RulerCursor, ///< Used by the timeline's ruler.
    };
}

#endif // TYPES_H
