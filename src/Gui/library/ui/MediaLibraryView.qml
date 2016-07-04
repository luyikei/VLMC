import QtQuick 2.0
import QtQuick.Controls 1.4

ScrollView {
    Component {
        id: itemDelegate
        Item {
            id: mediaItem
            width: gridView.cellWidth - gridView.spacing
            height: gridView.cellHeight - gridView.spacing
            Column {
                Image {
                    id: thumbnail
                    source: thumbnailPath.length > 0 ? "file://" + thumbnailPath : "qrc:///images/vlmc"
                    fillMode: Image.PreserveAspectFit
                    width: mediaItem.width
                    height: mediaItem.height - 20
                }
                Row {
                    width: mediaItem.width
                    Text {
                        id: mediaTitle
                        text: title
                        fontSizeMode: Text.HorizontalFit
                        minimumPixelSize: 4
                        width: mediaItem.width - durationLabel.width
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
                        text: toDuration( duration / 1000 )
                    }
                }
            }
            Drag.active: dragArea.drag.active
            Drag.dragType: Drag.Automatic
            Drag.mimeData: {
                "vlmc/uuid":"test",
            }
            MouseArea {
                id: dragArea
                drag.target: mediaItem
                anchors.fill: parent
                anchors.centerIn: parent
                onClicked: gridView.currentIndex = index
            }
        }
    }
    GridView {
        id: gridView
        model: mlModel
        anchors.fill: parent
        property int spacing: 10
        cellWidth: 200 + spacing
        cellHeight: 180 + spacing
        delegate: itemDelegate
        highlight: Rectangle {
            color: "lightsteelblue"
            radius: 5
        }
    }
}
