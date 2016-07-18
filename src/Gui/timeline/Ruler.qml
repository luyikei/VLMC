import QtQuick 2.0

Rectangle {
    id: ruler
    height: 52
    width: parent.width - initPosOfCursor
    x: initPosOfCursor
    color: "#333333"

    Column {
        Rectangle {
            height: 9
            width: ruler.width
            color: "#666666"
        }

        Rectangle {
            id: curveBorder
            height: 7
            width: parent.width
            gradient: Gradient {
                GradientStop {
                    position: 0.00;
                    color: "#000000";
                }
                GradientStop {
                    position: 0.20;
                    color: "#999999";
                }
                GradientStop {
                    position: 1.00;
                    color: "#222222";
                }
            }
        }

        ListView {
            model: ruler.width / ppu // pixels per minimum unit
            height: 35
            width: parent.width
            orientation: Qt.Horizontal
            layoutDirection: Qt.LeftToRight
            delegate: Item {
                width: ppu
                height: 35
                Rectangle {
                    id: scale
                    width: 1
                    height: ruler.height / 2
                    anchors.bottom: parent.bottom
                    gradient: Gradient {
                        GradientStop {
                            position: 0.00;
                            color: "#00000088";
                        }
                        GradientStop {
                            position: 1.00;
                            color: "#c2c2c2";
                        }
                    }
                }

                function zerofill( number, width ) {
                    var str = "" + number;
                    while ( str.length < width ) {
                        str = "0" + str;
                    }
                    return str;
                }

                Component.onCompleted: {
                    if ( index % 10 == 0 ) {
                        // Text
                        var obj = Qt.createQmlObject('import QtQuick 2.0; Text {'+
                                               'color: "white";' +
                                               'font.pointSize: ruler.height / 6' +
                                               '}',
                                               this
                                               );
                        obj.text = Qt.binding( function()
                        {
                            var seconds = Math.floor( index * unit / 1000 );
                            var minutes = Math.floor( seconds / 60 );
                            var hours = Math.floor( minutes / 60 );

                            return zerofill( hours, 3 ) + ':' + // hours
                                    zerofill( minutes % 60, 2 ) + ':' + // minutes
                                    zerofill( seconds % 60, 2 ) + ':' + // seconds
                                    // The second Math.round prevents the first value from exceeding fps.
                                    // e.g. 30 % Math.round( 29.97 ) = 0
                                    zerofill( Math.round( ptof( index * ppu ) % fps ) % Math.round( fps ), 2 ); // frames in a minute
                        } );
                        scale.color = "#AA7777";
                        scale.width = 2;
                        scale.gradient = undefined;
                    }
                }
            }
        }

        Rectangle {
            height: 1
            width: ruler.width
            color: "#000000"
            gradient: Gradient {
                GradientStop {
                    position: 0.00;
                    color: "#ff6e00";
                }
                GradientStop {
                    position: 1.00;
                    color: "#7a0000";
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent

        onPressed: {
            cursorPosition = ptof( mouseX );
        }

        onReleased: {
            workflow.setPosition( cursorPosition );
        }

        onClicked: {
            cursorPosition = ptof( mouseX );
            workflow.setPosition( cursorPosition );
        }

        onPositionChanged: {
            cursorPosition = ptof( mouseX );
            workflow.setPosition( cursorPosition );
        }
    }

    Connections {
        target: workflow
        onFrameChanged: {
            cursorPosition = newFrame;
        }
    }
}

