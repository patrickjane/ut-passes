import QtQuick 2.5
import Lomiri.Components 1.3
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.11

import QtQuick.Window 2.11

Rectangle {
   id: passCard
   property var pass
   property bool selected: false
   property color foregroundColor: pass.standard.foregroundColor || "black"
   property color labelColor: pass.standard.labelColor || foregroundColor
   property color backgroundColor: pass.standard.backgroundColor || "white"

   // iPhone 6: 750x1334 -> strip size shall be 375 x 98 (= 750 x 196)
   // as per https://developer.apple.com/library/archive/documentation/UserExperience/Conceptual/PassKit_PG/Creating.html

   property double stripSizeFactor: (passCard.width - 2) / 750.0
   property double stripWidth: 750.0 * stripSizeFactor
   property double stripHeight: 196.0 * stripSizeFactor

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

   // primary fields (on top of the strip)

   PassPrimaryFields {
      id: primaryFields
      anchors.top: logoTextContainer.bottom
      anchors.left: parent.left
      anchors.leftMargin: units.gu(1)
      anchors.right: parent.right
      anchors.rightMargin: units.gu(1)

      passId: passCard.pass.id
      style: passCard.pass.details.style
      primaryFields: passCard.pass.details.primaryFields
      foregroundColor: passCard.pass.standard.stripExtraForegroundColor || passCard.foregroundColor
      labelColor: passCard.pass.standard.stripExtraLabelColor || passCard.labelColor

      z: stripContainer.z + 1
   }

   // strip image

   Rectangle {
      id: stripContainer
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.leftMargin: 1
      anchors.rightMargin: 1
      anchors.top: logoTextContainer.bottom
      width: stripWidth
      height: passCard.pass.haveStripImage ? stripHeight : primaryFields.height

      color: "transparent"
      clip: true

      Image {
         property double factor: stripWidth / sourceSize.width

         anchors.centerIn: parent
         width: stripWidth
         height: sourceSize.height * factor

         fillMode: Image.PreserveAspectFit
         source: "image://passes/" + passCard.pass.id + "/strip"
      }
   }

   // rest of fields

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

//         PassPrimaryFields {
//            anchors.left: parent.left
//            anchors.right: parent.right

//            passId: passCard.pass.id
//            style: passCard.pass.details.style
//            primaryFields: passCard.pass.details.primaryFields
//            foregroundColor: passCard.foregroundColor
//            labelColor: passCard.labelColor
//         }

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

   ListView {
      id: barcodeContent

      property double codeWidth: switch (passCard.pass.standard.barcodeFormat) {
                                    case "PKBarcodeFormatQR":
                                        passCard.width * 0.75
                                        break
                                    case "PKBarcodeFormatAztec":
                                        passCard.width * 0.9
                                        break
                                    default:
                                        passCard.width
                                }
      property double codeHeight: switch (passCard.pass.standard.barcodeFormat) {
                                    case "PKBarcodeFormatQR":
                                        passCard.width * 0.75
                                        break
                                    case "PKBarcodeFormatAztec":
                                        passCard.width * 0.9
                                        break
                                    default:
                                        passCard.width * 0.4
                                }

      anchors.horizontalCenter: parent.horizontalCenter
      anchors.bottom: infoIconRect.top
      width: codeWidth
      height: codeHeight + units.gu(3)
      contentWidth: passCard.pass.standard.barcodes.length * barcodeContent.codeWidth
      contentHeight: codeHeight + units.gu(3)

      orientation: ListView.Horizontal
      clip: true
      highlightRangeMode: ListView.StrictlyEnforceRange
      snapMode: ListView.SnapToItem

      model: passCard.pass.standard.barcodes

      delegate: Column {
         PassBarcode {
            id: barcodeImage
            anchors.horizontalCenter: parent.horizontalCenter
            width: barcodeContent.codeWidth
            height: barcodeContent.codeHeight

            passID: passCard.pass.bundleId || passCard.pass.id
            bundleIndex: passCard.pass.bundleIndex
            expired: passCard.pass.standard.expired
            index: index
         }

         Text {
            anchors.horizontalCenter: parent.horizontalCenter
            font.pointSize: units.gu(1)
            text: modelData.altText
            visible: !!modelData.altText
            color: passCard.foregroundColor
         }
      }
   }

   PageIndicator {
      currentIndex: barcodeContent.currentIndex
      count: passCard.pass.standard.barcodes.length

      anchors.top: barcodeContent.bottom
      anchors.horizontalCenter: parent.horizontalCenter
      height: infoIconRect.height
      visible: count > 1
   }


   Rectangle {
      id: infoIconRect
      height: units.gu(3)
      width: units.gu(3)
      anchors.right: parent.right
      anchors.rightMargin: units.gu(2)
      anchors.bottom: parent.bottom
      anchors.bottomMargin: units.gu(2)
      color: backgroundColor

      Icon {
         id: infoIcon
         anchors.fill: parent
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
