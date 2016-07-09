import QtQuick 2.0

Rectangle {
    id: ruler
    height: 52
    width: parent.width - initPosOfCursor
    x: initPosOfCursor
    color: "#333333"

    MouseArea {
        anchors.fill: parent

        onPressed: {
            cursor.x = mouseX + initPosOfCursor;
        }

        onReleased: {
            workflow.setPosition( ptof( mouseX ) );
        }

        onClicked: {
            workflow.setPosition( ptof( mouseX ) );
        }

        onPositionChanged: {
            cursor.x = mouseX + initPosOfCursor;
            workflow.setPosition( ptof( mouseX ) );
        }
    }

    Column {
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

        Row {
            id: rootRow
            height: 35
            Repeater {
                model: ruler.width / ppu // pixels per minimum unit
                Item {
                    width: ppu
                    height: rootRow.height
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
                        if ( index % 20 == 0 ) {
                            var seconds = parseInt( index * unit / 1000 );
                            var minutes = parseInt( seconds / 60 );
                            var hours = parseInt( minutes / 60 );
                            // Text
                            Qt.createQmlObject('import QtQuick 2.0; Text {text:"' +
                                                   zerofill( hours, 3 ) + ':' +
                                                   zerofill( minutes % 60, 2 ) + ':' +
                                                   zerofill( seconds % 60, 2 ) + '";' +
                                                   'color: "white";' +
                                                   'font.pointSize: ruler.height / 5' +
                                                   '}',
                                                   this
                                                   );
                            scale.color = "#AA7777";
                            scale.width = 2;
                            scale.gradient = undefined;
                        }
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

        Rectangle {
            height: 9
            width: ruler.width
            color: "#666666"
        }
    }

    Connections {
        target: workflow
        onFrameChanged: {
            cursor.x = ftop( newFrame ) + initPosOfCursor;
        }
    }
}

