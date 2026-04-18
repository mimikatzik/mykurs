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

    // Список результатов из C++
    property var findingsList: backend ? backend.findings : []

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
                var rawUrl = selectedFiles[i].toString()
                filesModel.append({ path: rawUrl, name: rawUrl.split("/").pop() })
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

            // --- ЛЕВАЯ ПАНЕЛЬ (Выбор файлов) ---
            Rectangle {
                Layout.preferredWidth: parent.width * 0.30
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
                                Text {
                                    Layout.fillWidth: true
                                    text: name
                                    color: "#f0f0f0"
                                    elide: Text.ElideMiddle
                                }
                                ToolButton {
                                    text: "×"
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
                                                }
                                            }

                                            background: Rectangle {
                                                implicitHeight: 40
                                                radius: 10
                                                color: !scanButton.enabled ? "#1f2937" : (scanButton.down ? "#16a34a" : "#22c55e")
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

            // --- ПРАВАЯ ПАНЕЛЬ (Результаты) ---
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

                    // Сообщения о статусе
                    Text {
                        visible: !backend || (backend && !backend.hasScanRun)
                        text: qsTr("Добавьте файлы и нажмите «Сканировать».")
                        color: "#888888"
                    }

                    Text {
                        visible: backend && backend.hasScanRun && findingsList.length === 0
                        text: qsTr("Уязвимостей не обнаружено!")
                        color: "#22c55e"
                    }

                    ScrollView {
                        id: findingsScrollView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true

                        Column {
                            id: findingsColumn
                            width: findingsScrollView.availableWidth
                            spacing: 12

                            // Используем Repeater для отображения каждой найденной ошибки
                            Repeater {
                                model: findingsList

                                delegate: Rectangle {
                                    width: findingsColumn.width
                                    radius: 10
                                    color: "#202020"
                                    border.color: "#303030"
                                    implicitHeight: contentLayout.implicitHeight + 24
                                    property var findingData: modelData

                                    ColumnLayout {
                                        id: contentLayout
                                        anchors.fill: parent
                                        anchors.margins: 12
                                        spacing: 10

                                        // Заголовок карточки
                                        RowLayout {
                                            Layout.fillWidth: true
                                            Text {
                                                text: findingData.fileName + ":" + findingData.lineNumber + " — " + findingData.functionName + "()"
                                                color: "#f0f0f0"
                                                font.bold: true
                                                Layout.fillWidth: true
                                            }

                                            // Кнопка копирования
                                            Button {
                                                id: copyBtn
                                                text: qsTr("Копировать")
                                                visible: findingData.fixedLine && findingData.fixedLine.length > 0
                                                onClicked: backend.copyToClipboard(findingData.fixedLine)

                                                background: Rectangle {
                                                    implicitWidth: 110
                                                    implicitHeight: 24
                                                    color: copyBtn.hovered ? "#374151" : "#1f2937"
                                                    radius: 4
                                                }
                                                contentItem: Text {
                                                    text: copyBtn.text
                                                    color: "#6bd96b"
                                                    font.pixelSize: 10
                                                    horizontalAlignment: Text.AlignHCenter
                                                    verticalAlignment: Text.AlignVCenter
                                                }
                                            }
                                        }

                                        // Блок с кодом (черный фон)
                                        Rectangle {
                                            Layout.fillWidth: true
                                            implicitHeight: codeLinesColumn.implicitHeight + 20
                                            color: "#0a0a0a"
                                            radius: 6

                                            Column {
                                                id: codeLinesColumn
                                                anchors.fill: parent
                                                anchors.margins: 10
                                                spacing: 2

                                                // Компонент для отрисовки ОДНОЙ строки кода с номером
                                                Component {
                                                    id: codeLineRow
                                                    RowLayout {
                                                        width: codeLinesColumn.width
                                                        spacing: 10
                                                        property string txt: ""
                                                        property int num: 0
                                                        property color clr: "#cccccc"

                                                        Text {
                                                            text: num > 0 ? num : ""
                                                            color: "#555555"
                                                            font.pixelSize: 12
                                                            Layout.preferredWidth: 30
                                                            horizontalAlignment: Text.AlignRight
                                                        }
                                                        Text {
                                                            text: txt
                                                            color: clr
                                                            font.pixelSize: 12
                                                            Layout.fillWidth: true
                                                            // Сохраняем все пробелы и табы
                                                            textFormat: Text.PlainText
                                                        }
                                                    }
                                                }

                                                // 1. Контекст "ДО"
                                                Repeater {
                                                    model: findingData.beforeLines || []
                                                    Loader {
                                                        sourceComponent: codeLineRow
                                                        onLoaded: { item.txt = modelData.text; item.num = modelData.ln }
                                                    }
                                                }

                                                // 2. СТРОКА С ОШИБКОЙ
                                                Loader {
                                                    sourceComponent: codeLineRow
                                                    onLoaded: {
                                                        item.txt = findingData.codeLine
                                                        item.num = findingData.lineNumber
                                                        item.clr = "#ff5c5c" // Красный
                                                    }
                                                }

                                                // 3. ПРЕДЛОЖЕННОЕ ИСПРАВЛЕНИЕ
                                                Loader {
                                                    sourceComponent: codeLineRow
                                                    visible: findingData.fixedLine.length > 0
                                                    onLoaded: {
                                                        item.txt = findingData.fixedLine
                                                        item.num = 0 // У фикса нет номера строки
                                                        item.clr = "#6bd96b" // Зеленый
                                                    }
                                                }

                                                // 4. Контекст "ПОСЛЕ"
                                                Repeater {
                                                    model: findingData.afterLines || []
                                                    Loader {
                                                        sourceComponent: codeLineRow
                                                        onLoaded: { item.txt = modelData.text; item.num = modelData.ln }
                                                    }
                                                }
                                            }
                                        }

                                        // Описание проблемы
                                        Text {
                                            Layout.fillWidth: true
                                            text: findingData.warning
                                            color: "#ff8a8a"
                                            font.pixelSize: 12
                                            wrapMode: Text.WordWrap
                                        }

                                        Text {
                                            Layout.fillWidth: true
                                            text: findingData.recommendation
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