import QtQuick 2.0

Rectangle {
    id: clip
    // NEVER SET X DIRECTLY. BINDING WILL BE REMOVED.
    x: ftop( position )
    width: ftop( end - begin + 1 )
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
    // Usualy it is trackId, the clip will be moved to the new track immediately.
    property int newTrackId
    property int position
    property int begin
    property int end
    property string uuid
    property string linkedClip // Uuid
    property bool linked: false
    property string type
    property bool selected: false

    property var clipInfo

    function setPixelPosition( pixels )
    {
        if ( pixels >= 0 )
            position = ptof( pixels );
        // FIXME: Binding can be lost because of dragging.
        x = Qt.binding( function() { return ftop( position ); } );
    }

    function pixelPosition()
    {
        return ftop( position );
    }

    function move() {
        moveClipTo( track.type, uuid, newTrackId, position );
    }

    function resize() {
        // This function updates Backend
        workflow.resizeClip( uuid, begin, end, position )
    }

    function selectLinkedClip() {
        if ( selected === true && linked === true && linkedClip )
            findClipItem( linkedClip ).selected = true;
    }

    onXChanged: {
        if ( sView.width - initPosOfCursor < width )
            return;

        if ( sView.flickableItem.contentX + sView.width <
                x + width + initPosOfCursor + sView.sViewPadding )
            Drag.hotSpot.x = 0;
        else if ( sView.flickableItem.contentX + sView.sViewPadding > x )
            Drag.hotSpot.x = width;
    }

    onYChanged: {
        y -= y % trackHeight;
        // Don't move outside its TrackContainer
        // For Top
        var yToMoveUp = track.mapToItem( container, 0, 0 ).y + y;
        if ( yToMoveUp < 0 )
            y -= yToMoveUp;
        // For Bottom
        if ( yToMoveUp + height > container.height )
            y -= yToMoveUp - container.height + height;
    }

    onClipInfoChanged: {
        if ( !clipInfo )
            return;

        position = clipInfo["position"];
        begin = clipInfo["begin"];
        end = clipInfo["end"];
        linkedClip = clipInfo["linkedClip"];
    }

    onPositionChanged: {
        clipInfo["position"] = position;
    }

    onBeginChanged: {
        clipInfo["begin"] = begin;
    }

    onEndChanged: {
        clipInfo["end"] = end;
    }

    onLinkedClipChanged: {
        clipInfo["linkedClip"] = linkedClip;
        if ( linkedClip ) {
            linked = true;
            var linkedClipItem = findClipItem( linkedClip );
            if ( linkedClipItem ) {
                linkedClipItem.linkedClip = clip.uuid;
                linkedClipItem.linked = true;
            }
        }
        else
            linked = false;
    }

    onLinkedChanged: {
        selectLinkedClip();

        if ( !linkedClip )
            return;

        var linkedClipItem = findClipItem( linkedClip );
        if ( !linkedClipItem )
            return;

        if ( linked === true )
            linkedClipItem.linked = true;
        else
            linkedClipItem.linked = false;
    }

    onSelectedChanged: {
        for ( var i = 0; i < selectedClips.length; ++i )
            if ( !selectedClips[i] || selectedClips[i] === clip ) {
                selectedClips.splice( i, 1 );
                --i;
            }

        if ( selected === true ) {
            selectedClips.push( clip );

            var group = findGroup( uuid );
            for ( i = 0; i < ( group ? group.length : 0 ); ++i ) {
                var clipItem = findClipItem( group[i] );
                if ( clipItem )
                    clipItem.selected = true;
            }
            selectLinkedClip();
        }
    }

    Component.onCompleted: {
        if ( clipInfo["selected"] === false )
            selected = false;
        else
            selected = true;
        newTrackId = trackId;
        allClips.push( clip );
    }

    Component.onDestruction: {
        Drag.drop();
        selected = false;

        if ( linkedClip ) {
            var linkedClipItem = findClipItem( linkedClip );
            if ( linkedClipItem )
                linkedClipItem.linkedClip = "";
        }

        for ( var i = 0; i < allClips.length; ++i ) {
            if ( allClips[i] === clip ) {
                allClips.splice( i, 1 );
                return;
            }
        }
    }

    Drag.keys: ["Clip"]
    Drag.active: dragArea.drag.active

    Text {
        id: text
        color: "white"
        width: parent.width - 4
        height: trackHeight
        x: 4
        y: 4 - font.pointSize / 2
        font.pointSize: trackHeight / 4
        elide: Text.ElideRight
        wrapMode: Text.Wrap
    }

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
                        var newBegin = begin + ( newPos - position );
                        if ( newBegin < 0 || newPos < 0 || newBegin >= end )
                            return;
                        begin = newBegin;
                        position = newPos;
                    }
                    else {
                        var newEnd = ptof( mouseX + ftop( begin ) );
                        if ( newEnd <= begin || newEnd + 1 > clipInfo["length"] )
                            return;
                        end = newEnd;
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
            clip.Drag.hotSpot = Qt.point( mouseX, clip.height / 2 );

            if ( selected === true )
                return;

            if ( !( mouse.modifiers & Qt.ControlModifier ) )
                clearSelectedClips();

            if ( mouse.button & Qt.LeftButton )
                selected = true;
        }

        onClicked: {
            if ( mouse.button & Qt.RightButton ) {
                clipContextMenu.popup();
            }
        }

        onReleased: {
            if ( resizing === true )
                resize();
            else
                dragFinished();
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

    DropArea {
        id: effectDropArea
        anchors.fill: parent
        keys: ["vlmc/effect_name"]

        onDropped: {
            workflow.addEffect( uuid, drop.getDataAsString( "vlmc/effect_name" ) );
        }
    }

    ClipContextMenu {
        id: clipContextMenu
        clip: clip
    }

    states: [
        State {
            name: "EffectDrop"
            when: effectDropArea.containsDrag
            PropertyChanges { target: gStop1; color: "#427080" }
            PropertyChanges { target: gStop2; color: "#225060" }
        },
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

