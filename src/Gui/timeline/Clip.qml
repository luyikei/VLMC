import QtQuick 2.0

Rectangle {
    id: clip
    // NEVER SET X DIRECTLY. BINDING WILL BE REMOVED.
    x: clipInfo ? ftop( clipInfo["position"] ) : 0
    width: clipInfo ? ftop( clipInfo["end"] - clipInfo["begin"] + 1 ) : 0
    z: 10001
    gradient: Gradient {
        GradientStop {
            id: gStop1
            position: 0.00;
            color: "#4276a6";
        }
        GradientStop {
            id: gStop2
            position: 1.00;
            color: "#1f546f";
        }
    }
    radius: 2
    border.color: "#1f546f"
    border.width: 1

    property alias name: text.text
    property int trackId
    // Usualy it is set -1. If not, the clip will be moved to the new track immediately.
    property int newTrackId: -1
    property string uuid
    property string type
    property bool selected: false

    property var clipInfo

    function setPixelPosition( pixels )
    {
        if ( pixels >= 0 )
            clipInfo["position"] = ptof( pixels );
    }

    function pixelPosition()
    {
        return ftop( clipInfo["position"] );
    }

    function move() {
        // This function updates Backend
        if ( newTrackId > -1 )
            moveClipTo( track.type, uuid, newTrackId );
        else
            workflow.moveClip( trackId, uuid, clipInfo["position"] )
    }

    function resize() {
        // This function updates Backend
        workflow.resizeClip( uuid, clipInfo["begin"], clipInfo["end"], clipInfo["position"] )
    }

    Component.onCompleted: {

        clipInfo["item"] = clip;
        clipInfo["track"] = track;

        selected = true;
    }

    Component.onDestruction: {
        selected = false;
    }

    Text {
        id: text
        color: "white"
        width: parent.width - 4
        x: 4
        y: 4 - font.pointSize / 2
        font.pointSize: trackHeight / 4
    }

    Drag.active: dragArea.drag.active
    Drag.hotSpot: Qt.point( width / 2 , height / 2 )
    Drag.keys: "Clip"
    MouseArea {
        id: dragArea
        anchors.fill: parent
        drag.target: resizing ? null : parent
        drag.minimumX: 0
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true
        cursorShape: Qt.OpenHandCursor

        property bool resizing: false

        onPositionChanged: {
           // If it's too short, don't resize.
            if ( width < 6 ) {
                resizing = false;
                return;
            }

            if ( dragArea.pressed === true ) {
                // Handle resizing
                if ( resizing === true ) {
                    if ( mouseX < width / 2 ) {
                        var newPos = ptof( clip.x + mouseX );
                        var newBegin = clipInfo["begin"] + ( newPos - clipInfo["position"] );
                        if ( newBegin < 0 || newPos < 0 || newBegin >= clipInfo["end"] )
                            return;
                        clipInfo["begin"] = newBegin;
                        clipInfo["position"] = newPos;
                    }
                    else {
                        var newEnd = ptof( mouseX + ftop( clipInfo["begin"] ) );
                        if ( newEnd <= clipInfo["begin"] || newEnd + 1 > clipInfo["length"] )
                            return;
                        clipInfo["end"] = newEnd;
                    }
                }
            }
            else {
                if ( mouseX < 3 || ( clip.width - mouseX ) < 3 )
                    resizing = true;
                else
                    resizing = false;
            }
        }

        onPressed: {
            if ( selected === true )
                return;

            if ( !( mouse.modifiers & Qt.ControlModifier ) )
                clearSelectedClips();

            if ( mouse.button & Qt.LeftButton )
                selected = true;
        }

        onClicked: {
            if ( mouse.button & Qt.RightButton ) {
                var menu = Qt.createComponent( "ClipContextMenu.qml" ).createObject( parent );
                menu.popup();
            }
        }

        onReleased: {
            for ( var i = 0; i < selectedClips.length; ++i )
                if ( selectedClips[i]["item"] ) {
                    if ( resizing === true )
                        selectedClips[i]["item"].resize();
                    else
                        selectedClips[i]["item"].move();
                }
        }

        states: [
            State {
                name: "Normal"
                when: !dragArea.pressed && !dragArea.resizing
                PropertyChanges { target: dragArea; cursorShape: Qt.OpenHandCursor }
            },
            State {
                name: "Resizing"
                when: dragArea.resizing
                PropertyChanges { target: dragArea; cursorShape: Qt.SizeHorCursor }
            },
            State {
                name: "Dragging"
                when: dragArea.pressed && !dragArea.resizing
                PropertyChanges { target: dragArea; cursorShape: Qt.ClosedHandCursor }
            }
        ]
    }

    onXChanged: {
        if ( x < 0 )
            setPixelPosition( 0 );
    }

    onYChanged: {
        // Don't move outside its TrackContainer
        // For Top
        var yToMoveUp = track.mapToItem( container, 0, 0 ).y + y;
        if ( yToMoveUp < 0 )
            y -= yToMoveUp;
        // For Bottom
        if ( yToMoveUp + height > container.height )
            y -= yToMoveUp - container.height + height;
    }

    onSelectedChanged: {
        if ( selected === true ) {
            selectedClips.push( clipInfo );
        }
        else {
            for ( var i = 0; i < selectedClips.length; ++i )
                if ( !selectedClips[i]["item"] || selectedClips[i]["item"] === clip ) {
                    selectedClips.splice( i, 1 );
                    --i;
                }
        }
    }

    states: [
        State {
            name: "Selected"
            when: selected
            PropertyChanges { target: gStop1; color: "#6498c8" }
            PropertyChanges { target: gStop2; color: "#427080" }
        },
        State {
            name: "Unselected"
            when: !selected
            PropertyChanges { target: gStop1; color: "#4276a6" }
            PropertyChanges { target: gStop2; color: "#1f546f" }
        }
    ]

    transitions: Transition {
        PropertyAnimation {
            properties: "color"
            easing.type: Easing.InOutQuad
            duration: 100
        }
    }
}

