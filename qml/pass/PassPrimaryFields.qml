import QtQuick 2.0
import QtGraphicalEffects 1.0

Item {
   id: fields
   property string passId
   property string style: "generic"
   property string thumbnail: ""
   property var primaryFields: []
   property color foregroundColor
   property color labelColor

   height: f.height

   Column {
      id: f
      anchors.left: parent.left
      width: fields.primaryFields.length === 1 ? parent.width : parent.width / 2
      height: childrenRect.height

      Text {
         width: parent.width
         visible: fields.primaryFields.length
         text: (fields.primaryFields.length && primaryFields[0].label || "").toUpperCase()
         wrapMode: Text.WordWrap
         font.pointSize: units.gu(1)
         font.bold: true
         color: labelColor
      }

      Text {
         width: parent.width
         visible: fields.primaryFields.length
         text: fields.primaryFields.length && primaryFields[0].value || ""
         wrapMode: Text.WordWrap
         font.pointSize: fields.primaryFields.length === 1 ? units.gu(1.5) : units.gu(2)
         color: foregroundColor
      }
   }

   Image {
      id: airplaneImage
      anchors.centerIn: parent
      width: units.gu(5)
      height: units.gu(5)
      source: "qrc:///airplane-flying.png"
      visible: fields.style === "boardingPass"
   }

   ColorOverlay {
       anchors.fill: airplaneImage
       source: airplaneImage
       color: fields.foregroundColor
       visible: fields.style === "boardingPass"
   }

   Image {
      anchors.right: parent.right
      width: Math.max(f.height, units.gu(5))
      height: Math.max(f.height, units.gu(5))
      source: "image://passes/" + passId + "/thumbnail"
      visible: fields.style !== "boardingPass" && fields.primaryFields.length < 2
   }

   Column {
      visible: fields.primaryFields.length === 2
      anchors.right: parent.right
      width: fields.primaryFields.length === 1 ? parent.width : parent.width / 2

      height: childrenRect.height

      Text {
         width: parent.width
         visible: fields.primaryFields.length >= 2
         text: (fields.primaryFields.length >= 2 && primaryFields[1].label || "").toUpperCase()
         horizontalAlignment: Text.AlignRight
         wrapMode: Text.WordWrap
         font.pointSize: units.gu(1)
         font.bold: true
         color: labelColor
      }

      Text {
         width: parent.width
         visible: fields.primaryFields.length >= 2
         text: fields.primaryFields.length >= 2 && primaryFields[1].value || ""
         horizontalAlignment: Text.AlignRight
         wrapMode: Text.WordWrap
         font.pointSize: units.gu(2)
         color: foregroundColor
      }
   }
}
