import QtQuick 2.0

Rectangle {
    id: ruler
    height: parent.height
    width: parent.width - initPosOfCursor
    color: "#333333"

    Column {
        Rectangle {
            id: markersArea
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

                Component.onCompleted: {
                    if ( index % 15 == 0 ) {
                        // Text
                        var obj = Qt.createQmlObject('import QtQuick 2.0; Text {'+
                                               'color: "white";' +
                                               'font.pointSize: ruler.height / 6' +
                                               '}',
                                               this
                                               );
                        obj.text = Qt.binding( function()
                        {
                            return timecodeFromFrames( ptof( x ) );
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

    DropArea {
        id: markersDropArea
        width: parent.width
        height: markersArea.height
        keys: ["Marker"]

        onPositionChanged: {
            var newPosition = ptof( drag.x );

            var samePosMarkersCount = 0;
            for ( var i = 0; i < markers.count; ++i ) {
                if ( markers.get( i )["position"] === newPosition )
                    samePosMarkersCount++;
            }

            // Move the marker to the right a bit
            if ( samePosMarkersCount >= 2 )
                newPosition += Math.max( ptof( 1 ), 1 );

            drag.source.position = newPosition;
        }
    }

    MouseArea {
        id: markersMouseArea
        width: parent.width
        height: markersArea.height

        onClicked: {
            timeline.addMarker( ptof( mouseX ) );
        }
    }

    Connections {
        target: workflow
        onFrameChanged: {
            cursorPosition = newFrame;
        }
    }
}

