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

                        // Шарик на конце полоски
                        Rectangle {
                            width: 12
                            height: 12
                            radius: 6
                            color: "#FFFFFF"
                            border.color: "#7B61FF"
                            border.width: 2
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.right
                        }
                    }
                }

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

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            console.log("Кнопка Play нажата")
                        }
                    }
                }
            }
        }