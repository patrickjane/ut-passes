import QtQuick 2.0

Item {
   property string type: "header"
   property var field
   property color foregroundColor
   property color labelColor

   height: type === "header" ? parent.height : f.height
   width: childrenRect.width

   Column {
      id: f
      anchors.verticalCenter: parent.verticalCenter
      height: childrenRect.height

      Text {
         anchors.right: parent.right
         text: field.label.toUpperCase()
         font.pointSize: units.gu(0.8)
         visible: type === "header"
         color: labelColor
      }

      Text {
         anchors.right: parent.right
         text: field.value
         font.pointSize: units.gu(0.8)
         visible: type === "header"
         color: foregroundColor
      }

      Text {
         anchors.left: parent.left
         text: field.label.toUpperCase()
         font.pointSize: units.gu(1)
         visible: type !== "header"
         color: labelColor
      }

      Text {
         anchors.left: parent.left
         text: field.value
         font.pointSize: units.gu(1.5)
         visible: type !== "header"
         color: foregroundColor
      }
   }
}
