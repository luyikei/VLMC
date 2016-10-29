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
    property alias thumbnailSource: thumbnailImage.source
    property int trackId
    // Usualy it is trackId, the clip will be moved to the new track immediately.
    property int newTrackId
    property int position
    property int begin
    property int end
    property string libraryUuid // Library UUID: For thumbnails
    property string uuid // Instance UUID
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
        var _length = selectedClips.length;
        for ( var i = _length - 1; i >= 0; --i ) {
            if ( selectedClips[i] ) {
                workflow.resizeClip( selectedClips[i].uuid, begin, end, position );
            }
        }
    }

    function selectLinkedClip() {
        if ( selected === true && linked === true && linkedClip )
            findClipItem( linkedClip ).selected = true;
    }

    function updateEffects( clipInfo ) {
        if ( !clipInfo["filters"] )
            return;

        var str = "";
        for ( var i = 0; i < clipInfo["filters"].length; ++i ) {
            str += clipInfo["filters"][i]["identifier"];
            if ( i < clipInfo["filters"].length - 1 )
                str += ", "
        }
        effectsItem.text = str;
    }

    function updateThumbnail( pos ) {
        thumbnailSource = "image://thumbnail/" + libraryUuid + "/" + pos;
    }

    function resizeLinkedClips( oldPos, oldBegin, oldEnd ) {
        if ( !linkedClip )
            return;
        var lc = findClipItem( linkedClip );
        if ( lc === null )
            return;
        // Don't resize from the begining if the clips didn't shared the same begin position
        if ( lc.position === oldPos ) {
            lc.position = position;
            lc.begin = begin;
        }
        if ( lc.end === oldEnd )
            lc.end = end;
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

        updateEffects( workflow.clipInfo( uuid ) );

        if ( uuid === "videoUuid" || uuid === "audioUuid" )
            return;

        if ( thumbnailProvider.hasImage( uuid, begin ) )
            updateThumbnail( begin );
        else
            workflow.takeThumbnail( uuid, begin );
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

    ListModel {
        id: effects
    }

    Text {
        id: text
        color: "white"
        width: parent.width - 4
        height: font.pointSize + 4
        x: 4
        y: 4 - font.pointSize / 2
        font.pointSize: 7
        elide: Text.ElideRight
        wrapMode: Text.Wrap
    }

    Image {
        id: thumbnailImage
        x: 4
        anchors.top: text.bottom
        anchors.bottom: effectsItem.visible ? effectsItem.top : clip.bottom
        anchors.topMargin: 4
        anchors.bottomMargin: 4
        fillMode: Image.PreserveAspectFit
        visible: width < clip.width
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
            if ( isCutMode === true )
                return;

            // If it's too short, don't resize.
            if ( width < 6 ) {
                resizing = false;
                return;
            }

            if ( dragArea.pressed === true ) {
                // Handle resizing
                if ( resizing === true ) {
                    var oldPos = position;
                    var oldBegin = begin;
                    var oldEnd = end;
                    if ( mouseX < width / 2 ) {
                        var newPos = position + ptof( mouseX );
                        var newBegin = begin + ptof( mouseX );
                        if ( newBegin < 0 || newPos < 0 || newBegin >= end )
                            return;
                        begin = newBegin;
                        position = newPos;
                    }
                    else {
                        var newEnd = begin + ptof( mouseX );
                        if ( newEnd <= begin || newEnd + 1 > clipInfo["length"] )
                            return;
                        end = newEnd;
                    }
                    resizeLinkedClips(oldPos, oldBegin, oldEnd);
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
            else if ( isCutMode === true ) {
                var newClipPos = position + ptof( mouseX );
                var newClipBegin = begin + ptof( mouseX );
                if ( newClipPos - position < 1 || end - newClipBegin < 1 )
                    return;
                workflow.splitClip( uuid, newClipPos, newClipBegin );
            }
        }

        onReleased: {
            // Don't trigger event if the mouse didn't move
            if ( resizing === true && isCutMode === false )
                resize();
            else if ( dragArea.drag.active )
            {
                dragFinished();
            }
        }

        states: [
            State {
                name: "Normal"
                when: isCutMode
                PropertyChanges { target: dragArea; cursorShape: Qt.ArrowCursor }
            },
            State {
                name: "Move"
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

    Rectangle {
        id: effectsItem
        height: 6
        width: clip.width
        anchors.bottom: clip.bottom
        visible: effectsTextItem.text ? true : false
        radius: 2
        gradient: Gradient {
            GradientStop {
                id: eGStop1
                position: 0.00;
            }
            GradientStop {
                id: eGStop2
                position: 1.00;
            }
        }

        property alias text: effectsTextItem.text

        Text {
            id: effectsTextItem
            width: clip.width
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: parent.height - 2
            anchors.centerIn: parent
            elide: Text.ElideRight
        }

        MouseArea {
            id: effectsItemMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                workflow.showEffectStack( uuid );
            }
        }

        states: [
            State {
                when: effectsItemMouseArea.pressed
                PropertyChanges { target: eGStop1; color: "#220f3c" }
                PropertyChanges { target: eGStop2; color: "#24245c" }
            },
            State {
                when: effectsItemMouseArea.containsMouse
                PropertyChanges { target: eGStop1; color: "#46469F" }
                PropertyChanges { target: eGStop2; color: "#32215a" }
            },
            State {
                when: !effectsItemMouseArea.containsMouse
                PropertyChanges { target: eGStop1; color: "#24245c" }
                PropertyChanges { target: eGStop2; color: "#200f3c" }
            }
        ]
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

