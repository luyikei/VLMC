import QtQuick 2.0

Item {
    id: track
    width: parent.width
    height: trackHeight
    z: 10

    property int trackId
    property string type
    property ListModel clips

    Rectangle {
        id: clipArea
        x: trackInfo.width
        color: "#222222"
        height: parent.height
        width: track.width - initPosOfCursor

        Rectangle {
            color: "#666666"
            height: 1
            width: parent.width
            anchors.bottom: clipArea.bottom

            Component.onCompleted: {
                if ( track.type === "Video" && track.trackId == 0 )
                {
                    color = "#111111";
                }
            }
        }

        DropArea {
            id: dropArea
            anchors.fill: parent
            keys: ["Clip", "vlmc/uuid"]

            // Enum for drop mode
            readonly property var dropMode: {
                "New": 0,
                "Move": 1,
            }
            readonly property int magneticMargin: 25

            property string currentUuid
            property var aClipInfo: null
            property var vClipInfo: null

            property int lastX: 0
            property int deltaX: 0

            function findNewPosition( newX, target, useMagneticMode ) {
                var oldX = target.pixelPosition();

                if ( useMagneticMode === true ) {
                    var leastDistance = magneticMargin;
                    // Check two times
                    for ( var k = 0; k < 2; ++k ) {
                        for ( var j = 0; j < markers.count; ++j ) {
                            var mx = ftop( markers.get( j ).position );
                            if ( Math.abs( newX - mx ) < leastDistance ) {
                                leastDistance = Math.abs( newX - mx );
                                newX = mx;
                            }
                            else if ( Math.abs( newX + target.width - mx ) < leastDistance ) {
                                leastDistance = Math.abs( newX + target.width - mx );
                                newX = mx - target.width;
                            }
                        }
                    }
                }

                // Collision detection
                var isCollided = true;
                var currentTrack = trackContainer( target.type )["tracks"].get( target.newTrackId );
                if ( currentTrack )
                    var clips = currentTrack["clips"];
                else
                    return oldX;
                for ( j = 0; j < clips.count + 2 && isCollided; ++j ) {
                    isCollided = false;
                    for ( k = 0; k < clips.count; ++k ) {
                        var clip = clips.get( k );
                        if ( clip.uuid === target.uuid )
                            continue;
                        var sw = target.width; // Width of the source clip
                        var cx = ftop( clip["position"] );
                        var cw = ftop( clip["end"] - clip["begin"] + 1);
                        // Set a right position
                        //
                        // HACK: If magnetic mode, consider clips bigger.
                        var clipMargin = useMagneticMode ? magneticMargin : 0;
                        if ( cx  + cw > newX && newX + sw > cx )
                            isCollided = true;

                        cw += clipMargin * 2
                        cx -= clipMargin
                        if ( cx + cw > newX && newX + sw > cx ) {
                            if ( cx > newX ) {
                                if ( cx - sw > 0 )
                                    newX = cx - sw + clipMargin;
                                else
                                    newX = oldX;
                            } else {
                                newX = cx + cw - clipMargin;
                            }
                        }
                        if ( isCollided )
                            break;
                    }
                }

                if ( isCollided ) {
                    for ( k = 0; k < clips.count; ++k ) {
                        clip = clips.get( k );
                        if ( clip.uuid === target.uuid )
                            continue;
                        cx = ftop( clip["position"] );
                        cw = ftop( clip["end"] - clip["begin"] + 1);
                        newX = Math.max( newX, cx + cw );
                    }
                }

                return newX;
            }

            onDropped: {
                if ( drop.keys.indexOf( "vlmc/uuid" ) >= 0 ) {
                    aClipInfo = findClipFromTrack( "Audio", trackId, "audioUuid" );
                    vClipInfo = findClipFromTrack( "Video", trackId, "videoUuid" );
                    var pos = 0;
                    if ( aClipInfo ) {
                        pos = aClipInfo["position"];
                        removeClipFromTrack( "Audio", trackId, "audioUuid" );
                    }
                    if ( vClipInfo ) {
                        pos = vClipInfo["position"];
                        removeClipFromTrack( "Video", trackId, "videoUuid" );
                    }
                    workflow.addClip( drop.getDataAsString("vlmc/uuid"), trackId, pos, false );
                    currentUuid = "";
                    aClipInfo = null;
                    vClipInfo = null;
                    clearSelectedClips();
                    adjustTracks( "Audio" );
                    adjustTracks( "Video" );
                }
            }

            onExited: {
                if ( currentUuid !== "" ) {
                    removeClipFromTrack( "Audio", trackId, "audioUuid" );
                    removeClipFromTrack( "Video", trackId, "videoUuid" );
                }
            }

            onEntered: {
                if ( drag.keys.indexOf( "vlmc/uuid" ) >= 0 ) {
                    clearSelectedClips();
                    if ( currentUuid === drag.getDataAsString( "vlmc/uuid" ) ) {
                        if ( aClipInfo )
                        {
                            aClipInfo["position"] = ptof( drag.x );
                            aClipInfo = addClip( "Audio", trackId, aClipInfo );
                        }
                        if ( vClipInfo )
                        {
                            vClipInfo["position"] = ptof( drag.x );
                            vClipInfo = addClip( "Video", trackId, vClipInfo );
                        }
                    }
                    else {
                        var newClipInfo = workflow.libraryClipInfo( drag.getDataAsString( "vlmc/uuid" ) );
                        currentUuid = "" + newClipInfo["uuid"];
                        newClipInfo["position"] = ptof( drag.x );
                        if ( newClipInfo["audio"] ) {
                            newClipInfo["uuid"] = "audioUuid";
                            aClipInfo = addClip( "Audio", trackId, newClipInfo );
                        }
                        if ( newClipInfo["video"] ) {
                            newClipInfo["uuid"] = "videoUuid";
                            vClipInfo = addClip( "Video", trackId, newClipInfo );
                        }
                    }
                    lastX = drag.x;
                }
                else
                    lastX = drag.source.x;
            }

            onPositionChanged: {
                // If resizing, ignore
                if ( drag.source.resizing === true )
                    return;

                if ( drag.keys.indexOf( "vlmc/uuid" ) >= 0 )
                    var dMode = dropMode.New;
                else
                    dMode = dropMode.Move;

                var toMove = selectedClips.concat();

                if ( dMode === dropMode.Move ) {
                    // Prepare newTrackId for all the selected clips
                    for ( var i = 0; i < toMove.length; ++i ) {
                        var target = findClipItem( toMove[i] );
                        if ( drag.source === target ) {
                            drag.source.parent.parent.z = ++maxZ;
                            var oldTrackId = target.newTrackId;
                            target.newTrackId = trackId;
                            for ( var j = 0; j < selectedClips.length; ++j ) {
                                if ( drag.source.uuid !== selectedClips[j] )
                                {
                                    var clip = findClipItem( selectedClips[j] );
                                    clip.newTrackId = Math.max( 0, trackId - oldTrackId + clip.trackId );
                                }
                            }
                        }
                        // Let's move to the new tracks
                        else if ( target.newTrackId !== target.trackId ) {
                                target.clipInfo["selected"] = true;
                                addClip( target.type, target.newTrackId, target.clipInfo );
                                removeClipFromTrack( target.type, target.trackId, target.uuid );
                        }
                    }

                    deltaX = drag.source.x - lastX;
                }
                else
                    deltaX = drag.x - lastX;

                while ( toMove.length > 0 ) {
                    target = findClipItem( toMove[0] );

                    var uuid = target.uuid;
                    var oldX = target.pixelPosition();
                    var newX = findNewPosition( Math.max( oldX + deltaX, 0 ), target, isMagneticMode );

                    // Let's find newX of the linked clip
                    for ( j = 0; j < target.linkedClips.length; ++j )
                    {
                        var linkedClipItem = findClipItem( target.linkedClips[j] );

                        if ( linkedClipItem ) {
                            var newLinkedClipX = findNewPosition( newX, linkedClipItem, isMagneticMode );

                            // If linked clip collides
                            if ( ptof( Math.abs( newLinkedClipX - newX ) ) !== 0 ) {
                                // Recalculate target's newX
                                // This time, don't use magnets
                                if ( isMagneticMode === true )
                                {
                                    newX = findNewPosition( newLinkedClipX, target, false );
                                    newLinkedClipX = findNewPosition( newX, target, false );

                                    // And if newX collides again, we don't move
                                    if ( ptof( Math.abs( newLinkedClipX - newX ) ) !== 0 )
                                        newX = oldX;
                                }
                                else
                                    newX = oldX;
                            }

                            if ( length < ptof( newX + linkedClipItem.width ) ) {
                                length = ptof( newX + linkedClipItem.width );
                            }

                            linkedClipItem.setPixelPosition( newX );
                            var ind = toMove.indexOf( linkedClipItem.uuid );
                            if ( ind > 0 )
                                toMove.splice( ind, 1 );
                        }
                    }

                    if ( length < ptof( newX + target.width ) ) {
                        length = ptof( newX + target.width );
                    }

                    target.setPixelPosition( newX );
                    toMove.splice( 0, 1 );

                    // Scroll if needed
                    if ( drag.source === target || dMode === dropMode.New )
                        target.scrollToThis();
                }
                // END of while ( toMove.length > 0 )

                if ( dMode === dropMode.Move )
                    lastX = drag.source.x;
                else
                    lastX = drag.x;
            }
        }

        Repeater {
            id: repeater
            model: clips
            delegate: Clip {
                height: track.height - 3
                name: model.name
                trackId: model.trackId
                type: track.type
                uuid: model.uuid
                libraryUuid: model.libraryUuid
                position: model.position
                lastPosition: model.lastPosition
                begin: model.begin
                end: model.end
                clipInfo: model
            }
        }
    }

    Rectangle {
        id: trackInfo
        x: sView.flickableItem.contentX
        width: initPosOfCursor
        height: parent.height
        color: "#444444"

        Rectangle {
            width: parent.width
            height: 1
            anchors.bottom: parent.bottom
            color: "#111111"
        }

        Rectangle {
            width: 1
            height: parent.height
            anchors.left: parent.left
            color: "#111111"
        }

        Rectangle {
            width: 1
            height: parent.height
            anchors.right: parent.right
            color: "#111111"
        }

        Text {
            id: trackText
            anchors.verticalCenter: parent.verticalCenter
            x: 10
            text: type + " " + ( trackId + 1 )
            color: "white"
            font.pointSize: 10
        }

        Row {
            anchors.verticalCenter: parent.verticalCenter
            x: trackText.y + trackText.contentWidth + 10
            spacing: 4

            PropertyButton {
                id: fxButton
                text: "Fx"
                selected: true

                onSelectedChanged: {
                    if ( selected === false ) {
                        workflow.showEffectStack( trackId );
                        selected = true;
                    }
                }
            }
        }
    }
}

