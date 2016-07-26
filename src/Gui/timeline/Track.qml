import QtQuick 2.0

Item {
    id: track
    width: parent.width
    height: trackHeight
    z: 10

    property int trackId
    property string type
    property ListModel clips

    Row {
        anchors.fill: parent

        Rectangle {
            id: trackInfo
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
                text: type + " " + ( trackId + 1 )
                anchors.right: parent.right
                anchors.rightMargin: 10
                color: "white"
                font.pointSize: trackHeight / 3
            }
        }

        Rectangle {
            id: clipArea
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

                readonly property int magneticMargin: 25

                property string currentUuid
                property var aClipInfo: null
                property var vClipInfo: null

                property int lastX: 0
                property int deltaX: 0

                function findNewPosition( newX, target, useMagneticMode ) {
                    var oldX = target.pixelPosition();

                    if ( useMagneticMode === true ) {
                        var leastDestance = magneticMargin;
                        // Check two times
                        for ( var k = 0; k < 2; ++k ) {
                            for ( var j = 0; j < markers.count; ++j ) {
                                var mx = ftop( markers.get( j ).position );
                                if ( Math.abs( newX - mx ) < leastDestance ) {
                                    leastDestance = Math.abs( newX - mx );
                                    newX = mx;
                                }
                                else if ( Math.abs( newX + target.width - mx ) < leastDestance ) {
                                    leastDestance = Math.abs( newX + target.width - mx );
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
                        clips = [];
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
                            if ( useMagneticMode === true ) {
                                if ( cx  + cw > newX && newX + sw > cx )
                                    isCollided = true;

                                cw += magneticMargin * 2
                                cx -= magneticMargin
                                if ( cx + cw > newX && newX + sw > cx ) {
                                    if ( cx > newX ) {
                                        if ( cx - sw > 0 )
                                            newX = cx - sw + magneticMargin;
                                        else
                                            newX = oldX;
                                    } else {
                                        newX = cx + cw - magneticMargin;
                                    }
                                }
                            }
                            else {
                                if ( cx  + cw > newX && newX + sw > cx ) {
                                    isCollided = true;
                                    if ( cx > newX ) {
                                        if ( cx - sw > 0 )
                                            newX = cx - sw;
                                        else
                                            newX = oldX;
                                    } else {
                                        newX = cx + cw;
                                    }
                                }
                            }
                            if ( isCollided )
                                break;
                        }
                    }

                    if ( isCollided ) {
                        for ( k = 0; k < clips.count; ++k ) {
                            clip = clips.get( k );
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
                        if ( aClipInfo ) {
                            var pos = aClipInfo["position"];
                            var audioClipUuid = workflow.addClip( currentUuid, trackId, pos, true );
                            removeClipFromTrack( "Audio", trackId, "audioUuid" );
                        }
                        if ( vClipInfo ) {
                            pos = vClipInfo["position"];
                            var videoClipUuid = workflow.addClip( currentUuid, trackId, pos, false );
                            removeClipFromTrack( "Video", trackId, "videoUuid" );
                        }
                        if ( audioClipUuid && videoClipUuid ) {
                            workflow.linkClips( audioClipUuid, videoClipUuid );
                        }
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
                            var newClipInfo = workflow.clipInfo( drag.getDataAsString( "vlmc/uuid" ) );
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

                    if ( drag.keys.indexOf( "vlmc/uuid" ) < 0 ) {

                        // Put drag.source top
                        for ( var i = 1; i < selectedClips.length; ++i ) {
                            if ( selectedClips[i] === drag.source ) {
                                selectedClips.splice( i, 1 );
                                selectedClips.unshift( drag.source );
                            }
                        }

                        // Optimization: Delta delta X should be different
                        if ( deltaX === drag.source.x - lastX ) {
                            lastX = drag.source.x;
                            return;
                        }
                        else
                            deltaX = drag.source.x - lastX;

                    }
                    else
                        deltaX = drag.x - lastX;

                    var alreadyCalculated = []; // Uuids of clips being already set new x position.
                    for ( i = 0; i < selectedClips.length; ++i ) {
                        var target = selectedClips[i];
                        if ( drag.source === target ) {
                            var oldTrackId = target.newTrackId;
                            target.newTrackId = trackId;
                            for ( var j = 0; j < selectedClips.length; ++j ) {
                                if ( drag.source !== selectedClips[j] )
                                    selectedClips[j].newTrackId = trackId - oldTrackId + selectedClips[j].trackId;
                            }
                        }

                        if ( alreadyCalculated.indexOf( target.uuid ) < 0 ) {
                            var oldX = target.pixelPosition();
                            var newX = Math.max( oldX + deltaX, 0 );

                            // Recalculate deltaX in case of drag.source being moved
                            if ( drag.source === target ) {
                                if ( oldTrackId === target.newTrackId )
                                    deltaX = Math.round( newX - oldX );
                                else
                                    // Don't move other clips if drag.source's track is changed
                                    deltaX = 0;
                            }

                            newX = findNewPosition( newX, target, isMagneticMode );

                            // Let's find newX of the linked clip
                            if ( target.linked === true ) {
                                var linkedClipItem = findClipItem( target.linkedClip );

                                if ( linkedClipItem ) {
                                    var newLinkedClipX = findNewPosition( newX, linkedClipItem, isMagneticMode );

                                    // If linked clip collides
                                    if ( Math.abs( newLinkedClipX - newX ) > 1 ) {

                                        // Recalculate target's newX
                                        // This time, don't use magnets
                                        newX = findNewPosition( newLinkedClipX, target, false );
                                        newLinkedClipX = findNewPosition( newX, target, false );

                                        // And if newX collides again, we don't move
                                        if ( Math.abs( newLinkedClipX - newX ) > 1 )
                                            newX = oldX;
                                    }
                                    linkedClipItem.setPixelPosition( newX );
                                    alreadyCalculated.push( target.linkedClip );
                                }
                            }
                            target.setPixelPosition( newX );
                            alreadyCalculated.push( target.uuid );
                        }

                        // Scroll if needed
                        if ( length < ptof( newX ) ) {
                            length = ptof( newX );
                            // Never show the background behind the timeline
                            var newContentX = sView.flickableItem.contentWidth - sView.width;
                            if ( newContentX >= 0 )
                                sView.flickableItem.contentX = newContentX;
                        }

                        if ( drag.keys.indexOf( "vlmc/uuid" ) < 0 ) {
                            if ( target.newTrackId !== target.trackId ) {
                                drag.source.parent.parent.parent.z = ++maxZ;
                                if ( drag.source.uuid !== target.uuid ) {
                                    target.clipInfo["selected"] = true;
                                    addClip( target.type, target.newTrackId, target.clipInfo );
                                    removeClipFromTrack( target.type, target.trackId, target.uuid );
                                    --i;
                                }
                            }
                        }
                    }
                    // END of for ( var i = 0; i < selectedClips.length; ++i )

                    if ( drag.keys.indexOf( "vlmc/uuid" ) < 0 )
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
                    position: model.position
                    begin: model.begin
                    end: model.end
                    linkedClip: model.linkedClip
                    clipInfo: model
                }
            }
        }
    }
}

