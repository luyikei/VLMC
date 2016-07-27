import QtQuick 2.0

Rectangle {
    id: marker

    width: 1
    height: page.height
    color: "#3F6644"
    x: ftop( position )
    visible: x < sView.flickableItem.contentX ? false : true

    property int position: 0
    property var markerModel

    onPositionChanged: {
        markerModel["position"] = position;
        length = Math.max( length, position + 100 );
    }

    onMarkerModelChanged: {
        position = markerModel["position"];
    }

    Drag.keys: ["Marker"]
    Drag.active: markerMouseArea.drag.active
    Rectangle {
        x: - ( width - 1 ) / 2
        width: 5
        height: 9
        color: "#63ff44"

        MouseArea {
            id: markerMouseArea
            anchors.fill: parent
            drag.target: marker
            drag.axis: Drag.XAxis
            drag.minimumX: 0
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            onClicked: {
                if ( mouse.button & Qt.RightButton ) {
                    markerContextMenu.popup();
                }
            }
        }

        MarkerContextMenu {
            id: markerContextMenu
            marker: marker
        }
    }
}

