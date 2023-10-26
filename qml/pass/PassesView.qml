import QtQuick 2.5
import Lomiri.Components 1.3

Rectangle {
   id: view
   property var model
   property var selectedPass
   property var cards: []
   property double cardHeight: height*0.85
   property double cardWidth: width*0.85
   property double topMargin: (height-cardHeight)/2
   property double cardPeekHeight: units.gu(8)
   property bool showExpiredPasses: false
   property bool showActivity: false

   color: "#efefef"

   states: [
      State {
         name: "noCardShown"
         PropertyChanges { target: view; color: "#efefef"  }
      },
      State {
         name: "cardShown"
         PropertyChanges { target: view; color: "#cdcdcd" }
      }
   ]

   transitions: [
      Transition {
         PropertyAnimation { properties: "color"; duration: 100 }
      }
   ]

   Connections {
      target: model
      onModelAboutToBeReset: cards = []
      onRowsAboutToBeRemoved: cards.splice(first, 1)
   }

   Rectangle {
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.right: parent.right
      height: topMargin
      color: "transparent"

      Rectangle {
         anchors.verticalCenter: parent.verticalCenter
         anchors.left: parent.left
         anchors.leftMargin: (parent.width - view.cardWidth) / 2
         color: "transparent"

         height: parent.height
         width: parent.height

         ActivityIndicator {
            id: activity
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            running: view.showActivity
         }
      }

      Button {
         anchors.verticalCenter: parent.verticalCenter
         anchors.horizontalCenter: parent.horizontalCenter
         visible: model.countExpired > 0 && !view.selectedPass
         text: view.showExpiredPasses
               ? i18n.tr(model.countExpired > 1 ? "Hide %1 expired passes" : "Hide %1 expired pass").arg(model.countExpired)
               : i18n.tr(model.countExpired > 1 ? "Show %1 expired passes" : "Show %1 expired pass").arg(model.countExpired)
         onClicked: {
            if (!view.showExpiredPasses)
               model.showExpired()
            else
               model.hideExpired()

            view.showExpiredPasses = !view.showExpiredPasses
         }
      }
   }

   Connections {
      target: model
      onCountChanged: {
         flickable.contentHeight = (view.cards.length-1) * cardPeekHeight + view.cardHeight + topMargin
      }
   }

   PassDetailView {
      id: detailView
      visible: !!view.selectedPass
      anchors.fill: parent

      pass: view.selectedPass
      cardHeight: view.cardHeight
      cardWidth: view.cardWidth
   }

   Flickable {
      id: flickable
      anchors.fill: parent
      anchors.topMargin: view.topMargin
      contentHeight: view.cardHeight
      contentWidth: parent.width
      visible: !view.selectedPass

      Repeater {
         id: repeater
         model: view.model

         Card {
            id: card
            property int index

            width: view.cardWidth
            height: view.cardHeight
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: index*view.cardPeekHeight

            pass: modelData.bundlePasses.length ? modelData.bundlePasses[0] : modelData

            onCardFrontClicked: showCard(index, modelData)
         }

         onItemAdded: {
            item.index = index
            cards.push(item)
         }
      }
   }

   Timer {
      id: dismissTimer
      interval: 600
      repeat: false
      running: false
      onTriggered: afterDismiss()
   }

   Rectangle {
      id: bottomStatusBar
      anchors.horizontalCenter: parent.horizontalCenter
      anchors.bottom: parent.bottom
      height: view.topMargin
      width: view.cardWidth
      color: "transparent"

      Text {
         id: cardDetailStatusText
         anchors.fill: parent

         visible: view.selectedPass && !!view.selectedPass.updateError || false
         text: view.selectedPass && view.selectedPass.updateError || ""

         horizontalAlignment: Text.AlignHCenter
         verticalAlignment: Text.AlignVCenter

         font.pointSize: units.gu(1)
         wrapMode: Text.WordWrap
      }
   }

   function showCard(index, pass) {
      view.state = "cardShown"
      view.selectedPass = pass
   }

   function dismissCard() {
      view.state = "noCardShown"
      view.selectedPass = null
   }

   function afterDismiss() {
      view.selectedPass = null
   }
}
