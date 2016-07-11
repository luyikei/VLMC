import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2

Rectangle {
    id: page
    anchors.fill: parent
    color: "#777777"
    border.width: 0
    focus: true

    property int initPosOfCursor: 100
    property int ppu: 10 // Pixels Per minimum Unit
    property int unit: 3000 // In milliseconds so ppu / unit = Pixels Per milliseconds
    property double fps: 29.97
    property int maxZ: 100
    property var selectedClips: []

    property int trackHeight: 30

    function clearSelectedClips() {
        while ( selectedClips.length ) {
            var clipInfo = selectedClips.pop();
            if ( clipInfo["item"] )
                clipInfo["item"].selected = false;
        }
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
        newDict["length"] = clipDict["length"];
        newDict["uuid"] = clipDict["uuid"];
        newDict["trackId"] = trackId;
        newDict["name"] = clipDict["name"];
        var tracks = trackContainer( trackType )["tracks"];
        tracks.get( trackId )["clips"].append( newDict );

        if ( clipDict["uuid"] === "tempUuid" )
            return newDict;

        while ( tracks.count < trackId + 2 ) {
            addTrack( trackType );
        }
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
        if ( uuid === "tempUuid" )
            return ret;

        while ( tracks.count > 1 && tracks.get( tracks.count - 2 )["clips"].count === 0 )
            removeTrack( trackType );

        return ret;
    }

    function removeClipFromTrackContainer( trackType, uuid )
    {
        for ( var i = 0; i < trackContainer( trackType )["tracks"].count; i++  )
            removeClipFromTrack( trackType, i, uuid );
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

    function moveClipTo( trackType, uuid, trackId )
    {
        var clip = findClipFromTrackContainer( trackType, uuid );
        if ( !clip )
            return;
        var oldId = clip["trackId"];
        clip["trackId"] = trackId;
        workflow.moveClip( trackId, uuid, clip["position"] );
        addClip( trackType, trackId, clip );
        removeClipFromTrack( trackType, oldId, uuid );
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

    MouseArea {
        id: selectionArea
        anchors.fill: page

        onPressed: {
            clearSelectedClips();
            selectionRect.visible = true;
            selectionRect.x = mouseX;
            selectionRect.y = mouseY;
            selectionRect.width = 0;
            selectionRect.height = 0;
            selectionRect.initPos = Qt.point( mouseX, mouseY );
        }

        onPositionChanged: {
            if ( selectionRect.visible === true ) {
                selectionRect.x = Math.min( mouseX, selectionRect.initPos.x );
                selectionRect.y = Math.min( mouseY, selectionRect.initPos.y );
                selectionRect.width = Math.abs( mouseX - selectionRect.initPos.x );
                selectionRect.height = Math.abs( mouseY - selectionRect.initPos.y );
            }
        }

        onReleased: {
            selectionRect.visible = false;
        }
    }

    Ruler {
        id: ruler
        z: 1000
    }

    Cursor {
        id: cursor
        z: 2000
        height: page.height
        x: initPosOfCursor
    }

    Rectangle {
        id: borderBottomOfRuler
        width: page.width
        height: 1
        color: "#111111"
        anchors.top: ruler.bottom
    }

    ScrollView {
        id: sView
        anchors.top: borderBottomOfRuler.bottom
        anchors.bottom: page.bottom
        anchors.right: page.right
        anchors.left: page.left

        Column {
            TrackContainer {
                id: videoTrackContainer
                type: "Video"
                isUpward: true
                tracks: trackContainers.get( 0 )["tracks"]
            }

            Rectangle {
                height: 20
                width: page.width
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
                id: audioTrackContainer
                type: "Audio"
                isUpward: false
                tracks: trackContainers.get( 1 )["tracks"]
            }
        }
    }

    Rectangle {
        id: selectionRect
        visible: false
        color: "#999999cc"
        property point initPos
    }

    MessageDialog {
        id: removeClipDialog
        title: "VLMC"
        text: qsTr( "Do you really want to remove selected clips?" )
        icon: StandardIcon.Question
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            while ( selectedClips.length ) {
                workflow.removeClip( selectedClips[0]["uuid"] );
                removeClipFromTrackContainer( selectedClips[0]["item"].type, selectedClips[0]["uuid"] );
            }
        }
    }

    Keys.onPressed: {
        if ( event.key === Qt.Key_Delete ) {
            removeClipDialog.visible = true;
            event.accepted = true;
        }
    }

    Connections {
        target: workflow
        onLengthChanged: {
            if ( ruler.width < ftop( length ) + 100 ) {
                var newPpu = ppu;
                var newUnit = unit;
                newPpu *= width / ( ftop( length ) + 100 )
                while ( newPpu < 5 ) {
                    newPpu *= 2;
                    newUnit *= 2;
                }
                unit = newUnit;
                ppu = newPpu;
            }
        }
    }
}

