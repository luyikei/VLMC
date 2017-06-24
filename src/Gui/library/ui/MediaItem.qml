import QtQuick 2.0
import QtQuick.Controls 1.4

Rectangle {
    id: mediaItem
    width: visible ? gridView.cellWidth : 0
    height: visible ? gridView.cellWidth + 50 : 0
    color: gridView.currentMedia == mediaId ? "black" : "#333333"
    border.width: 1
    visible: searchText.length == 0 || containsString( searchText )
    enabled: visible
    property string thumbnailPath
    property string title
    property int duration
    property int mediaId

    function containsString( str )
    {
        return title.toLowerCase().indexOf( str.toLowerCase() ) !== -1;
    }

    Image {
        id: thumbnail
        anchors.left: mediaItem.left
        anchors.right: mediaItem.right
        anchors.top: mediaItem.top
        source: thumbnailPath.length > 0 ? "file://" + thumbnailPath : "qrc:///images/vlmc"
        fillMode: Image.PreserveAspectFit
    }

    Column {
        id: textColumn
        anchors.top: thumbnail.bottom
        anchors.bottom: mediaItem.bottom
        width: mediaItem.width

        Text {
            id: mediaTitle
            width: gridView.cellWidth
            height: textColumn.height / 3 * 2
            color: "#EEEEEE"
            lineHeight: 0.8
            text: title
            wrapMode: Text.Wrap
            elide: Text.ElideRight
        }

        Text {
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
            id: durationLabel
            color: "#EEEEEE"
            text: toDuration( duration / 1000 )
            width: mediaItem.width
            height: textColumn.height / 3
            elide: Text.ElideRight
        }
    }

    MouseArea {
        id: dragArea
        anchors.fill: parent
        onPressed: {
            gridView.currentMedia = mediaId;
            view.onMediaSelected( mediaId );
        }
    }
}
