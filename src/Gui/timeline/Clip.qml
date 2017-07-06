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
    property int lastPosition
    property int begin
    property int end
    property string libraryUuid // Library UUID: For thumbnails
    property string uuid // Instance UUID
    property var linkedClips: linkedClipsDict[uuid] // Uuid
    property string type
    property bool selected: false
    property alias mouseX: dragArea.mouseX

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

    function resize() {
        // This function updates Backend
        workflow.resizeClip( uuid, begin, end, position );
    }

    function selectLinkedClip() {
        if ( selected === false )
            return;
        for ( var i = 0; i < linkedClips.length; ++i )
        {
            var linkedClip = linkedClips[i];
            findClipItem( linkedClip ).selected = true;
        }
    }

    function linked() {
        if ( selectedClips.length < 2 )
            return false;
        for ( var i = 0; i < selectedClips.length; ++i ) {
            for ( var j = i + 1; j < selectedClips.length; ++j ) {
                if ( selectedClips[i].linkedClips.indexOf( selectedClips[j].uuid ) === -1 )
                    return false;
            }
        }
        return true;
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

    function resizeLinkedClips( oldPos, oldBegin, oldEnd ) {
        for ( var i = 0; i < linkedClips.length; ++i )
        {
            var linkedClip = linkedClips[i];
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
    }

    function scrollToThis() {
        if ( width > sView.width )
            return;

        var newContentX = sView.flickableItem.contentX;

        if ( sView.flickableItem.contentX + sView.width <
                x + mouseX + initPosOfCursor + sView.sViewPadding )
            newContentX = x + mouseX + initPosOfCursor + sView.sViewPadding - sView.width;
        else if ( sView.flickableItem.contentX + sView.sViewPadding > x + mouseX )
            newContentX = x + mouseX - sView.sViewPadding;

        sView.flickableItem.contentX = Math.max( newContentX, 0 );
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

    onLinkedClipsChanged: {
        for ( var i = 0; i < linkedClips.length; ++i )
        {
            var linkedClip = linkedClips[i];
            var linkedClipItem = findClipItem( linkedClip );
            if ( linkedClipItem ) {
                if ( linkedClipItem.linkedClips.indexOf( uuid ) !== -1 )
                    linkedClipItem.linkedClips.push( uuid );
            }
        }
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
        for ( var i = 0; i < allClips.length; ++i ) {
            if ( allClips[i].linkedClips.indexOf( uuid ) !== -1 )
                linkedClips.push( allClips[i].uuid );
        }
        if ( clipInfo["selected"] === false )
            selected = false;
        else
            selected = true;
        newTrackId = trackId;
        allClips.push( clip );
        allClipsDict[uuid] = clip;

        updateEffects( workflow.clipInfo( uuid ) );

        if ( uuid === "videoUuid" || uuid === "audioUuid" )
            return;
        thumbnailSource = "image://thumbnail/" + libraryUuid + "/0";
    }

    Component.onDestruction: {
        Drag.drop();
        selected = false;

        for ( var i = 0; i < linkedClips.length; ++i )
        {
            var linkedClip = linkedClips[i];
            var linkedClipItem = findClipItem( linkedClip );
            if ( linkedClipItem )
                for ( var j = 0; j < linkedClipItem.linkedClips.length; ++j )
                {
                    if ( linkedClipItem.linkedClips[j] === uuid )
                    {
                        linkedClipItem.linkedClips.splice( j, 1 );
                        break;
                    }
                }
        }

        for ( i = 0; i < allClips.length; ++i ) {
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
        asynchronous: true
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
                dragFinished();
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

