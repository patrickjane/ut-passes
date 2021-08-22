import QtQuick 2.0

Column {
   property var field
   property color foregroundColor
   property color labelColor

   anchors.verticalCenter: parent.verticalCenter
   height: childrenRect.height

   Text {
      anchors.right: parent.right
      text: field.label.toUpperCase()
      font.pointSize: units.gu(0.8)
      color: labelColor
   }

   Text {
      anchors.right: parent.right
      text: field.value
      font.pointSize: units.gu(0.8)
      color: foregroundColor
   }
}
