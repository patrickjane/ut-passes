import QtQuick 2.0
import QtGraphicalEffects 1.0

Rectangle {
   id: barcodeImageItem
   property string passID
   property int bundleIndex
   property bool expired
   property int index: 0
   radius: 10

   Image {
      id: barcodeImage
      anchors.fill: parent
      anchors.leftMargin: units.gu(3)
      anchors.rightMargin: units.gu(3)
      anchors.topMargin: units.gu(3)
      anchors.bottomMargin: units.gu(3)
      source: "image://passes/" + barcodeImageItem.passID + "/barcode/" + index + "/" + bundleIndex
   }

   BrightnessContrast {
      anchors.fill: barcodeImage
      source: barcodeImage
      brightness: 0.7
      visible: barcodeImageItem.expired
   }

   Text {
      anchors.horizontalCenter: parent.horizontalCenter
      anchors.bottom: parent.bottom
      anchors.bottomMargin: units.gu(0.9)

      font.pointSize: units.gu(1)
      text: i18n.tr('This pass has expired')

      visible: barcodeImageItem.expired
   }
}
