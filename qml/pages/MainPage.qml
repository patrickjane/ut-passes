import QtQuick 2.7
import Ubuntu.Components 1.3
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1
import Ubuntu.Content 1.1

import "../notify"
import "../pass"
import "../util"

import PassesModel 1.0

Page {
   id: mainPage
   property var passesModel

   anchors.fill: parent

   header: PageHeader {
      id: header
      title: i18n.tr('Passes')

      trailingActionBar.actions: [
         Action {
            iconName: "import"
            visible: !passesView.selectedCard
            onTriggered: {
               var importPage = pageStack.push(Qt.resolvedUrl("ImportPage.qml"), {})
               importPage.imported.connect(passesModel.importPass)
            }
         },
         Action {
            iconName: "close"
            visible: !!passesView.selectedCard
            onTriggered: passesView.dismissCard()
         },
         Action {
            iconName: "share"
            visible: !!passesView.selectedCard
            onTriggered: {
               var passFile = passesView.selectedCard.pass.filePath

               if (passFile)
                  pageStack.push(Qt.resolvedUrl("SharePage.qml"), { url: "file://" + passFile })
               else
                  console.log("No filepath for pass", passesView.selectedCard.pass.id)
            }
         },
         Action {
            iconName: "delete"
            visible: !!passesView.selectedCard
            onTriggered: {
               var popup = Dialogs.deleteDialog(root)

               popup.accepted.connect(function() {
                  var passID =  passesView.selectedCard.pass.id
                  passesView.dismissCard()
                  passesModel.deletePass(passID)
               })
            }
         }
      ]
   }

   PassesView {
      id: passesView
      anchors.top: header.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.bottom: parent.bottom

      model: passesModel
   }

   Rectangle {
      id: placeholder
      radius: units.gu(4)
      border.width: 2
      border.color: "gray"
      visible: !passesModel.count

      anchors.centerIn: parent
      width: parent.width * 0.6
      height: parent.height * 0.6

      Column {
         anchors.centerIn: parent
         spacing: units.gu(2)

         Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: i18n.tr("No passes have been added yet")
         }

         Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Add new pass"
            onClicked: {
               var importPage = pageStack.push(Qt.resolvedUrl("ImportPage.qml"), {})
               importPage.imported.connect(passesModel.importPass)
            }
         }
      }
   }
}
