import QtQuick 2.5
import QtQuick.Controls 2.11
import Lomiri.Components 1.3

Item {
   id: view
   property var pass
   property double cardHeight: height*0.85
   property double cardWidth: width*0.85
   property var theModel: pass ? (pass.bundlePasses.length ? pass.bundlePasses : [ pass ]) : []

   ListView {
      id: listView
      anchors.centerIn: parent
      width: parent.width
      height: view.cardHeight

      spacing: view.width * 0.15 / 2
      snapMode: ListView.SnapOneItem
      orientation: ListView.Horizontal
      highlightRangeMode: ListView.StrictlyEnforceRange
      model: theModel
      interactive: theModel ? theModel.length > 1 : false

      delegate: Item {
         height: view.cardHeight
         width: listView.width

         Card {
            id: card

            anchors.centerIn: parent
            width: view.cardWidth
            height: view.cardHeight
            pass: listView.model[index]
            selected: true
         }
      }
   }

   PageIndicator {
      id: indicator
      anchors.top: listView.bottom
      anchors.topMargin: units.gu(2)
      anchors.horizontalCenter: parent.horizontalCenter
      visible: theModel ? theModel.length > 1 : false

      currentIndex: listView.currentIndex
      count: listView.count

      delegate: Rectangle {
         implicitWidth: units.gu(1)
         implicitHeight: units.gu(1)

         radius: width / 2
         color: "#323232"

         opacity: index === indicator.currentIndex ? 0.95 : pressed ? 0.7 : 0.45

         Behavior on opacity {
            OpacityAnimator {
                duration: 100
            }
         }
      }
   }
}