import QtQuick 2.0
import QtQuick.Controls 1.4

Rectangle {
    id: mediaItem
    color: "#333333"
    border.color: "#222222"
    border.width: 1
    property string thumbnailPath
    property string title
    property int duration

    Image {
        id: thumbnail
        anchors.left: mediaItem.left
        anchors.right: mediaItem.right
        anchors.top: mediaItem.top
        anchors.bottom: textColumn.top
        source: thumbnailPath.length > 0 ? "file://" + thumbnailPath : "qrc:///images/vlmc"
        fillMode: Image.PreserveAspectFit
    }
    Column {
        id: textColumn
        anchors.bottom: mediaItem.bottom
        width: mediaItem.width
        Text {
            id: mediaTitle
            text: title
            fontSizeMode: Text.HorizontalFit
            minimumPixelSize: 4
            width: mediaItem.width
            elide: Text.ElideRight
            color: "#EEEEEE"
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
            color: "#EEEEEE"
            id: durationLabel
            text: toDuration( duration / 1000 )
        }
    }

    Drag.active: dragArea.drag.active
    Drag.dragType: Drag.Automatic
    Drag.mimeData: {
        "vlmc/uuid":"test",
    }

    MouseArea {
        id: dragArea
        drag.target: parent
        anchors.fill: parent
        onClicked: gridView.currentIndex = index
    }
}
