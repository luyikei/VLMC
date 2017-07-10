import QtQuick 2.0
import QtQuick.Controls 1.4

Rectangle {
    anchors.fill: parent
    color: "#999999"

    property alias searchText: searchTextInput.text

    ScrollView {
        id: sView
        height: parent.height
        width: parent.width

        readonly property int viewWidth: viewport.width

        Flickable {
            id: fItem
            contentHeight: inputRect.height + gridView.height
            contentWidth: parent.parent.width

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
                            id: searchTextInput
                            color: "#DDDDDD"
                            width: parent.width
                            selectByMouse: true
                            anchors.centerIn: parent
                        }
                    }
                }

                Flickable {
                    id: gridView
                    width: sView.viewport.width
                    height: contentHeight

                    property int cellWidth: 200
                    property int cellHeight: cellWidth
                    property int currentMedia: -1

                    Grid {
                        columns: 3

                        add: Transition {
                            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 200; easing.type: Easing.OutSine }
                        }

                        move: Transition {
                            NumberAnimation { property: "x"; duration: 400; easing.type: Easing.OutSine }
                            NumberAnimation { property: "y"; duration: 400; easing.type: Easing.OutSine }
                        }

                        Repeater {
                            id: mlRepeater
                            model: mlModel
                            MediaItem {
                                duration: model.duration
                                thumbnailPath: model.thumbnailPath
                                title: model.title
                                mediaId: model.id
                            }
                        }
                    }
                }
            }
        }

        // Avoid binding loop
        onViewWidthChanged: {
            gridView.cellWidth = Math.max( viewWidth / 3, 50 );
        }
    }
}
