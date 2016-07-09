import QtQuick 2.0
import QtQuick.Controls 1.4

Menu {
    id: clipContextMenu
    title: "Edit"

    MenuItem {
        text: "Cut"
        shortcut: "Ctrl+X"
    }

    MenuItem {
        text: "Copy"
        shortcut: "Ctrl+C"
    }

    MenuItem {
        text: "Paste"
        shortcut: "Ctrl+V"
    }

    MenuSeparator { }

    Menu {
        title: "More Stuff"

        MenuItem {
            text: "Do Nothing"
        }
    }
}
