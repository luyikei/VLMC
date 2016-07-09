import QtQuick 2.0

Rectangle {
    width: 2
    color: "#c24a00"

    onXChanged: {
         x = x > initPosOfCursor ? x : initPosOfCursor;
    }
}

