import QtQuick 2.0
import Ubuntu.Components 1.3

Rectangle {
   id: passCard
   property var pass
   property color foregroundColor: pass.standard.foregroundColor || "black"
   property color labelColor: pass.standard.labelColor || foregroundColor
   property color backgroundColor: pass.standard.backgroundColor || "white"
   radius: units.gu(4)
   signal backButtonPressed()
   signal cardClicked()

   color: backgroundColor
   border.width: 1
   border.color: Qt.rgba(backgroundColor.r * 0.6, backgroundColor.g * 0.6, backgroundColor.b * 0.6, backgroundColor.a * 0.6)

   // header line (Logo text + Header fields)

   Rectangle {
      id: logoTextContainer
      anchors.left: parent.left
      anchors.leftMargin: units.gu(2)
      anchors.top: parent.top
      anchors.topMargin: 1

      height: units.gu(7)
      width: parent.width - units.gu(2)

      color: "transparent"

      Text {
         id: logoText
         anchors.left: parent.left
         anchors.verticalCenter: parent.verticalCenter

         font.pointSize: units.gu(1.5)
         text: passCard.pass &&
               (passCard.pass.standard.logoText || "")
               || ""
         color: passCard.foregroundColor
      }

      Image {
         anchors.left: parent.left
         anchors.leftMargin: units.gu(1.1)
         anchors.verticalCenter: parent.verticalCenter
         height: units.gu(6)
         fillMode: Image.PreserveAspectFit
         source: "image://passes/" + passCard.pass.id + "/logo"
         visible: !logoText.text
      }
   }

   Row {
      id: headerFields
      anchors.right: parent.right
      anchors.rightMargin: units.gu(2)
      anchors.top: parent.top

      height: units.gu(7)
      width: childrenRect.width
      spacing: units.gu(1.5)

      Repeater {
         model: passCard.pass.details.headerFields.reverse()

         delegate: PassHeaderField {
            field: modelData
            foregroundColor: passCard.foregroundColor
            labelColor: passCard.labelColor
         }
      }
   }

   Flickable {
      anchors.top: logoTextContainer.bottom
      anchors.topMargin: units.gu(4)
      anchors.left: parent.left
      anchors.leftMargin: units.gu(1)
      anchors.right: parent.right
      anchors.rightMargin: units.gu(2)
      anchors.bottom: parent.bottom
      anchors.bottomMargin: units.gu(8)

      contentHeight: contents.implicitHeight
      contentWidth: parent.width - units.gu(3)
      clip: true

      Column {
         id: contents
         spacing: units.gu(3)
         width: parent.width
         Repeater {
            model: passCard.pass.details.backFields

            delegate: Column {
               width: parent.width

               Text {
                  anchors.left: parent.left
                  text: modelData.label.toUpperCase()
                  font.pointSize: units.gu(1)
                  font.bold: true
                  color: labelColor
               }

               Text {
                  anchors.left: parent.left
                  text: modelData.value
                  font.pointSize: units.gu(1.5)
                  color: foregroundColor
                  wrapMode: Text.WordWrap
                  width: parent.width
                  onLinkActivated: Qt.openUrlExternally(link)
               }
            }
         }
      }
   }

   Rectangle {
      height: units.gu(3)
      width: units.gu(3)
      anchors.right: parent.right
      anchors.rightMargin: units.gu(2)
      anchors.bottom: parent.bottom
      anchors.bottomMargin: units.gu(2)
      color: backgroundColor

      Icon {
         anchors.fill: parent
         name: "mail-reply"
         color: passCard.foregroundColor
      }

      MouseArea {
         anchors.fill: parent
         onClicked: {
            emit: backButtonPressed()
         }
      }
   }
}
