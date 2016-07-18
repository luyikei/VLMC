import QtQuick 2.0

Rectangle {
    width: 2
    color: "#c24a00"
    x: ftop( cursorPosition ) + initPosOfCursor

    Component.onCompleted: {
        // FIXME: Binding is lost
        x = Qt.binding( function() { return ftop( cursorPosition ) + initPosOfCursor; } );
    }

    function position() {
        return ptof( x - initPosOfCursor );
    }
}

