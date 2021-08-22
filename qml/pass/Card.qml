import QtQuick 2.0

Flipable {
   id: flipable
   property var pass
   property bool selected: false
   property bool flipped: false
   signal cardFrontClicked()
   signal cardBackClicked()

   front: CardFront {
      anchors.fill: parent
      pass: flipable.pass
      selected: flipable.selected

      onInfoButtonPressed: flipable.flipped = !flipable.flipped
      onCardClicked: { emit: cardFrontClicked() }
   }

   back: CardBack {
      anchors.fill: parent
      pass: flipable.pass
      onBackButtonPressed: flipable.flipped = !flipable.flipped
      onCardClicked:  { emit: cardFrontClicked() }
   }

   transform: Rotation {
       id: rotation
       origin.x: flipable.width/2
       origin.y: flipable.height/2
       axis.x: 0
       axis.y: 1
       axis.z: 0
       angle: 0
   }

   states: State {
       name: "back"
       PropertyChanges {
          target: rotation
          angle: 180
       }
       when: flipable.flipped
   }

   transitions: Transition {
       NumberAnimation {
          target: rotation
          property: "angle"
          duration: 500
       }
   }

   function unflip() {
      flipable.flipped = false
   }
}
