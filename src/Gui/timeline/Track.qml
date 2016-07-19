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

                property string currentUuid
                property var aClipInfo: null
                property var vClipInfo: null

                onDropped: {
                    if ( drop.keys.indexOf( "vlmc/uuid" ) >= 0 ) {
                        aClipInfo = findClipFromTrack( "Audio", trackId, "tempUuid" );
                        vClipInfo = findClipFromTrack( "Video", trackId, "tempUuid" );
                        if ( aClipInfo ) {
                            var pos = aClipInfo["position"];
                            removeClipFromTrack( "Audio", trackId, "tempUuid" );
                            addClip( "Audio", trackId, workflow.clipInfo( workflow.addClip( currentUuid, trackId, pos, true ) ) );
                        }
                        if ( vClipInfo ) {
                            pos = vClipInfo["position"];
                            removeClipFromTrack( "Video", trackId, "tempUuid" );
                            addClip( "Video", trackId, workflow.clipInfo( workflow.addClip( currentUuid, trackId, pos, false ) ) );
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
                        removeClipFromTrack( "Audio", trackId, "tempUuid" );
                        removeClipFromTrack( "Video", trackId, "tempUuid" );
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
                            newClipInfo["uuid"] = "tempUuid";
                            if ( newClipInfo["audio"] )
                                aClipInfo = addClip( "Audio", trackId, newClipInfo );
                            if ( newClipInfo["video"] )
                                vClipInfo = addClip( "Video", trackId, newClipInfo );
                        }
                        lastX = drag.x;
                    }
                }

                onPositionChanged: {
                    // If resizing, ignore
                    if ( drag.source.resizing === true )
                        return;

                    if ( drag.keys.indexOf( "vlmc/uuid" ) < 0 ) {
                        drag.source.y = drag.source.y - drag.y + track.height / 2 - 1; // Adjust to the center
                        var currentX = drag.source.x;
                    }
                    else
                        currentX = drag.x;

                    var deltaX = currentX - lastX;
                    lastX = currentX;

                    for ( var i = 0; i < selectedClips.length; ++i ) {
                        var target = selectedClips[i];
                        var oldx = target.pixelPosition();

                        if ( drag.source === target ) {
                            var newX = drag.source.x;

                            var oldTrackId = target.newTrackId;
                            target.newTrackId = trackId;
                            for ( var j = 0; j < selectedClips.length; ++j ) {
                                if ( drag.source !== selectedClips[j] )
                                    selectedClips[j].newTrackId = trackId - oldTrackId + selectedClips[j].trackId;
                            }
                        }
                        else
                            newX = Math.max( target.pixelPosition() + deltaX, 0 );

                        if ( isMagneticMode === true ) {
                            var leastDestance = 25;
                            // Check two times
                            for ( var k = 0; k < 2; ++k ) {
                                for ( j = 0; j < markers.count; ++j ) {
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
                                if ( isMagneticMode === true ) {
                                    if ( cx  + cw > newX && newX + sw > cx )
                                        isCollided = true;

                                    cw += 50
                                    cx -= 25
                                    if ( cx + cw > newX && newX + sw > cx ) {
                                        if ( cx > newX ) {
                                            if ( cx - sw > 0 )
                                                newX = cx - sw + 25;
                                            else
                                                newX = oldx;
                                        } else {
                                            newX = cx + cw - 25;
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
                                                newX = oldx;
                                        } else {
                                            newX = cx + cw;
                                        }
                                    }
                                }
                                if ( isCollided )
                                    break;
                            }
                        }

                        // Scroll if needed
                        if ( length < ptof( newX + target.width ) ) {
                            length = ptof( newX + target.width );
                            // Never show the background behind the timeline
                            var newContentX = sView.flickableItem.contentWidth - sView.width;
                            if ( newContentX >= 0 )
                                sView.flickableItem.contentX = newContentX;
                        }

                        if ( isCollided ) {
                            for ( k = 0; k < clips.count; ++k ) {
                                clip = clips.get( k );
                                cx = ftop( clip["position"] );
                                cw = ftop( clip["end"] - clip["begin"] + 1);
                                newX = Math.max( newX, cx + cw );
                            }
                        }

                        if ( target === drag.source )
                            lastX = drag.source.x;

                        target.setPixelPosition( newX );

                        if ( drag.keys.indexOf( "vlmc/uuid" ) < 0 ) {
                            if ( target.newTrackId !== target.trackId ) {
                                drag.source.parent.parent.parent.z = ++maxZ;
                                if ( drag.source.uuid !== target.uuid ) {
                                    addClip( target.type, target.newTrackId, target.clipInfo );
                                    removeClipFromTrack( target.type, target.trackId, target.uuid );
                                    --i;
                                }
                            }
                        }
                    }
                    // END of for ( var i = 0; i < selectedClips.length; ++i )
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
                    clipInfo: model
                }
            }
        }
    }
}

