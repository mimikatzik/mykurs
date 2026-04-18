import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ApplicationWindow {
    id: window
    width: 1200
    height: 720
    visible: true
    title: qsTr("Анализатор небезопасных функций C/C++")
    color: "#101010"

    property alias filesModel: filesModel

    font.family: "Consolas"

    ListModel {
        id: filesModel
    }

    // Явно обновляем список по сигналу — иначе Repeater может не перерисоваться при изменении backend.findings
    property var findingsList: []

    Component.onCompleted: {
        console.log("DELEGATE CREATED:", fileName, fileNumber);
        if (backend)
            findingsList = backend.findings
    }

    Connections {
        target: backend
        function onFindingsChanged() {
            console.log("[QML] findingsChanged signal received")
            if (backend) {
                var list = backend.findings
                console.log("[QML] findings count:", list.length)
                findingsList = list
                console.log("[QML] findingsList updated, length:", findingsList.length)
            }
        }
    }

    function currentPaths() {
        var paths = []
        for (var i = 0; i < filesModel.count; ++i)
            paths.push(filesModel.get(i).path)
        return paths
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Выберите .cpp файлы для анализа")
        nameFilters: [qsTr("C++ файлы (*.cpp)")]
        fileMode: FileDialog.OpenFiles
        onAccepted: {
            for (var i = 0; i < selectedFiles.length; ++i) {
                var url = selectedFiles[i]
                var path = url.toString().replace("file:///", "")
                filesModel.append({ path: path, name: path.split(/[\\/]/).pop() })
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#101010"

        RowLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 16

            // Левая панель — выбор файлов
            Rectangle {
                Layout.preferredWidth: parent.width * 0.35
                Layout.fillHeight: true
                radius: 10
                color: "#181818"
                border.color: "#272727"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    Text {
                        text: qsTr("Файлы для анализа")
                        color: "white"
                        font.pixelSize: 18
                    }

                    Text {
                        text: qsTr("Добавьте .cpp файлы, затем запустите анализ.")
                        color: "#aaaaaa"
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                    }

                    ListView {
                        id: filesView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: filesModel
                        spacing: 4

                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 32
                            radius: 6
                            color: "#202020"

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 8

                                Text {
                                    Layout.fillWidth: true
                                    Layout.alignment: Qt.AlignVCenter
                                    text: name
                                    elide: Text.ElideMiddle
                                    color: "#f0f0f0"
                                }

                                ToolButton {
                                    text: "×"
                                    contentItem: Text {
                                        text: "×"
                                        font.pixelSize: 16
                                        color: "#888888"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    background: Rectangle { color: "transparent" }
                                    onClicked: filesModel.remove(index)
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Button {
                            id: addFilesButton
                            Layout.fillWidth: true
                            text: qsTr("Добавить файлы…")
                            onClicked: fileDialog.open()

                            background: Rectangle {
                                implicitHeight: 36
                                radius: 8
                                color: addFilesButton.down ? "#2563eb" : "#1d4ed8"
                                border.color: "#1e40af"
                            }

                            contentItem: Text {
                                text: addFilesButton.text
                                color: "white"
                                font.pixelSize: 13
                                font.family: window.font.family
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        Button {
                            id: clearButton
                            text: qsTr("Очистить")
                            onClicked: {
                                filesModel.clear()
                                if (backend)
                                    backend.clear()
                            }

                            background: Rectangle {
                                implicitWidth: 110
                                implicitHeight: 36
                                radius: 8
                                color: "transparent"
                                border.color: "#4b5563"
                            }

                            contentItem: Text {
                                text: clearButton.text
                                color: "#e5e7eb"
                                font.pixelSize: 13
                                font.family: window.font.family
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }

                    Button {
                        id: scanButton
                        Layout.fillWidth: true
                        text: qsTr("Сканировать файлы")
                        enabled: filesModel.count > 0
                            onClicked: {
                                if (backend) {
                                    console.log("[QML] Scan button clicked")
                                    backend.scanFiles(currentPaths())
                                    // Принудительно обновляем после сканирования
                                    Qt.callLater(function() {
                                        if (backend) {
                                            findingsList = backend.findings
                                            console.log("[QML] findingsList force-updated, length:", findingsList.length)
                                        }
                                    })
                                }
                            }

                        background: Rectangle {
                            implicitHeight: 40
                            radius: 10
                            color: !scanButton.enabled
                                   ? "#1f2937"
                                   : (scanButton.down ? "#16a34a" : "#22c55e")
                            border.color: "#15803d"
                        }

                        contentItem: Text {
                            text: scanButton.text
                            color: scanButton.enabled ? "white" : "#6b7280"
                            font.pixelSize: 14
                            font.bold: true
                            font.family: window.font.family
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }

            // Правая панель — результаты анализа
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 10
                color: "#181818"
                border.color: "#272727"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    Text {
                        text: qsTr("Результаты анализа")
                        color: "white"
                        font.pixelSize: 18
                    }

                    // Текст до первого запуска анализа
                    Text {
                        visible: !backend || (backend && !backend.hasScanRun)
                        text: qsTr("Анализ ещё не выполнялся. Добавьте файлы слева и нажмите «Сканировать файлы».")
                        color: "#888888"
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                    }

                    // Текст после анализа, когда небезопасные функции не найдены
                    Text {
                        visible: backend && backend.hasScanRun && findingsList.length === 0
                        text: qsTr("Сканирование завершено: небезопасные вызовы gets/strcpy/sprintf не найдены в выбранных файлах.")
                        color: "#22c55e"
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                    }

                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true

                        Column {
                            id: findingsColumn
                            width: parent.width
                            spacing: 12

                            Repeater {
                                model: findingsList

                                delegate: Rectangle {
                                    width: findingsColumn.width
                                    radius: 10
                                    color: "#202020"
                                    border.color: "#303030"
                                    border.width: 1

                                    property var findingData: modelData

                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 12
                                        spacing: 8

                                        Text {
                                            text: findingData.fileName + ":" + findingData.lineNumber + "  —  " + findingData.functionName + "()"
                                            color: "#f0f0f0"
                                            font.pixelSize: 13
                                            Layout.alignment: Qt.AlignHCenter
                                        }

                                        // Блок с фрагментом исходного кода
                                        Rectangle {
                                            Layout.fillWidth: true
                                            color: "#121212"
                                            radius: 8
                                            border.color: "#333333"
                                            border.width: 1

                                            Column {
                                                anchors.fill: parent
                                                anchors.margins: 10
                                                spacing: 3

                                                Repeater {
                                                    model: findingData.beforeLines || []
                                                    delegate: Text {
                                                        width: parent.width
                                                        text: modelData || ""
                                                        color: "#cccccc"
                                                        font.pixelSize: 12
                                                        font.family: window.font.family
                                                        wrapMode: Text.NoWrap
                                                    }
                                                }

                                                // Найденная строка — красным
                                                Text {
                                                    width: parent.width
                                                    text: findingData.codeLine || ""
                                                    color: "#ff5c5c"
                                                    font.pixelSize: 12
                                                    font.family: window.font.family
                                                    wrapMode: Text.NoWrap
                                                }

                                                // Зеленая строка с безопасным вариантом
                                                Text {
                                                    width: parent.width
                                                    visible: findingData.fixedLine && findingData.fixedLine.length > 0
                                                    text: findingData.fixedLine || ""
                                                    color: "#6bd96b"
                                                    font.pixelSize: 12
                                                    font.family: window.font.family
                                                    wrapMode: Text.NoWrap
                                                }

                                                Repeater {
                                                    model: findingData.afterLines || []
                                                    delegate: Text {
                                                        width: parent.width
                                                        text: modelData || ""
                                                        color: "#cccccc"
                                                        font.pixelSize: 12
                                                        font.family: window.font.family
                                                        wrapMode: Text.NoWrap
                                                    }
                                                }
                                            }
                                        }

                                        // Предупреждение
                                        Text {
                                            visible: findingData.warning && findingData.warning.length > 0
                                            text: findingData.warning || ""
                                            color: "#ff8a8a"
                                            font.pixelSize: 12
                                            wrapMode: Text.WordWrap
                                        }

                                        // Рекомендация по замене
                                        Text {
                                            visible: findingData.recommendation && findingData.recommendation.length > 0
                                            text: findingData.recommendation || ""
                                            color: "#8ee78e"
                                            font.pixelSize: 12
                                            wrapMode: Text.WordWrap
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
