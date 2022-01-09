import QtQuick 2.0
import QtGraphicalEffects 1.10

Flipable {
   id: flipable
   property var pass
   property bool selected: false
   property bool flipped: false
   signal cardFrontClicked()
   signal cardBackClicked()

   front: CardFront {
      id: frontCard
      anchors.fill: parent
      pass: flipable.pass
      selected: flipable.selected

      onInfoButtonPressed: flipable.flipped = !flipable.flipped
      onCardClicked: { emit: cardFrontClicked() }
   }

   back: CardBack {
      id: backCard
      anchors.fill: parent
      pass: flipable.pass
      onBackButtonPressed: flipable.flipped = !flipable.flipped
      onCardClicked:  { emit: cardBackClicked() }
   }

//   DropShadow {
//      anchors.fill: frontCard
//      horizontalOffset: units.gu(1)
//      verticalOffset: units.gu(1)
//      radius: units.gu(1.5)
//      samples: 17
//      color: "#80000000"
//      source: frontCard
//      visible: flipable.selected && !flipable.flipped
//   }

//   DropShadow {
//      anchors.fill: backCard
//      horizontalOffset: units.gu(1)
//      verticalOffset: units.gu(1)
//      radius: units.gu(1.5)
//      samples: 17
//      color: "#80000000"
//      source: backCard
//      visible: flipable.selected && flipable.flipped
//   }

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
