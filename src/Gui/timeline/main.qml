import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2

Rectangle {
    id: page
    anchors.fill: parent
    color: "#777777"
    border.width: 0
    focus: true

    property int length // in frames
    property int cursorPosition: 0 // in frames
    property int initPosOfCursor: 150
    property double ppu: 10 // Pixels Per minimum Unit
    property double unit: 3000 // In milliseconds therefore ppu / unit = Pixels Per milliseconds
    property double fps: 29.97
    property int maxZ: 100
    property int scale: 4
    property var allClips: [] // Actual clip item objects
    property var allClipsDict: ({}) // Actual clip item objects
    property var selectedClips: [] // Selected clip uuids
    property var groups: [] // list of lists of clip uuids
    property var linkedClipsDict: ({}) // Uuid
    property alias isMagneticMode: magneticModeButton.selected
    property bool isCutMode: false
    property bool dragging: false

    property int trackHeight: 60

    function clearSelectedClips() {
        while ( selectedClips.length ) {
            var clip = findClipItem( selectedClips.pop() );
            if ( clip )
                clip.selected = false;
        }
    }

    function zerofill( number, width ) {
        var str = "" + number;
        while ( str.length < width ) {
            str = "0" + str;
        }
        return str;
    }

    function timecodeFromFrames( frames ) {
        var seconds = Math.floor( frames / Math.round( fps )  );
        var minutes = Math.floor( seconds / 60 );
        var hours = Math.floor( minutes / 60 );

        return zerofill( hours, 3 ) + ':' + // hours
                zerofill( minutes % 60, 2 ) + ':' + // minutes
                zerofill( seconds % 60, 2 ) + ':' + // seconds
                // The second Math.round prevents the first value from exceeding fps.
                // e.g. 30 % Math.round( 29.97 ) = 0
                zerofill( Math.floor( frames % Math.round( fps ) ), 2 ); // frames in a minute
    }

    // Convert length in frames to pixels
    function ftop( frames )
    {
        return frames / fps * 1000 * ppu / unit;
    }

    // Convert length in pixels to frames
    function ptof( pixels )
    {
        return Math.round( pixels * fps / 1000 / ppu * unit );
    }

    function trackContainer( trackType )
    {
        if ( trackType === "Video" )
            return trackContainers.get( 0 );
        return trackContainers.get( 1 );
    }

    function addTrack( trackType )
    {
        trackContainer( trackType )["tracks"].append( { "clips": [] } );
    }

    function removeTrack( trackType )
    {
        var tracks = trackContainer( trackType )["tracks"];
        tracks.remove( tracks.count - 1 );
    }

    function addClip( trackType, trackId, clipDict )
    {
        var newDict = {};
        newDict["begin"] = clipDict["begin"];
        newDict["end"] = clipDict["end"];
        newDict["position"] = clipDict["position"];
        newDict["lastPosition"] = clipDict["position"];
        newDict["length"] = clipDict["length"];
        newDict["libraryUuid"] = clipDict["libraryUuid"];
        newDict["uuid"] = clipDict["uuid"];
        newDict["trackId"] = trackId;
        newDict["name"] = clipDict["name"];
        newDict["selected"] = clipDict["selected"] === false ? false : true ;
        var tracks = trackContainer( trackType )["tracks"];
        while ( trackId > tracks.count - 1 )
            addTrack( trackType );
        tracks.get( trackId )["clips"].append( newDict );
        return newDict;
    }

    function removeClipFromTrack( trackType, trackId, uuid )
    {
        var ret = false;
        var tracks = trackContainer( trackType )["tracks"];
        var clips = tracks.get( trackId )["clips"];

        for ( var j = 0; j < clips.count; j++ ) {
            var clip = clips.get( j );
            if ( clip.uuid === uuid ) {
                clips.remove( j );
                ret = true;
                j--;
            }
        }
        return ret;
    }

    function removeClipFromTrackContainer( trackType, uuid )
    {
        for ( var i = 0; i < trackContainer( trackType )["tracks"].count; i++  )
            removeClipFromTrack( trackType, i, uuid );
    }

    function removeClip( uuid )
    {
        removeClipFromTrackContainer( "Audio", uuid );
        removeClipFromTrackContainer( "Video", uuid );
    }

    function findClipFromTrackContainer( trackType, uuid )
    {
        var tracks = trackContainer( trackType )["tracks"];
        for ( var i = 0; i < tracks.count; i++  ) {
            var clip = findClipFromTrack( trackType, i, uuid );
            if( clip )
                return clip;
        }

        return null;
    }

    function findClipFromTrack( trackType, trackId, uuid )
    {
        var clips = trackContainer( trackType )["tracks"].get( trackId )["clips"];
        for ( var j = 0; j < clips.count; j++ ) {
            var clip = clips.get( j );
            if ( clip.uuid === uuid )
                return clip;
        }
        return null;
    }

    function findClip( uuid )
    {
        var v = findClipFromTrackContainer( "Video", uuid );
        if ( !v )
            return findClipFromTrackContainer( "Audio", uuid );
        return v;
    }

    function findClipItem( uuid ) {
        return allClipsDict[uuid];
    }

    function adjustTracks( trackType ) {
        var tracks = trackContainer( trackType )["tracks"];

        while ( tracks.count > 1 && tracks.get( tracks.count - 1 )["clips"].count === 0 &&
               tracks.get( tracks.count - 2 )["clips"].count === 0 )
            removeTrack( trackType );

        if ( tracks.get( tracks.count - 1 )["clips"].count > 0 )
            addTrack( trackType );
    }

    function addMarker( pos ) {
        markers.append( {
                           "position": pos
                       } );
    }

    function findMarker( pos ) {
        for ( var i = 0; i < markers.count; ++i ) {
            if ( markers.get( i )["position"] === pos ) {
                return markers.get( i );
            }
        }
        return null;
    }

    function removeMarker( pos ) {
        for ( var i = 0; i < markers.count; ++i ) {
            if ( markers.get( i )["position"] === pos ) {
                markers.remove( i );
                return;
            }
        }
    }

    function addGroup( clips ) {
        groups.push( clips );
    }

    function findGroup( uuid ) {
        for ( var i = 0; i < groups.length; ++i ) {
            var group = groups[i];
            for ( var j = 0; j < group.length; ++j ) {
                if ( group[j] === uuid )
                    return group;
            }
        }
        return null;
    }

    function removeGroup( uuid ) {
        for ( var i = 0; i < groups.length; ++i ) {
            var group = groups[i];
            for ( var j = 0; j < group.length; ++j ) {
                if ( group[j] === uuid ) {
                    groups.splice( i, 1 );
                    return;
                }
            }
        }
    }

    function updateLinkedClips( uuid ) {
        var item = findClipItem( uuid );
        if ( item )
            item.linkedClips = linkedClipsDict[uuid];
    }

    function zoomIn( ratio, scrollToCuror ) {
        var newPpu = ppu;
        var newUnit = unit;
        newPpu *= ratio;
        var contentXPos = ptof( sView.flickableItem.contentX );

        // Don't be too narrow.
        while ( newPpu < 10 ) {
            newPpu *= 2;
            newUnit *= 2;
        }

        // Don't be too distant.
        while ( newPpu > 20 ) {
            newPpu /= 2;
            newUnit /= 2;
        }

        // Can't be more precise than 1000msec / fps.
        var mUnit = 1000 / fps;

        if ( newUnit < mUnit ) {
            newPpu /= ratio; // Restore the original scale.
            newPpu *= mUnit / newUnit;
            newUnit = mUnit;
        }

        // Make unit a multiple of 1 / fps.
        newPpu *= ( newUnit - ( newUnit % mUnit ) ) / newUnit;
        newUnit -= newUnit % mUnit;

        // If "almost" the same value, don't bother redrawing the ruler.
        if ( Math.abs( unit - newUnit ) > 0.01 )
            unit = newUnit;

        if ( Math.abs( ppu - newPpu ) > 0.0001 )
            ppu = newPpu;

        if ( scrollToCuror === true ) {
            // Let's scroll to the cursor position!
            var newContentX = cursor.x - sView.width / 2;
            // Never show the background behind the timeline
            if ( newContentX >= 0 && sView.flickableItem.contentWidth - newContentX > sView.width  )
                sView.flickableItem.contentX = newContentX;
        }
        else {
            sView.flickableItem.contentX = ftop( contentXPos );
        }


        scale = Math.floor( Math.log( newUnit / mUnit ) / Math.log( 2 ) - 1 );
        scale = Math.min( 9, scale );
        scale = Math.max( 0, scale );
        mainwindow.setScale( scale );
    }

    // Sort clips in a manner that clips won't overlap each other while they are being moved
    function sortSelectedClips() {
        // Workaround: We cannot sort selectedClips directly maybe because of a Qt bug
        var sorted = selectedClips.slice();
        sorted.sort(
                    function( clipAUuid, clipBUuid )
                    {
                        var clipA = findClipItem( clipAUuid );
                        var clipB = findClipItem( clipBUuid );
                        if ( clipA.newTrackId > clipA.trackId )
                        {
                            return - ( clipA.newTrackId - clipB.newTrackId );
                        }
                        else if ( clipA.newTrackId < clipA.trackId )
                        {
                            return clipA.newTrackId - clipB.newTrackId;
                        }
                        else if ( clipA.position > clipA.lastPosition )
                        {
                            return - ( clipA.position - clipB.position );
                        }
                        else if ( clipA.position < clipA.lastPosition )
                        {
                            return clipA.position - clipB.position;
                        };
                        return 0;
                    }
                    );
    }

    function dragFinished() {
        dragging = false;
        sortSelectedClips();

        // We don't want to rely on selectedClips while moving since it "will" be changed
        // I'm aware that it's not the best solution but it's the safest solution for sure
        var toMove = [];
        for ( var i = 0; i < selectedClips.length; ++i )
        {
            var clip = findClipItem( selectedClips[i] );
            toMove.push( [clip.uuid, clip.newTrackId, clip.position] );
        }
        for ( i = 0; i < toMove.length; ++i )
            workflow.moveClip( toMove[i][0], toMove[i][1], toMove[i][2] );

        adjustTracks( "Audio" );
        adjustTracks( "Video" );
    }

    ListModel {
        id: trackContainers

        ListElement {
            name: "Video"
            tracks: []
        }

        ListElement {
            name: "Audio"
            tracks: []
        }

        Component.onCompleted: {
            addTrack( "Video" );
            addTrack( "Audio" );
        }
    }

    ListModel {
        id: markers
    }

    MouseArea {
        id: selectionArea
        width: parent.width - initPosOfCursor
        height: audioTrackContainer.y + audioTrackContainer.height - videoTrackContainer.y
        y: videoTrackContainer.y
        x: initPosOfCursor

        onPressed: {
            clearSelectedClips();
            selectionRect.visible = true;
            selectionRect.x = mouseX + x;
            selectionRect.y = mouseY + y;
            selectionRect.width = 0;
            selectionRect.height = 0;
            selectionRect.initPos = Qt.point( mouseX + x, mouseY + y );
        }

        onPositionChanged: {
            if ( selectionRect.visible === true ) {
                selectionRect.x = Math.min( mouseX + x, selectionRect.initPos.x );
                selectionRect.y = Math.min( mouseY + y, selectionRect.initPos.y );
                selectionRect.width = Math.abs( mouseX + x - selectionRect.initPos.x );
                selectionRect.height = Math.abs( mouseY + y - selectionRect.initPos.y );
                selectionRect.selectClips();
            }
        }

        onReleased: {
            selectionRect.visible = false;
        }
    }

    ScrollView {
        id: sView
        height: page.height
        width: page.width

        readonly property int sViewPadding: 50

        flickableItem.contentWidth: Math.max( page.width, ftop( length ) + initPosOfCursor + sViewPadding )
        flickableItem.contentHeight: Math.max( sView.height,
                                              topArea.height + videoTrackContainer.height +
                                              containerMarginItem.height + audioTrackContainer.height )

        Flickable {

            interactive: false

            TrackContainer {
                y: topArea.height
                id: videoTrackContainer
                type: "Video"
                isUpward: true
                tracks: trackContainers.get( 0 )["tracks"]
            }

            Rectangle {
                id: containerMarginItem
                anchors.top: videoTrackContainer.bottom
                height: 20
                width: parent.width
                gradient: Gradient {
                    GradientStop {
                        position: 0.00;
                        color: "#797979"
                    }

                    GradientStop {
                        position: 0.748
                        color: "#959697"
                    }

                    GradientStop {
                        position: 0.986
                        color: "#858f99"
                    }
                }
            }

            TrackContainer {
                anchors.top: containerMarginItem.bottom
                id: audioTrackContainer
                type: "Audio"
                isUpward: false
                tracks: trackContainers.get( 1 )["tracks"]
            }

            Item {
                id: topArea
                width: parent.width
                height: 52
                x: topLeftArea.width
                y: sView.flickableItem.contentY

                Ruler {
                    id: ruler

                    Rectangle {
                        id: borderBottomOfRuler
                        width: parent.width
                        height: 1
                        color: "#111111"
                    }
                }

                Cursor {
                    id: cursor
                    anchors.top: ruler.bottom
                    z: 2000
                    height: page.height
                }

                Repeater {
                    model: markers
                    anchors.top: topArea.top
                    delegate: Marker {
                        position: model.position
                        markerModel: model
                    }
                }
            }

            Rectangle {
                id: topLeftArea
                x: sView.flickableItem.contentX
                y: sView.flickableItem.contentY
                width: initPosOfCursor
                height: topArea.height
                color: "#333333"
                border.width: 1
                border.color: "#111111"

                Text {
                    id: cursorTimecode
                    x: 5
                    y: 2

                    text: timecodeFromFrames( cursorPosition )
                    color: "#EEEEEE"
                    font.pixelSize: parent.height / 4
                }

                Item {
                    id: properties
                    x: 5
                    y: parent.height / 2
                    width: parent.width - x * 2
                    height: parent.height / 2

                    Row {
                        spacing: 2

                        PropertyButton {
                            id: magneticModeButton
                            text: "M"
                            selected: true
                        }

                        PropertyButton {
                            id: zoomInButton
                            text: "+"
                            selected: false

                            onPressed: {
                                zoomIn( 2.0, true );
                                selected = false;
                            }
                        }

                        PropertyButton {
                            id: zoomOutButton
                            text: "-"
                            selected: false

                            onPressed: {
                                zoomIn( 0.5, true );
                                selected = false;
                            }
                        }

                        PropertyButton {
                            id: fxButton
                            text: "Fx"
                            selected: false

                            onPressed: {
                                workflow.showEffectStack();
                                selected = false;
                            }
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: selectionRect
        visible: false
        color: "#999999cc"
        property point initPos

        function selectClips() {
            for ( var i = 0; i < allClips.length; ++i ) {
                var clip = allClips[i];
                var clipPos = clip.mapToItem( page, 0, 0 );
                if ( ( x - clip.width < clipPos.x && clipPos.x < x + width ) &&
                     ( y - clip.height < clipPos.y && clipPos.y < y + height ) )
                    clip.selected = true;
            }
        }
    }

    MessageDialog {
        id: removeSelectedClipsDialog
        title: "VLMC"
        text: qsTr( "Do you really want to remove selected clips?" )
        icon: StandardIcon.Question
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            while ( selectedClips.length > 0 )
                workflow.removeClip( selectedClips[0] );
        }
    }

    Keys.onPressed: {
        if ( event.key === Qt.Key_Delete ) {
            removeSelectedClipsDialog.visible = true;
        }
        else if ( event.key === Qt.Key_Plus &&
                 event.modifiers & Qt.ControlModifier
                 && scale > 0 ) {
            zoomIn( 2, true );
        }
        else if ( event.key === Qt.Key_Minus &&
                 event.modifiers & Qt.ControlModifier &&
                 scale < 9 ) {
            zoomIn( 0.5, true );
        }
        event.accepted = true;
    }

    Connections {
        target: workflow

        onFpsChanged: {
            page.fps = fps;
        }

        onLengthChanged: {
            page.length = length;
        }

        onClipAdded: {
            var clipInfo = workflow.clipInfo( uuid );
            var type = clipInfo["audio"] ? "Audio" : "Video";
            clipInfo["selected"] = false;
            linkedClipsDict[uuid] = clipInfo["linkedClips"];
            addClip( type, clipInfo["trackId"], clipInfo );
            adjustTracks( type );

            zoomIn( page.width / sView.flickableItem.contentWidth, false );
        }

        onClipMoved: {
            var clipInfo = workflow.clipInfo( uuid );
            var type = clipInfo["audio"] ? "Audio" : "Video";
            var oldClip = findClipFromTrackContainer( type, uuid );
            linkedClipsDict[uuid] = clipInfo["linkedClips"];
            updateLinkedClips( uuid );

            if ( clipInfo["trackId"] !== oldClip["trackId"] ) {
                addClip( type, clipInfo["trackId"], clipInfo );
                removeClipFromTrack( type, oldClip["trackId"], uuid );
            }
            findClipItem( uuid ).position = clipInfo["position"];
            findClipItem( uuid ).lastPosition = clipInfo["position"];
            adjustTracks( type );
        }

        onClipRemoved: {
            removeClip( uuid );
            adjustTracks( "Audio" );
            adjustTracks( "Video" );
        }

        onClipResized: {
            var clipInfo = workflow.clipInfo( uuid );
            var clip = findClipItem( uuid );
            clip.position = clipInfo["position"];
            clip.end = clipInfo["end"];
            clip.begin = clipInfo["begin"];
            clip.updateEffects( clipInfo );
        }

        onClipLinked: {
            linkedClipsDict[uuidA].push( uuidB );
            linkedClipsDict[uuidB].push( uuidA );
            updateLinkedClips( uuidA );
            updateLinkedClips( uuidB );
        }

        onClipUnlinked: {
            for ( var i = 0; i < linkedClipsDict[uuidA].length; ++i )
                if ( linkedClipsDict[uuidA][i] === uuidB )
                {
                    linkedClipsDict[uuidA].splice( i, 1 );
                    break;
                }
            for ( i = 0; i < linkedClipsDict[uuidB].length; ++i )
                if ( linkedClipsDict[uuidB][i] === uuidA )
                {
                    linkedClipsDict[uuidB].splice( i, 1 );
                    break;
                }
            updateLinkedClips( uuidA );
            updateLinkedClips( uuidB );
        }

        onEffectsUpdated: {
            var item = findClipItem( clipUuid );
            if ( item )
                item.updateEffects( workflow.clipInfo( clipUuid ) );
        }
    }

    Connections {
        target: mainwindow
        onScaleChanged: {
            // 10 levels
            if ( scale < scaleLevel )
                zoomIn( 0.5, false );
            else if ( scale > scaleLevel )
                zoomIn( 2, false );
            scale = scaleLevel;
        }
        onCutToolSelected: {
            isCutMode = true;
        }
        onSelectionToolSelected: {
            isCutMode = false;
        }
    }

    Connections {
        target: timeline
        onMarkerAdded: {
            addMarker( pos );
        }
        onMarkerRemoved: {
            removeMarker( pos );
        }
    }
}

