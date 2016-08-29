import QtQuick 2.0
import QtQuick.Controls 1.4

Rectangle {
    anchors.fill: parent
    color: "#999999"

    ListModel {
        id: effects
    }

    Component.onCompleted: {
        var effectsInfo = view.effects();
        for ( var i = 0; i < effectsInfo.length; ++i ) {
            effects.append( effectsInfo[i] );
        }
    }

    ScrollView {
        id: sView
        height: parent.height
        width: parent.width

        ListView {
            width: sView.viewport.width
            model: effects
            delegate: Effect {
                identifier: model.identifier
                name: model.name
                description: model.description
                author: model.author
            }
        }
    }
}
