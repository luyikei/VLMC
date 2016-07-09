import QtQuick 2.0

Rectangle {
    id: clip
    // NEVER SET X DIRECTLY. BINDING WILL BE REMOVED.
    x: clipInfo ? ftop( clipInfo["begin"] + clipInfo["position"] ) : 0
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
        clipInfo["position"] = ptof( pixels ) - clipInfo["begin"];
    }

    function pixelPosition( pixels )
    {
        return ftop( clipInfo["begin"] + clipInfo["position"] );
    }

    function move() {
        // This function updates Backend
        if ( newTrackId > -1 )
            moveClipTo( track.type, uuid, newTrackId );
        else
            workflow.moveClip( trackId, uuid, clipInfo["position"] )
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
        drag.target: parent
        drag.minimumX: 0
        acceptedButtons: Qt.LeftButton | Qt.RightButton

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
                if ( selectedClips[i]["item"] )
                    selectedClips[i]["item"].move();
        }
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
                if ( !selectedClips[i]["item"] || selectedClips[i]["item"] === clip )
                    selectedClips.splice( i, 1 );
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

