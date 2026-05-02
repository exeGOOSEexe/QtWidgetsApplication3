import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3

ApplicationWindow {
    id: root
    visible: true
    width: 1440
    height: 900
    title: "Soundora"
    color: "#0F0F14"

    property bool isRightPanelOpen: false
    property bool isPlaying: false

    FileDialog {
        id: fileDialog
        title: "Выберите музыкальный файл"
        folder: shortcuts.music
        nameFilters: ["Audio files (*.mp3 *.wav *.flac)", "All files (*)"]
        onAccepted: {
            audioPlayer.playQml(fileUrl.toString())
            isPlaying = true
        }
    }

    font.family: "Inter"
    font.pixelSize: 14

    RowLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: bottomBar.top
        spacing: 0

        // 1. ЛЕВОЕ МЕНЮ
        Rectangle {
            Layout.preferredWidth: 240
            Layout.fillHeight: true
            color: "#0F0F14"

            ScrollView {
                anchors.fill: parent
                anchors.margins: 20
                clip: true

                ColumnLayout {
                    width: parent.width
                    spacing: 15

                    Text {
                        text: "ılı Soundora"
                        color: "white"
                        font.pixelSize: 22
                        font.bold: true
                        Layout.bottomMargin: 20
                    }

                    Rectangle {
                        Layout.fillWidth: true; height: 36
                        radius: 8; color: "#1F1F28"
                        Text { anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 15; text: "🔍 Поиск"; color: "#808089" }
                    }

                    Item { Layout.preferredHeight: 10 }
                    MenuButton { text: "🏠 Главная"; active: true }
                    MenuButton { text: "🧭 Обзор" }
                    MenuButton { text: "📻 Радио" }
                    MenuButton { text: "🎙 Подкасты" }

                    Item { Layout.preferredHeight: 20 }
                    Text { text: "Моя медиатека"; color: "#808089"; font.pixelSize: 12 }
                    MenuButton { text: "💜 Любимые треки" }
                    MenuButton { text: "🎵 Плейлисты" }
                    MenuButton { text: "💿 Альбомы" }
                    MenuButton { text: "👤 Исполнители" }
                    MenuButton { text: "⬇ Загруженное" }
                }
            }
        }

        // 2. ЦЕНТРАЛЬНЫЙ КОНТЕНТ
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#0F0F14"

            ScrollView {
                anchors.fill: parent
                anchors.margins: 20
                clip: true
                ScrollBar.vertical.policy: ScrollBar.AlwaysOff

                ColumnLayout {
                    width: parent.width
                    spacing: 30

                    Rectangle {
                        Layout.fillWidth: true; height: 280
                        radius: 20; color: "#1F1F28"
                        
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#1F1F28" }
                            GradientStop { position: 1.0; color: "#2A2A33" }
                        }

                        Column {
                            anchors.left: parent.left; anchors.top: parent.top
                            anchors.margins: 30; spacing: 10
                            Text { text: "плейлист дня"; color: "#B3B3BC"; font.pixelSize: 12; font.capitalization: Font.AllUppercase }
                            Text { text: "Поток\nвдохновения"; color: "white"; font.pixelSize: 40; font.bold: true; lineHeight: 1.1 }
                            Text { text: "Музыка, которая помогает\nсосредоточиться и творить."; color: "#B3B3BC" }
                            
                            Rectangle {
                                width: 120; height: 40; radius: 8; topMargin: 10
                                gradient: Gradient { GradientStop {position: 0; color: "#5A67FF"} GradientStop {position: 1; color: "#9B7CFF"} }
                                Text { anchors.centerIn: parent; text: "Слушать"; color: "white"; font.bold: true }
                            }
                        }
                    }

                    SectionHeader { title: "Made for you" }
                    RowLayout {
                        spacing: 20
                        Repeater {
                            model: 4
                            Rectangle {
                                id: playlistCard
                                width: 160; height: 220; radius: 12
                                color: cardMouseArea.containsMouse ? "#2A2A33" : "#16161D"
                                Behavior on color { ColorAnimation { duration: 150 } }

                                Rectangle {
                                    id: cover
                                    width: 140; height: 140; radius: 10
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    anchors.top: parent.top; anchors.topMargin: 10
                                    gradient: Gradient { 
                                        GradientStop {position: 0; color: "#3DD6C6"} 
                                        GradientStop {position: 1; color: "#4D8CFF"} 
                                    }

                                    Rectangle {
                                        anchors.bottom: parent.bottom; anchors.right: parent.right
                                        anchors.margins: 10
                                        width: 40; height: 40; radius: 20
                                        color: "#5A67FF"
                                        opacity: cardMouseArea.containsMouse ? 1 : 0
                                        anchors.bottomMargin: cardMouseArea.containsMouse ? 10 : 0
                                        
                                        Behavior on opacity { NumberAnimation { duration: 200 } }
                                        Behavior on anchors.bottomMargin { NumberAnimation { duration: 200; easing.type: Easing.OutBack } }

                                        Text { anchors.centerIn: parent; text: "▶"; color: "white" }
                                    }
                                }

                                Text { text: "Daily Mix " + (index+1); color: "white"; font.bold: true; anchors.bottom: parent.bottom; anchors.left: parent.left; anchors.margins: 10; anchors.bottomMargin: 30 }
                                Text { text: "Spotify, Yandex..."; color: "#808089"; font.pixelSize: 11; anchors.bottom: parent.bottom; anchors.left: parent.left; anchors.margins: 10 }

                                MouseArea {
                                    id: cardMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                }
                            }
                        }
                    }
                }
            }
        }

        // 3. ПРАВАЯ ПАНЕЛЬ
        Rectangle {
            id: rightPanel
            Layout.preferredWidth: isRightPanelOpen ? 320 : 0
            Layout.fillHeight: true
            color: "#16161D"
            clip: true

            Behavior on Layout.preferredWidth {
                NumberAnimation { duration: 350; easing.type: Easing.OutCubic }
            }

            ColumnLayout {
                visible: rightPanel.Layout.preferredWidth > 0
                anchors.fill: parent
                anchors.margins: 20
                spacing: 20

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Сейчас играет"; color: "white"; font.bold: true; font.pixelSize: 16; Layout.fillWidth: true }
                    Text { text: "v"; color: "white" }
                }

                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: Layout.width
                    radius: 12; color: "#FF4FD8"
                    gradient: Gradient { GradientStop {position: 0; color: "#FF4FD8"} GradientStop {position: 1; color: "#7B61FF"} }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Column {
                        Layout.fillWidth: true
                        Text { text: "Выбранный трек"; color: "white"; font.bold: true; font.pixelSize: 20 }
                        Text { text: "Локальный файл"; color: "#B3B3BC"; font.pixelSize: 14 }
                    }
                }
            }
        }
    }

    // 4. НИЖНЯЯ ПАНЕЛЬ ПЛЕЕРА
    Rectangle {
        id: bottomBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 90
        color: "#16161D"

        Rectangle { width: parent.width; height: 1; color: "#2A2A33"; anchors.top: parent.top }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 20

            RowLayout {
                Layout.preferredWidth: 300
                spacing: 15
                Rectangle { width: 50; height: 50; radius: 6; color: "#FF6B9A" }
                Column {
                    Text { text: "Выбранный трек"; color: "white"; font.bold: true }
                    Text { text: "Локальный файл"; color: "#B3B3BC"; font.pixelSize: 12 }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                spacing: 8
                
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 25
                    
                    Text { 
                        text: "📂" 
                        font.pixelSize: 24
                        color: folderMouseArea.containsMouse ? "white" : "#B3B3BC" 
                        MouseArea { 
                            id: folderMouseArea; anchors.fill: parent; hoverEnabled: true
                            onClicked: fileDialog.open()
                        }
                    }

                    Text { text: "⏮"; color: "white"; font.pixelSize: 20 }
                    
                    Rectangle { 
                        width: 48; height: 48; radius: 24; 
                        color: playMouseArea.containsMouse ? "#E6E6E8" : "white"
                        
                        Text { 
                            anchors.centerIn: parent; 
                            text: isPlaying ? "⏸" : "▶" 
                            font.pixelSize: 24
                            color: "black" 
                        }
                        
                        MouseArea { 
                            id: playMouseArea; anchors.fill: parent; hoverEnabled: true 
                            onClicked: {
                                if (isPlaying) {
                                    audioPlayer.pauseQml()
                                    isPlaying = false
                                } else {
                                    audioPlayer.resumeQml()
                                    isPlaying = true
                                }
                            }
                        }
                    }
                    
                    Text { text: "⏭"; color: "white"; font.pixelSize: 20 }
                }

                RowLayout {
                    Layout.fillWidth: true; Layout.maximumWidth: 500
                    Text { text: "0:00"; color: "#808089"; font.pixelSize: 11 }
                    Rectangle { Layout.fillWidth: true; height: 4; radius: 2; color: "#2A2A33"; Rectangle { width: parent.width * 0.3; height: parent.height; radius: 2; color: "#7B61FF" } }
                    Text { text: "0:00"; color: "#808089"; font.pixelSize: 11 }
                }
            }

            RowLayout {
                Layout.preferredWidth: 200
                Layout.alignment: Qt.AlignRight
                spacing: 15
                
                Text { 
                    text: "📄" 
                    color: isRightPanelOpen ? "white" : "#B3B3BC" 
                    font.pixelSize: 16
                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: -10
                        cursorShape: Qt.PointingHandCursor
                        onClicked: isRightPanelOpen = !isRightPanelOpen
                    }
                }
                
                Text { text: "🔊"; color: "white" }
                Rectangle { Layout.fillWidth: true; height: 4; radius: 2; color: "#2A2A33"; Rectangle { width: parent.width * 0.7; height: parent.height; radius: 2; color: "white" } }
            }
        }
    }

    component MenuButton: Rectangle {
        property string text: ""
        property bool active: false
        Layout.fillWidth: true
        height: 40
        radius: 8
        color: active ? "#1F1F28" : "transparent"
        
        Text {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left; anchors.leftMargin: 15
            text: parent.text
            color: active ? "white" : "#B3B3BC"
            font.bold: active
        }
        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; hoverEnabled: true; onEntered: parent.color = "#1F1F28"; onExited: parent.color = active ? "#1F1F28" : "transparent" }
    }

    component SectionHeader: RowLayout {
        property string title: ""
        Layout.fillWidth: true
        Layout.topMargin: 10
        Text { text: title; color: "white"; font.pixelSize: 20; font.bold: true; Layout.fillWidth: true }
        Text { text: "Показать все"; color: "#808089"; font.pixelSize: 12 }
    }
}