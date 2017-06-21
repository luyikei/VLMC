import QtQuick 2.0
import QtQuick.Controls 1.4

Rectangle {
    id: clipLibraryView
    anchors.fill: parent
    color: "#999999"

    property var clipItems: []
    property int currentMediaId: -1

    ListModel {
        id: clips
    }

    ListModel {
        id: subClips
    }

    ScrollView {
        id: sView
        height: parent.height
        width: parent.width

        ListView {
            id: clipListView
            width: sView.viewport.width
            model: clips
            focus: true
            delegate: ClipItem {
                uuid: model.uuid
                isBaseClip: model.isBaseClip
                duration: model.duration
                thumbnailPath: model.thumbnailPath
                mediaId: model.mediaId
                onTimeline: model.onTimeline
                title: model.title
                width: parent.width
            }
        }
    }

    Connections {
        target: view

        onClipAdded: {
            var clip = view.clip( uuid );
            clips.append( clip );
        }

        onClipOnTimelineChanged: {
            for ( var i = 0; i < clips.count; ++i ) {
                if ( clips.get(i)["uuid"] === uuid ) {
                    clips.get(i)["onTimeline"] = onTimeline;
                    break;
                }
            }
        }

        onClipRemoved: {
            var clip = view.clip( uuid );
            if ( clip.isBaseClip === false ) {
                for ( var i = 0; i < clipItems.length; ++i ) {
                    if ( clipItems[i].mediaId === clip.mediaId && clipItems[i].isBaseClip ) {
                        clipItems[i].removeSubClip( uuid );
                        break;
                    }
                }
            }
            for ( i = 0; i < clips.count; ++i ) {
                if ( clips[i].uuid === uuid ) {
                    clips.remove( i );
                    break;
                }
            }
        }
    }
}
