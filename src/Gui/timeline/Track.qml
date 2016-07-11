import QtQuick 2.0

Item {
    id: track
    width: page.width
    height: trackHeight
    z: 10

    property int trackId
    property string type
    property ListModel clips

    Row {
        height: parent.height
        width: parent.width
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
                    }
                }

                onExited: {
                    if ( currentUuid ) {
                        removeClipFromTrack( "Audio", trackId, "tempUuid" );
                        removeClipFromTrack( "Video", trackId, "tempUuid" );
                    }
                }

                onEntered: {
                    if ( drag.keys.indexOf( "vlmc/uuid" ) >= 0 ) {
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

                            clearSelectedClips();
                            if ( newClipInfo["audio"] )
                                aClipInfo = addClip( "Audio", trackId, newClipInfo );
                            if ( newClipInfo["video"] )
                                vClipInfo = addClip( "Video", trackId, newClipInfo );
                        }
                    }
                }

                onPositionChanged: {
                    // Find the previous X of the clip
                    var newX = drag.x;
                    for ( var i = 0; i < selectedClips.length; ++i ) {
                        var target = selectedClips[i]["item"];

                        if ( !target )
                        {
                            selectedClips.splice( i, 1 );
                            --i;
                            continue;
                        }

                        if ( drag.keys.indexOf( "vlmc/uuid" ) < 0 )
                            newX = drag.source.x; // FIXME: Mysterious QML Bug, you can't use drag.x

                        var oldx = target.pixelPosition();
                        newX = Math.max( newX, 0 );

                        // Collision detection
                        var isCollided = true;
                        var clips = trackContainer( target.type )["tracks"].get( trackId )["clips"];
                        for ( var j = 0; j < clips.count + 2 && isCollided; ++j ) {
                            isCollided = false;
                            for ( var k = 0; k < clips.count; ++k ) {
                                var clip = clips.get( k );
                                if ( clip.uuid === target.uuid )
                                    continue;
                                var sw = target.width; // Width of the source clip
                                var cx = clip["item"].x;
                                var cw = clip["item"].width;
                                // Set a right position
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
                                if ( isCollided )
                                    break;
                            }
                        }

                        target.setPixelPosition( newX );

                        if ( isCollided ) {
                            for ( k = 0; k < clips.count; ++k ) {
                                clip = clips.get( k );
                                if ( clip["item"].x + clip["item"].width > target.pixelPosition() )
                                    target.setPixelPosition( clip["item"].x + clip["item"].width );
                            }
                        }

                        if ( drag.keys.indexOf( "vlmc/uuid" ) < 0 ) {

                            if ( target.trackId !== trackId )
                                target.newTrackId = trackId ;
                            else
                                target.newTrackId = -1;

                            if ( drag.source.uuid !== target.uuid ) {
                                if ( target.newTrackId !== -1 ) {
                                    addClip( target.type, target.newTrackId, target.clipInfo );
                                    removeClipFromTrack( target.type, target.trackId, target.uuid );
                                    --i;
                                }
                            }
                            else {
                                target.y = target.y - drag.y + track.height / 2 - 1;
                                target.parent.parent.parent.z = maxZ++; // FIXME: Ugly workaround for z-index
                            }
                        }
                    }
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
                    clipInfo: findClipFromTrack( track.type, model.trackId, model.uuid );
                }
            }
        }
    }
}

