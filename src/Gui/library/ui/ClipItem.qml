import QtQuick 2.0
import QtQuick.Controls 1.4

Rectangle {
    height: visible ? 100 : 0
    id: clipItem
    color: ListView.isCurrentItem ? "black" : "#333333"
    visible: ( clipLibraryView.currentMediaId === mediaId && !isBaseClip ) ||
             ( clipLibraryView.currentMediaId === -1 && isBaseClip && ( onTimeline || subClipsCount ) )

    property string uuid
    property int mediaId
    property bool isBaseClip
    property string thumbnailPath
    property string title
    property int duration
    property bool onTimeline
    property var subClips: []
    property int subClipsCount: 0

    function addSubClip( uuid ) {
        subClips.push( uuid );
        subClipsCount = subClips.length;
    }

    function removeSubClip( uuid ) {
        for ( var i = 0; i < subClips.length; i++ ) {
            if ( subClips[i] === uuid ) {
                subClips.splice( i, 1 );
                subClipsCount = subClips.length;
                break;
            }
        }
    }
    
    function toDuration( seconds ) {
        if ( seconds <= 0 )
            return "00:00:00";
        var hours   = Math.floor(seconds / 3600);
        seconds = seconds % 3600;
        var minutes = Math.floor( seconds / 60 );
        seconds = Math.floor( seconds % 60 );
        
        if (hours   < 10) {hours   = "0" + hours;}
        if (minutes < 10) {minutes = "0" + minutes;}
        if (seconds < 10) {seconds = "0" + seconds;}
        return hours + ':' + minutes + ':' + seconds;
    }

    Component.onCompleted: {
        clipLibraryView.clipItems.push( this );
        if ( !isBaseClip ) {
            for ( var i = 0; i < clipItems.length; ++i ) {
                if ( clipItems[i].mediaId === mediaId && clipItems[i].isBaseClip ) {
                    clipItems[i].addSubClip( uuid );
                    break;
                }
            }
        }
    }

    Component.onDestruction: {
        for ( var i = 0; i < clipLibraryView.clipItems.length; i++ ) {
            if ( clipLibraryView.clipItems[i] === this ) {
                clipLibraryView.clipItems.splice( i, 1 );
                break;
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onPressed: {
            clipListView.currentIndex = index;
            view.onClipSelected( uuid );
            view.startDrag( uuid );
        }
    }

    Row {
        anchors.fill: parent
        Image {
            id: markerRight
            visible: !isBaseClip
            height: clipItem.height
            source: "qrc:///images/marker_right"
            anchors.verticalCenter: parent.verticalCenter
            fillMode: Image.PreserveAspectFit

            MouseArea {
                anchors.fill: parent
                onPressed: {
                    clipLibraryView.currentMediaId = -1;
                }
            }
        }

        Image {
            id: thumbnail
            height: clipItem.height
            source: thumbnailPath.length > 0 ? "file://" + thumbnailPath : "qrc:///images/vlmc"
            fillMode: Image.PreserveAspectFit
        }

        Column {
            width: clipItem.width - thumbnail.width - 30 - markerLeft.width
            Text {
                id: titleText
                width: parent.width
                font.pointSize: 8
                maximumLineCount: 2
                color: "#EEEEEE"
                text: title
                elide: Text.ElideRight
                wrapMode: Text.Wrap
            }
            Text {
                id: durationText
                width: parent.width
                font.pointSize: 8
                maximumLineCount: 1
                color: "#EEEEEE"
                text: "Duration: " + toDuration( duration )
                elide: Text.ElideRight
                wrapMode: Text.Wrap
            }
        }

        Image {
            id: markerLeft
            visible: isBaseClip && subClipsCount
            height: clipItem.height
            source: "qrc:///images/marker_left"
            anchors.verticalCenter: parent.verticalCenter
            fillMode: Image.PreserveAspectFit
            z: -100

            MouseArea {
                anchors.fill: parent
                onPressed: {
                    clipLibraryView.currentMediaId = mediaId;
                }
            }
        }
    }
}
