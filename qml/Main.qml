import QtQuick 2.7
import Ubuntu.Components 1.3
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1
import Ubuntu.Content 1.1
import Ubuntu.Components.Popups 1.3

import "./notify"
import "./util"
import "./pages"

import PassesModel 1.0

MainView {
   id: root
   objectName: 'mainView'
   applicationName: 'passes.s710'
   automaticOrientation: false

   width: units.gu(45)
   height: units.gu(75)

   Notification {
      notificationId: "mainNotification"
   }

   Text { id: text; font.pointSize: units.gu(1) }

   PassesModel {
      id: passesModel

      font: text.font

      onError: Notify.error(i18n.tr("Error"), error)

      onFailedPasses: {
         console.log(JSON.stringify(passes))

         passes.forEach(function(pass) {
            var popup = Dialogs.invalidPassDialog(root, pass.filePath, pass.error)

            popup.accepted.connect(function() {
               passesModel.deletePass(filePath)
            })
         })
      }
   }

   Connections {
      target: ContentHub

      onImportRequested: {
         var filePath = String(transfer.items[0].url).replace('file://', '')
         var fileName = filePath.split("/").pop();
         var popup = Dialogs.addDialog(root, fileName) // PopupUtils.open(addDialogComponent, root, {fileName: fileName});

         popup.accepted.connect(function() {
            passesModel.importPass(filePath)
         })
      }
   }

   PageStack {
      id: pageStack
      anchors {
         fill: parent
      }

      Component.onCompleted: {
         push(mainPage)

         passesModel.init()
         passesModel.reload()
      }

      MainPage {
         id: mainPage
         passesModel: passesModel
      }
   }
}
