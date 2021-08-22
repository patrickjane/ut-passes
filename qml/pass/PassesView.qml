import QtQuick 2.0

Rectangle {
   id: view
   property var model
   property var selectedCard
   property var cards: []

   color: !!selectedCard ? "white" : "#dedede"

   Connections {
      target: model
      onModelAboutToBeReset: cards = []
      onRowsAboutToBeRemoved: cards.splice(first, 1)
   }

   Repeater {
      id: repeater
      model: view.model

      Card {
         id: card
         property int index
         property var topAnchor

         width: parent.width * 0.9
         height: parent.height * 0.7

         anchors.horizontalCenter: parent.horizontalCenter
         anchors.top: parent.top
         anchors.topMargin: units.gu(8)

         pass: modelData

         onCardFrontClicked: showCard(index)
       }

      onItemAdded: {
         console.log("Item added: ", index, "num cards now:", cards.length)

         var lastCard = view.cards.length && view.cards[view.cards.length - 1]

         if (lastCard) {
            item.anchors.top = lastCard.top
            item.topAnchor = lastCard.top
         }

         item.index = index
         cards.push(item)
      }
   }

   function showCard(index) {
      view.cards.forEach(function(card) {
         if (card.index === index) {
            card.anchors.top = card.parent.top
            card.selected = true
            view.selectedCard = card
         } else
            card.anchors.top = view.bottom
      })
   }

   function dismissCard() {
      if (!view.selectedCard)
         return

      view.selectedCard.unflip()

      var lastCard

      view.cards.forEach(function(card) {
         card.anchors.top = lastCard ? lastCard.top : card.parent.top
         card.selected = false
         lastCard = card
      })

      view.selectedCard = undefined
   }
}
