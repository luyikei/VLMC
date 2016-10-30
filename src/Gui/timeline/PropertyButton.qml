import QtQuick 2.0

Rectangle {
    width: 20
    height: 20
    color: mouseArea.containsMouse ? "#44FFFFFF" : selected ? "#AA000000" : "transparent"
    radius: 2
    id: propertyButton

    property alias text: _text.text
    property bool selected: false

    signal pressed()

    Text {
        id: _text
        color: "#EEEEEE"
        font.pixelSize: propertyButton.width / 2
        anchors.centerIn: parent
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true

        onPressed: {
            selected = !selected;
            parent.pressed();
        }
    }
}
