import QtQuick 2.0

Column {
   property var field
   property color foregroundColor
   property color labelColor

   Text {
      anchors.left: parent.left
      text: field.label.toUpperCase()
      width: parent.width
      font.pointSize: units.gu(1)
      color: labelColor
      wrapMode: Text.WordWrap
   }

   Text {
      anchors.left: parent.left
      text: field.value
      width: parent.width
      font.pointSize: units.gu(1.5)
      color: foregroundColor
      wrapMode: Text.WordWrap
   }
}
