import QtQuick // базовые графические элементы
import QtQuick.Controls // кнопки
import QtQuick.Layouts // расстановка элементов в сетке или строках
import QtQuick.Dialogs // вызов системного окна (выбора файлов)

ApplicationWindow {
    id: window // имя
    width: 1200 // ширина окна
    height: 720 // высота окна
    visible: true // видимость
    title: qsTr("Анализатор небезопасных функций C/C++") // заголовок
    color: "#101010" // цвет

    property alias filesModel: filesModel
    font.family: "Consolas" // шрифт

    // список путей к выбранным файлам
    ListModel {
        id: filesModel
    }

    // список результатов из бэкенда
    property var findingsList: backend ? backend.findings : []

    // перебор всех элементов в filesModel и упаковка их пути в JS-массив
    function currentPaths() {
        var paths = []
        for (var i = 0; i < filesModel.count; ++i)
            paths.push(filesModel.get(i).path)
        return paths
    }

    // системное окно для выбора файлов
    FileDialog {
        id: fileDialog
        title: qsTr("Выберите .cpp файлы для анализа") // заголовок
        nameFilters: [qsTr("C++ файлы (*.cpp)")] // фильтр только C++ файлов
        fileMode: FileDialog.OpenFiles
        onAccepted: { // когда юзер подтверждает сигнал
            for (var i = 0; i < selectedFiles.length; ++i) {
                var rawUrl = selectedFiles[i].toString()
                filesModel.append({ path: rawUrl, name: rawUrl.split("/").pop() })
            }
        }
    }

    // визуальный интерфейс
    Rectangle {
        anchors.fill: parent
        color: "#101010"

        RowLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 16

            // левая панель (выбор файлов)
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
                        // файлы для анализа
                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 40
                            radius: 6
                            color: "#202020"

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 8
                                spacing: 8
                                // alignment: Qt.AlignVCenter // RowLayout центрирует по вертикали по умолчанию [cite: 12]

                                Text {
                                    Layout.fillWidth: true
                                    Layout.alignment: Qt.AlignVCenter
                                    text: name
                                    color: "#f0f0f0"
                                    elide: Text.ElideMiddle
                                    font.pixelSize: 13
                                }

                                ToolButton {
                                    text: "×"
                                    Layout.alignment: Qt.AlignVCenter
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

            // --- правая панель (результаты) ---
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

                    // верхняя панель: заголовок + кнопка "исправить все"
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Text {
                            text: qsTr("Результаты анализа")
                            color: "white"
                            font.pixelSize: 18
                            Layout.fillWidth: true
                        }

                        Button {
                            id: applyAllBtn
                            text: qsTr("Исправить всё")
                            visible: findingsList.length > 0

                            onClicked: backend.applyAllFixes()

                            background: Rectangle {
                                implicitWidth: 120
                                implicitHeight: 32
                                color: applyAllBtn.down ? "#15803d" : (applyAllBtn.hovered ? "#16a34a" : "#22c55e")
                                radius: 6
                            }

                            contentItem: Text {
                                text: applyAllBtn.text
                                color: "white"
                                font.pixelSize: 12
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }

                    // сообщения о статусе
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

                            // отображение каждой найденной ошибки
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

                                        // заголовок карточки
                                        RowLayout {
                                            Layout.fillWidth: true
                                            Text {
                                                text: findingData.fileName + ":" + findingData.lineNumber + " — " + findingData.functionName + "()"
                                                color: "#f0f0f0"
                                                font.bold: true
                                                Layout.fillWidth: true
                                            }

                                            // кнопка копирования
                                            Button {
                                                id: copyBtn
                                                property string originalText: qsTr("Копировать") // Храним исходный текст
                                                text: originalText
                                                visible: findingData.fixedLine && findingData.fixedLine.length > 0

                                                onClicked: {
                                                    backend.copyToClipboard(findingData.fixedLine)
                                                    copyBtn.text = qsTr("Скопировано!") // Меняем текст
                                                    resetTimer.restart() // Запускаем таймер сброса
                                                }

                                                // таймер, который вернет текст кнопки назад через 3 секунды
                                                Timer {
                                                    id: resetTimer
                                                    interval: 3000
                                                    onTriggered: copyBtn.text = copyBtn.originalText
                                                }

                                                background: Rectangle {
                                                    implicitWidth: 110
                                                    implicitHeight: 24
                                                    color: copyBtn.down ? "#15803d" : (copyBtn.hovered ? "#374151" : "#1f2937")
                                                    radius: 4
                                                    border.color: copyBtn.text === qsTr("Скопировано!") ? "#22c55e" : "transparent"
                                                }

                                                contentItem: Text {
                                                    text: copyBtn.text
                                                    color: copyBtn.text === qsTr("Скопировано!") ? "#22c55e" : "#6bd96b"
                                                    font.pixelSize: 10
                                                    font.bold: copyBtn.text === qsTr("Скопировано!")
                                                    horizontalAlignment: Text.AlignHCenter
                                                    verticalAlignment: Text.AlignVCenter
                                                }
                                            }
                                        }

                                        // блок с кодом
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

                                                // компонент для отрисовки одной строки кода с номером
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
                                                            textFormat: Text.PlainText
                                                        }
                                                    }
                                                }

                                                // контекст до
                                                Repeater {
                                                    model: findingData.beforeLines || []
                                                    Loader {
                                                        sourceComponent: codeLineRow
                                                        onLoaded: { item.txt = modelData.text; item.num = modelData.ln }
                                                    }
                                                }

                                                // строка с ошибкой
                                                Loader {
                                                    sourceComponent: codeLineRow
                                                    onLoaded: {
                                                        item.txt = findingData.codeLine
                                                        item.num = findingData.lineNumber
                                                        item.clr = "#ff5c5c" // Красный
                                                    }
                                                }

                                                // предложенное исправление
                                                Loader {
                                                    sourceComponent: codeLineRow
                                                    visible: findingData.fixedLine.length > 0
                                                    onLoaded: {
                                                        item.txt = findingData.fixedLine
                                                        item.num = 0 // У фикса нет номера строки
                                                        item.clr = "#6bd96b" // Зеленый
                                                    }
                                                }

                                                // контекст после
                                                Repeater {
                                                    model: findingData.afterLines || []
                                                    Loader {
                                                        sourceComponent: codeLineRow
                                                        onLoaded: { item.txt = modelData.text; item.num = modelData.ln }
                                                    }
                                                }
                                            }
                                        }

                                        // описание проблемы
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