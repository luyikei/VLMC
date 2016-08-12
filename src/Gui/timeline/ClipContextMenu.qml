import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2

Menu {
    id: clipContextMenu
    title: "Edit"

    property var clip
    property bool grouped

    MenuItem {
        text: grouped ? "Ungroup" : "Group"

        onTriggered: {
            if ( selectedClips.length <= 1 )
                return;

            if ( grouped === true ) {
                removeGroup( clip.uuid );
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

    MenuItem {
        text: clip.linked ? "Unlink" : "Link"

        onTriggered: {
            clip.linked = !clip.linked;
        }
    }

    MenuSeparator { }

    MenuItem {
        text: "Delete"

        onTriggered: {
            removeClipDialog.visible = true;
        }
    }

    MessageDialog {
        id: removeClipDialog
        title: "VLMC"
        text: qsTr( "Do you really want to remove the clip?" )
        icon: StandardIcon.Question
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            workflow.removeClip( clip.uuid );
        }
    }

    onAboutToShow: {
        grouped = findGroup( clip.uuid );
    }

    MenuSeparator { }

    MenuItem {
        text: "Effects"

        onTriggered: {
            workflow.showEffectStack( clip.uuid );
        }
    }
}
