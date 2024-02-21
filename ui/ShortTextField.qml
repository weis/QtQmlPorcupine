import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12

Item {
    id: ctrl
    property string text
    property alias inputFocus: txf.focus

    onTextChanged: function () {
        if(!ctrl.inputFocus)
            txf.text = tm.shorten();
    }

    onWidthChanged: function () {
        if(!inputFocus)
            txf.text = tm.shorten();
    }

    onInputFocusChanged: function () {
        if(inputFocus) {
            txf.text = ctrl.text;
        } else {
            txf.text = tm.shorten()
        }
    }

    TextMetrics {
        id: tm
        function shorten(text) {
            tm.text = ctrl.text;
            tm.elideWidth = txf.width;
            return tm.elidedText;
        }

        font: defaultFont
        elide: Text.ElideLeft
    }

    TextField {
        id: txf
        anchors.fill: parent

        leftPadding: mm(1)
        rightPadding: mm(1)

        onTextChanged: {
            if(ctrl.inputFocus) {
                ctrl.text = txf.text;
            }
        }
    }
}
