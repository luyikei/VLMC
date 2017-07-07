import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2

Menu {
    id: markerContextMenu
    title: "Edit"

    property var marker

    MenuItem {
        text: "Delete"

        onTriggered: {
            removeMarkerDialog.visible = true;
        }
    }

    MessageDialog {
        id: removeMarkerDialog
        title: "VLMC"
        text: qsTr( "Do you really want to remove the marker?" )
        icon: StandardIcon.Question
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            timeline.removeMarker( marker.position );
        }
    }
}
