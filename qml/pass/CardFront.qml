import QtQuick 2.0
import Ubuntu.Components 1.3
import QtQuick.Layouts 1.11

Rectangle {
   id: passCard
   property var pass
   property bool selected: false
   property color foregroundColor: pass.standard.foregroundColor || "black"
   property color labelColor: pass.standard.labelColor || foregroundColor
   property color backgroundColor: pass.standard.backgroundColor || "white"
   radius: units.gu(4)
   signal infoButtonPressed()
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
         anchors.leftMargin: units.gu(1.5)
         anchors.verticalCenter: parent.verticalCenter
         height: units.gu(7)
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

   MouseArea {
      anchors.fill: parent
      onClicked: {
         emit: cardClicked()
      }
   }

   Flickable {
      id: passContents
      anchors.top: logoTextContainer.bottom
      anchors.topMargin: units.gu(4)
      anchors.left: parent.left
      anchors.leftMargin: units.gu(1)
      anchors.right: parent.right
      anchors.rightMargin: units.gu(2)
      anchors.bottom: barcodeContent.top
      anchors.bottomMargin: units.gu(4)

      contentHeight: contentItem.childrenRect.height
      contentWidth: parent.width - units.gu(3)
      clip: true

      MouseArea {
         anchors.fill: parent
         onClicked: {
            emit: cardClicked()
         }
      }

      Column {
         id: fields
         spacing: units.gu(3)
         width: parent.width

         // primary fields

         PassPrimaryFields {
            anchors.left: parent.left
            anchors.right: parent.right

            passId: passCard.pass.id
            style: passCard.pass.details.style
            primaryFields: passCard.pass.details.primaryFields
            foregroundColor: passCard.foregroundColor
            labelColor: passCard.labelColor
         }

         Grid {
            id: grid
            property int numCols: passCard.pass.details.style === "boardingPass" ? 3 : 2
            property double colSpacing: units.gu(2)
            property double colWidth: grid.width/numCols - (numCols-1)*colSpacing

            anchors.left: parent.left
            anchors.right: parent.right
            columns: numCols
            rowSpacing: units.gu(2)
            columnSpacing: colSpacing

            // secondary fields

            Repeater {
               model: passCard.pass.details.secondaryFields

               delegate: PassField {
                  Layout.fillWidth: true
                  field: modelData
                  width: grid.colWidth
                  foregroundColor: passCard.foregroundColor
                  labelColor: passCard.labelColor
               }
            }

            // auxiliary fields

            Repeater {
               model: passCard.pass.details.auxiliaryFields

               delegate: PassField {
                  Layout.fillWidth: true
                  field: modelData
                  width: grid.colWidth
                  foregroundColor: passCard.foregroundColor
                  labelColor: passCard.labelColor
               }
            }
         }
      }
   }

   // the scannable code (QR, PDF417, Code128, Aztec)

   Column {
      id: barcodeContent
      anchors.horizontalCenter: parent.horizontalCenter
      anchors.bottom: infoIcon.top

      PassBarcode {
         id: barcodeImage
         anchors.horizontalCenter: parent.horizontalCenter
         width: passCard.width * 0.6
         height: (passCard.pass.standard.barcode.format === "PKBarcodeFormatQR"
                  || passCard.pass.standard.barcode.format === "PKBarcodeFormatAztec")
                 ? passCard.width * 0.6
                 : passCard.width * 0.3

         passID: passCard.pass.id
         expired: passCard.pass.standard.expired
      }

      Text {
         anchors.horizontalCenter: parent.horizontalCenter
         font.pointSize: units.gu(1)
         text: passCard.pass.standard.barcode.altText
         visible: !!passCard.pass.standard.barcode.altText
         color: passCard.foregroundColor
      }
   }

   Icon {
      id: infoIcon
      height: units.gu(3)
      width: units.gu(3)
      anchors.right: parent.right
      anchors.rightMargin: units.gu(2)
      anchors.bottom: parent.bottom
      anchors.bottomMargin: units.gu(2)
      visible: selected
      name: "info"
      color: passCard.foregroundColor

      MouseArea {
         anchors.fill: parent
         onClicked: {
            emit: infoButtonPressed()
         }
      }
   }
}
