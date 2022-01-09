import QtQuick 2.5
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

      Row {
         anchors.left: parent.left
         anchors.leftMargin: units.gu(1.1)
         anchors.top: parent.top
         anchors.bottom: parent.bottom

         Image {
            anchors.verticalCenter: parent.verticalCenter
            height: units.gu(6)
            fillMode: Image.PreserveAspectFit
            source: "image://passes/" + passCard.pass.id + "/logo"
         }

         Text {
            id: logoText
            anchors.verticalCenter: parent.verticalCenter

            font.pointSize: units.gu(1.5)
            text: passCard.pass &&
                  (passCard.pass.standard.logoText || "")
                  || ""
            color: passCard.foregroundColor
         }
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

   // strip image

   Rectangle {
      id: stripContainer
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.leftMargin: 1
      anchors.rightMargin: 1
      anchors.top: logoTextContainer.bottom

      height: childrenRect.height

      Image {
         id: stripImage
         anchors.top: parent.top
         anchors.left: parent.left
         anchors.right: parent.right
         fillMode: Image.PreserveAspectCrop
         source: "image://passes/" + passCard.pass.id + "/strip"
      }
   }

   // primary fields & rest of fields

   Flickable {
      id: passContents
      anchors.top: stripContainer.bottom
      anchors.topMargin: units.gu(2)
      anchors.left: parent.left
      anchors.leftMargin: units.gu(1)
      anchors.right: parent.right
      anchors.rightMargin: units.gu(1)
      anchors.bottom: barcodeContent.top
      anchors.bottomMargin: units.gu(4)

      contentHeight: contentItem.childrenRect.height
      contentWidth: parent.width - units.gu(2)
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
         anchors.left: parent.left

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
            property double colSpacing: units.gu(1)
            property double colWidth2: (grid.width - (2-1)*colSpacing)/2
            property double colWidth3: (grid.width - (3-1)*colSpacing)/3
            property int numCols: passCard.pass.details.style !== "boardingPass"
                                  ? 2
                                  : (passCard.pass.details.maxFieldLabelWidth > colWidth3 ? 2 : 3)

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
                  width: grid.numCols == 2 ? grid.colWidth2 : grid.colWidth3
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
                  width: grid.numCols == 2 ? grid.colWidth2 : grid.colWidth3
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

   Image {
      id: passIcon
      height: units.gu(3)
      width: units.gu(3)
      anchors.left: parent.left
      anchors.leftMargin: units.gu(2)
      anchors.bottom: parent.bottom
      anchors.bottomMargin: units.gu(2)
      source: "image://passes/" + passCard.pass.id + "/icon"
   }
}
