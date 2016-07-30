import QtQuick 2.0
import QtQuick.Controls 1.4

Rectangle {
    anchors.fill: parent
    color: "#999999"

    ScrollView {
        anchors.fill: parent

        GridView {
            id: gridView
            model: mlModel
            anchors.fill: parent
            cellHeight: cellWidth
            delegate: MediaItem {
                width: gridView.cellWidth
                height: width
                duration: model.duration
                thumbnailPath: model.thumbnailPath
                title: model.title
            }

            // Avoid binding loop
            onWidthChanged: {
                cellWidth = width / 3;
            }
        }
    }
}
