import QtQuick 2.5

Rectangle {
    width: 1
    color: "#c24a00"
    x: ftop( cursorPosition )
    visible: x < sView.flickableItem.contentX ? false : true

    Component.onCompleted: {
        // FIXME: Binding is lost
        x = Qt.binding( function() { return ftop( cursorPosition ); } );
    }

    // Triangle
    Canvas {
        id: mycanvas
        width: 15
        height: width / 2 * 1.73
        y: - height
        x: - width / 2

        onPaint: {
            var ctx = getContext( "2d" );
            ctx.fillStyle = Qt.rgba( 0.9, 0.8, 0.25, 1 );
            ctx.strokeStyle = "transparent";
            ctx.beginPath();
            ctx.moveTo( 0, 0 );
            ctx.lineTo( width, 0 );
            ctx.lineTo( width / 2, height );
            ctx.closePath();
            ctx.fill();
            ctx.stroke();
        }
    }
}
