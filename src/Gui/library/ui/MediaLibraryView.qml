import QtQuick 2.0
import QtQuick.Controls 1.4

Rectangle {
    anchors.fill: parent
    color: "#999999"

    ScrollView {
        id: sView
        height: parent.height
        width: parent.width

        readonly property int viewWidth: viewport.width

        Flickable {
            id: fItem
            contentHeight: inputRect.height + gridView.height
            contentWidth: parent.width

            Column {
                Rectangle {
                    id: inputRect
                    width: sView.viewport.width
                    height: 30
                    color: "#222222"

                    Rectangle {
                        anchors.centerIn: parent
                        width: inputRect.width * 0.9
                        height: inputRect.height * 0.9
                        radius: 5
                        gradient: Gradient {
                            GradientStop {
                                position: 0.00;
                                color: "#313131";
                            }
                            GradientStop {
                                position: 1.00;
                                color: "#515151";
                            }
                        }

                        TextInput {
                            color: "#DDDDDD"
                            width: parent.width
                            anchors.centerIn: parent
                        }
                    }
                }

                GridView {
                    id: gridView
                    model: mlModel
                    width: sView.viewport.width
                    height: ( count + 3 - ( count % 3 ) ) / 3 * cellHeight
                    cellHeight: cellWidth
                    delegate: MediaItem {
                        width: gridView.cellWidth
                        height: width
                        duration: model.duration
                        thumbnailPath: model.thumbnailPath
                        title: model.title
                        mediaId: model.id
                    }
                }
            }
        }

        // Avoid binding loop
        onViewWidthChanged: {
            gridView.cellWidth = viewWidth / 3;
        }
    }
}
