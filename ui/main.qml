import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.3
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1
import Qt.labs.settings 1.0
import QtQuick.Controls.Universal 2.12
import Porcupine.Components 1.0

Window {
    width: isMobile ?  availWidth : mm(90)
    height: isMobile ?  availHeight : mm(140)
    visible: true
    title: 'Porcupine Check'

    readonly property real availWidth: Screen.desktopAvailableWidth
    readonly property real availHeight: Screen.desktopAvailableHeight
    readonly property bool isMobile: Qt.platform.os === "android" || Qt.platform.os === "ios"
    readonly property font defaultFont: Qt.platform.os === 'windows'
                                        ?  Qt.font({
                                                       "family": 'Calibri',
                                                       "pixelSize": mm(3)
                                                   })
                                        : Qt.font({
                                                      "pixelSize": mm(3)
                                                  })

    readonly property font listFont: Qt.platform.os === 'windows'
                                     ?  Qt.font({
                                                    "family": 'Calibri',
                                                    "pixelSize": mm(8)
                                                })
                                     : Qt.font({
                                                   "pixelSize": mm(8)
                                               })


    /**
     * Converts millimeter to the number of pixel for the current display
     */
    function mm(size) {
        return Math.trunc(size * Screen.pixelDensity);
    }

    function markKeywordItem(row)
    {
        listView.itemAtIndex(row).color = "red";
    }

    function clearKeywordItems()
    {
        for (let row = 0; row < listView.count; row++) {
            listView.contentItem.children[row].color = "transparent";
        }
    }


    QmlPorcupine {
        id: porcupine
        pvAccessKey: ''
        sensitivity: 0.5
        onKeyWordDetected: function(keywordIndex) {
            markKeywordItem(keywordIndex);
        }

        onErrorChanged: function() {
            if(error) {
                textMessage.text = porcupine.errorMsg + '\n';
            }
        }

        onStarted:  function () {
            btnStart.pvStarted();
        }

        onStopped: function () {
            btnStop.pvStopped();
        }

        onInfoMessage: function (message) {
            textMessage.text+= message + '\n';
        }

    }

    Settings {
        id: pvSettings
        fileName: porcupine.toNativePathSyntax(StandardPaths.writableLocation(StandardPaths.TempLocation) + "/QtPorcupine.ini")
        Component.onCompleted: console.log("pvSettings = ", pvSettings.fileName)
        property alias pvAccessKey: porcupine.pvAccessKey
        property alias pvModelPath: porcupine.pvModelPath
        property alias pvKeyWordsDir: porcupine.pvKeyWordsDir
        property alias sensitivity: porcupine.sensitivity
    }

    Component.onCompleted: {
        txfAcessKey.text = pvSettings.pvAccessKey
        txfModel.text = pvSettings.pvModelPath
        txfKeyDir.text = pvSettings.pvKeyWordsDir
        slSense.value = pvSettings.sensitivity
    }

    ColumnLayout {

        anchors.margins: mm(2)
        anchors.fill: parent

        Rectangle {
            id: keywords
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: mm(2)
            border.width: Math.max(1, mm(0.25))
            border.color: "gray"

            ColumnLayout {
                anchors.fill: parent
                ListView {
                    id: listView
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    clip: true

                    model: porcupine.keywords

                    delegate:Rectangle {
                        height: mm(12)
                        width: listView.width
                        color: "transparent"
                        Text {
                            anchors.fill: parent
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font: listFont
                            text: display
                        }
                    }
                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                }

                Button {
                    Layout.preferredHeight: mm(5)
                    Layout.fillWidth: true
                    Layout.margins: mm(1)
                    font: defaultFont
                    text: 'Clear'
                    onClicked: clearKeywordItems();
                }
            }

        }

        Rectangle {
            id: info
            Layout.fillWidth: true
            Layout.preferredHeight: mm(15)
            border.width: 1
            border.color: "gray"
            clip: true
            ScrollView {
                anchors.fill: parent
                anchors.margins: mm(1)
                ScrollBar.vertical.policy: ScrollBar.AsNeeded
                ScrollBar.horizontal.policy: ScrollBar.AsNeeded

                TextArea {
                    id: textMessage
                    readOnly: true
                    clip: true
                    width: parent.width
                    wrapMode: Text.WordWrap
                    onTextChanged: cursorPosition = text.length-1
                }
            }
        }

        RowLayout {
            id: accessKey
            Layout.topMargin: mm(2)
            Layout.fillWidth: true
            Label {
                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: mm(20)
                font: defaultFont

                text: "Access Key:"
            }
            TextField {
                id: txfAcessKey
                Layout.preferredHeight: mm(6)
                Layout.fillWidth: true
                leftPadding: mm(1)
                rightPadding: mm(1)
                onTextChanged: porcupine.pvAccessKey = text
            }
        }

        RowLayout {
            id: modelPath
            Layout.fillWidth: true
            Button {
                Layout.preferredHeight: mm(6)
                Layout.preferredWidth: mm(20)
                font: defaultFont
                text: "Model file:"

                onClicked: modelFileDialog.visible = true;
            }

            ShortTextField {
                id: txfModel
                Layout.fillWidth: true
                Layout.preferredHeight: mm(6)

                onTextChanged: porcupine.pvModelPath = text
            }
        }

        RowLayout {
            id: keywordsDir
            Layout.fillWidth: true
            Button {
                Layout.preferredWidth: mm(20)
                Layout.preferredHeight: mm(6)
                text: "Keywords Dir:"
                onClicked: modelKeywordsDialog.visible = true
            }
            ShortTextField {
                id: txfKeyDir
                Layout.preferredHeight: mm(6)
                Layout.fillWidth: true
                onTextChanged: porcupine.pvKeyWordsDir = text
            }
        }

        RowLayout {
            id: sensivity
            Layout.fillWidth: true
            Layout.topMargin: mm(1)

            Label {
                Layout.alignment: Qt.AlignCenter
                Layout.preferredHeight: mm(5)
                Layout.preferredWidth: mm(20)
                font: defaultFont

                text: "Sensitivity:"
            }
            Slider {
                id: slSense
                from: 0
                to: 1
                value: 0.5
                enabled: !porcupine.engineReady
                Layout.preferredHeight: mm(5)
                Layout.fillWidth: true
                font: defaultFont
                onValueChanged: porcupine.sensitivity = value
            }
            Label {
                Layout.leftMargin: mm(2)
                Layout.alignment: Qt.AlignRight
                Layout.preferredHeight: mm(5)
                Layout.preferredWidth: mm(5)
                font: defaultFont
                text: slSense.value.toFixed(2)
            }
        }

        RowLayout {
            id: rwAction
            property int aniIdx: -1
            property bool busy: false
            Layout.fillWidth: true
            Layout.preferredHeight: mm(8)
            Layout.margins: mm(2)


            visible: true

            onBusyChanged: function () {
                if(busy) {
                    tmAni.start();
                } else {
                    tmAni.stop();
                    aniIdx = -1;
                }
            }

            Button {
                id: btnStart
                enabled: true
                Layout.fillWidth: true
                text: enabled ? '<font color="darkred">Start</font>' : "Start"

                function pvStarted() {
                    pvSettings.sync()
                    enabled = !enabled;
                    btnStop.enabled = !enabled;
                    rwAction.busy = true;
                }

                onClicked: {
                    clearKeywordItems();
                    porcupine.startListening();
                }
            }

            Rectangle {
                id: b0
                property bool active: (rwAction.aniIdx % 3) === 0

                width: mm(3)
                height: mm(3)
                border.width: Math.max(mm(0.25),1)
                border.color: "black"
                color: active ? "black" : "transparent"
            }

            Rectangle {
                id: b1
                property bool active: (rwAction.aniIdx % 3) === 1

                width: mm(3)
                height: mm(3)
                border.width: Math.max(mm(0.25),1)
                border.color: "black"
                color: active ? "black" : "transparent"
            }
            Rectangle {
                id: b2
                property bool active: (rwAction.aniIdx % 3) === 2

                width: mm(3)
                height: mm(3)
                border.width: Math.max(mm(0.25),1)
                border.color: "black"
                color: active ? "black" : "transparent"
            }

            Timer {
                id: tmAni
                interval: 500
                triggeredOnStart: true
                repeat: true
                onTriggered: function () {
                    rwAction.aniIdx++;
                }
            }

            Button {
                id: btnStop
                enabled: false
                Layout.fillWidth: true

                function pvStopped() {
                    enabled = !enabled
                    btnStart.enabled = !enabled
                    rwAction.busy = false;
                }

                onClicked: {
                    clearKeywordItems();
                    porcupine.stopListening();
                }
                text:  enabled ? '<font color="darkred">Stop</font>' : "Stop"
            }
        }

    }


    FileDialog {
        id: modelFileDialog
        title: "Select corpucine model file"
        nameFilters: ["Corpucine model file (*.pv)"]
        defaultSuffix: "pv"
        visible: false
        onAccepted: {
            txfModel.text =  porcupine.toNativePathSyntax(file);
        }
    }

    FolderDialog {
        id: modelKeywordsDialog
        title: "Select corpucine keyword folder"
        visible: false
        onAccepted: {
            var fname = porcupine.toNativePathSyntax(folder);
            txfKeyDir.text = fname;
        }
    }

}
