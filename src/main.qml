import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: root
    visible: true
    width: 1280
    height: 800
    title: "Soundora Player"
    
    // Главный цвет фона (из твоей палитры)
    color: "#0F0F14" 

    // Основная сетка окна
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ВЕРХНЯЯ ЧАСТЬ (3 колонки)
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // 1. ЛЕВОЕ МЕНЮ (Навигация)
            Rectangle {
                Layout.preferredWidth: 240
                Layout.fillHeight: true
                color: "#16161D" 

                Rectangle {
                    width: parent.width; height: 1
                    color: "#2A2A33"
                    anchors.top: parent.right
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 24
                    spacing: 20

                    Text {
                        text: "Soundora"
                        color: "#FFFFFF"
                        font.pixelSize: 22
                        font.bold: true
                        Layout.bottomMargin: 20
                    }

                    Text { text: "Поиск"; color: "#808089"; font.pixelSize: 15; font.family: "Inter" }
                    Text { text: "Главная"; color: "#808089"; font.pixelSize: 15; font.bold: false; font.family: "Inter" }
                    Text { text: "Радио"; color: "#808089"; font.pixelSize: 15; font.family: "Inter" }
                    Text { text: "Подксты"; color: "#808089"; font.pixelSize: 15; font.family: "Inter"}

                    Text { text: "Моя медиатека"; color: "#808089"; font.pixelSize: 20; font.family: "Inter" }
                    Text { text: "Любимые треки"; color: "#808089"; font.pixelSize: 15; font.family: "Inter" }
                    Text { text: "Плейлисты"; color: "#808089"; font.pixelSize: 15; font.family: "Inter" }
                    Text { text: "Подписки"; color: "#808089"; font.pixelSize: 15; font.family: "Inter" }
                    Text { text: "История"; color: "#808089"; font.pixelSize: 15; font.family: "Inter" }
                    Text { text: "Загрузки"; color: "#808089"; font.pixelSize: 15; font.family: "Inter" }

                    Text { text: "Плейлисты"; color: "#808089"; font.pixelSize: 20; font.family: "Inter" }


                    Item { Layout.fillHeight: true } // Пустое место, чтобы прижать меню наверх
                }
            }

            // 2. ЦЕНТРАЛЬНАЯ ЧАСТЬ (Контент)
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#0F0F14"

                // Большой красивый баннер "Поток вдохновения"
                Rectangle {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 20
                    height: 280
                    radius: 20
                    color: "#1F1F28" // Цвет "Поверхности (приподнятые)"

                    Text {
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.margins: 30
                        text: "Поток\nвдохновения"
                        color: "#FFFFFF"
                        font.pixelSize: 40
                        font.bold: true
                    }

                    // Градиентная кнопка "Слушать"
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.margins: 30
                        width: 130; height: 44
                        radius: 8
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#5A67FF" }
                            GradientStop { position: 1.0; color: "#9B7CFF" }
                        }
                        
                        Text {
                            anchors.centerIn: parent
                            text: "Слушать"
                            color: "#FFFFFF"
                            font.pixelSize: 14
                            font.bold: true
                        }
                        
                        // Анимация при наведении
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: console.log("Клик по кнопке Слушать!")
                        }
                    }
                }
            }

            // 3. ПРАВАЯ ПАНЕЛЬ (Очередь и текущий трек)
            /*Rectangle {
                Layout.preferredWidth: 320
                Layout.fillHeight: true
                color: "#16161D" 
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 20

                    // Заголовок панели
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "Сейчас играет"; color: "white"; font.bold: true; font.pixelSize: 16; Layout.fillWidth: true }
                        Text { text: "v"; color: "white" }
                    }
                    // Обложка текущего трека
                    Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: width
                    radius: 12; color: "#FF4FD8"
                    gradient: Gradient { GradientStop {position: 0; color: "#FF4FD8"} GradientStop {position: 1; color: "#7B61FF"} }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Column {
                            Layout.fillWidth: true
                            Text { text: "Night Call"; color: "white"; font.bold: true; font.pixelSize: 20 }
                            Text { text: "Kavinsky"; color: "#B3B3BC"; font.pixelSize: 14 }
                        }
                        Text { text: "💜"; color: "#9B7CFF"; font.pixelSize: 20 }
                    }
                    Column {
                        Layout.fillWidth: true
                        Rectangle { width: parent.width; height: 4; radius: 2; color: "#2A2A33"; Rectangle { width: parent.width * 0.3; height: 4; radius: 2; color: "#7B61FF" } }
                        RowLayout { width: parent.width; Text { text: "1:24"; color: "#808089"; font.pixelSize: 11 } Item{Layout.fillWidth: true} Text { text: "4:18"; color: "#808089"; font.pixelSize: 11 } }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 20
                        Text { text: "🔀"; color: "#B3B3BC" }
                        Text { text: "⏮"; color: "white"; font.pixelSize: 20 }
                        Rectangle { width: 50; height: 50; radius: 25; color: "white"; Text { anchors.centerIn: parent; text: "⏸"; color: "black"; font.pixelSize: 20 } }
                        Text { text: "⏭"; color: "white"; font.pixelSize: 20 }
                        Text { text: "🔁"; color: "#B3B3BC" }
                    }
                }
            }*/
        }

        // 4. НИЖНЯЯ ПАНЕЛЬ (Плеер)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 90
            color: "#16161D" 
            
            // Тонкая линия разделитель сверху
            Rectangle {
                width: parent.width; height: 1
                color: "#2A2A33"
                anchors.top: parent.top
            }

            // Кнопки управления по центру
            RowLayout {
                anchors.centerIn: parent
                spacing: 25
                
                Text { text: "⏮"; color: "#B3B3BC"; font.pixelSize: 24 }
                
                // Круглая белая кнопка Play/Pause
                Rectangle {
                    width: 48; height: 48
                    radius: 24
                    color: "#FFFFFF"
                    Image {
                        anchors.centerIn: parent
                        source: "icons/play-solid-full.svg"
                        sourceSize.width: 20
                        sourceSize.height: 20
                    }
                }

                Text { text: "⏭"; color: "#B3B3BC"; font.pixelSize: 24 }
            }

            // Управление громкостью справа
            RowLayout {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: 40
                spacing: 15

                Text {
                    text: "🔊"
                    color: "#B3B3BC"
                    font.pixelSize: 18
                }

                // Полоска громкости
                Rectangle {
                    width: 100
                    height: 4
                    radius: 2
                    color: "#2A2A33"
                    
                    Rectangle {
                        width: parent.width * 0.6 // 60% громкости
                        height: parent.height
                        radius: 2
                        color: "#7B61FF"
                    }
                }
            }
        }
    }
}