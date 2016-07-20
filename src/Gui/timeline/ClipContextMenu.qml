import QtQuick 2.0
import QtQuick.Controls 1.4

Menu {
    id: clipContextMenu
    title: "Edit"

    property string uuid
    property bool isGrouped

    MenuItem {
        text: isGrouped ? "Ungroup" : "Group"

        onTriggered: {
            if ( selectedClips.length <= 1 )
                return;

            if ( isGrouped ) {
                removeGroup( uuid );
            }
            else {
                var l = [];
                for ( var i = 0; i < selectedClips.length; ++i ) {
                    l.push( "" + selectedClips[i].uuid );
                }
                addGroup( l );
            }
        }
    }

    onAboutToShow: {
        isGrouped = findGroup( uuid );
    }
}
