import QtQuick 2.4

Rectangle {
   id: view
   property var model
   property var selectedCard
   property var cards: []
   property double cardHeight: height*0.85
   property double cardWidth: width*0.85
   property double topMargin: (height-cardHeight)/2

   color: "#dedede"

   states: [
      State {
         name: "noCardShown"
         PropertyChanges { target: view; color: "#dedede"  }
      },
      State {
         name: "cardShown"
         PropertyChanges { target: view; color: "white" }
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

   Flickable {
      id: flickable
      anchors.fill: parent
      contentHeight: childrenRect.height
      contentWidth: parent.width

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
            anchors.topMargin: view.topMargin + index*units.gu(8)

            pass: modelData

            onCardFrontClicked: showCard(index)
         }

         onItemAdded: {
            item.index = index
            cards.push(item)
         }
      }
   }

   function showCard(index) {
      view.state = "cardShown"

      view.cards.forEach(function(card) {
         if (card.index === index) {
            card.anchors.topMargin = view.topMargin
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
      view.selectedCard.unflip()

      view.cards.forEach(function(card) {
         card.selected = false
         card.visible = true
         card.anchors.topMargin = view.topMargin + card.index*units.gu(8)
      })

      view.selectedCard = undefined
   }
}
