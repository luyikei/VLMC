import QtQuick 2.0
import QtQuick.Controls 1.4

Rectangle {
    id: effect
    color: "#444444"
    width: parent.width
    height: 100
    border.color: "#222222"
    border.width: 1

    property string identifier
    property string name
    property string description
    property string author

    Column {
        width: sView.viewport.width

        Text {
            text: name
            font.pointSize: 20
            color: "#EEEEEE"
            elide: Text.ElideRight
            width: parent.width
        }

        Text {
            text: identifier
            color: "#EEEEEE"
            elide: Text.ElideRight
            width: parent.width
        }

        Text {
            text: description
            color: "#EEEEEE"
            elide: Text.ElideRight
            width: parent.width
        }

        Text {
            text: author
            color: "#EEEEEE"
            elide: Text.ElideRight
            width: parent.width
        }
    }

    MouseArea {
        anchors.fill: parent
        onPressed: {
            view.startDrag( identifier );
        }
    }
}
