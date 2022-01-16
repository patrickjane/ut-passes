import QtQuick 2.5
import Ubuntu.Components 1.3

Rectangle {
   id: view
   property var model
   property var selectedCard
   property var cards: []
   property double cardHeight: height*0.85
   property double cardWidth: width*0.85
   property double topMargin: (height-cardHeight)/2
   property double cardPeekHeight: units.gu(8)
   property bool showExpiredPasses: false

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
      anchors.horizontalCenter: parent.horizontalCenter
      height: topMargin
      color: "transparent"

      Button {

         anchors.verticalCenter: parent.verticalCenter
         anchors.horizontalCenter: parent.horizontalCenter
         visible: model.countExpired > 0 && !view.selectedCard
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

   Flickable {
      id: flickable
      anchors.fill: parent
      anchors.topMargin: view.topMargin
      contentHeight: view.cardHeight
      contentWidth: parent.width
      interactive: !view.selectedCard

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

            pass: modelData

            onCardFrontClicked: showCard(index)
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

   function showCard(index) {
      view.state = "cardShown"

      view.cards.forEach(function(card) {
         if (card.index === index) {
            card.anchors.topMargin = flickable.contentY
            card.selected = true
            view.selectedCard = card
         } else {
            card.visible = false
         }
      })
   }

   function dismissCard() {
      if (!view.selectedCard || !view.cards.length)
         return

      view.state = "noCardShown"

      if (view.selectedCard.flipped) {
         view.selectedCard.unflip()
         dismissTimer.start()
      } else {
         view.cards.forEach(function(card) {
            card.selected = false
            card.visible = true
            card.anchors.topMargin = card.index*units.gu(8)
         })

         view.selectedCard = undefined
      }
   }

   function afterDismiss() {
      view.cards.forEach(function(card) {
         card.selected = false
         card.visible = true
         card.anchors.topMargin = card.index*units.gu(8)
      })

      view.selectedCard = undefined
   }
}
