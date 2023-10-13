import QtQuick 2.7
import Lomiri.Components 1.3
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1
import Lomiri.Content 1.1
import Lomiri.Components.Popups 1.3

import "./notify"
import "./util"
import "./pages"

import PassesModel 1.0

MainView {
   id: root
   objectName: 'mainView'
   applicationName: 'passes.s710'
   automaticOrientation: false

   property string initError: ""
   property var failedPasses: undefined

   // width: units.gu(45)
   // height: units.gu(75)

   Settings {
      id: settings
      property bool updateAtStartup: true
   }

   Notification {
      notificationId: "mainNotification"
   }

   Text { id: text; font.pointSize: units.gu(1) }

   Timer {
      id: initTimer
      interval: 200
      repeat: false
      running: false

      onTriggered: {
         if (root.initError) {
            Dialogs.showErrorDialog(root,
                                    i18n.tr("Failed to init storage directory"),
                                    i18n.tr("Storage directory could not be initialized (%1).").arg(root.initError))

            return
         }

         if (root.failedPasses && root.failedPasses.length) {
            root.failedPasses.forEach(function(pass) {
               var popup = Dialogs.showQuestionDialog(root,
                                                      i18n.tr("Failed to open pass"),
                                                      i18n.tr("Pass '%1' could not be opened (%2). Do you want to delete the pass from storage? This operation cannot be undone.")
                                                      .arg(pass.filePath)
                                                      .arg(pass.error),
                                                      i18n.tr("Delete"),
                                                      i18n.tr("Cancel"),
                                                      LomiriColors.red)

               popup.accepted.connect(function() {
                  var err = passesModel.deletePass(pass.filePath, true)

                  if (err) {
                     var comps = (pass.filePath || "").split("/")
                     var fileName = comps.length && comps[comps.length-1]

                     Dialogs.showErrorDialog(mainPage,
                                               i18n.tr("Failed to delete pass"),
                                               i18n.tr("Pass '%1' could not be deleted (%2).")
                                               .arg(fileName)
                                               .arg(err))
                  }
               })
            })

            return
         }

         // everything went well. fetch pass updates, if configured

         if (settings.updateAtStartup)
            passesModel.fetchPassUpdates()
      }
   }

   PassesModel {
      id: passesModel

      defaultFont: text.font

      onFailedPasses: {
         root.failedPasses = passes
      }
   }

   Connections {
      target: ContentHub

      onImportRequested: {
         var filePath = String(transfer.items[0].url).replace('file://', '')
         var fileName = filePath.split("/").pop();
         var popup = Dialogs.showQuestionDialog(root,
                                                i18n.tr("Add pass"),
                                                i18n.tr("Do you want to add '%1' to Passes?").arg(fileName),
                                                i18n.tr("Add"),
                                                i18n.tr("Cancel"),
                                                LomiriColors.green)

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

         root.initError = passesModel.init()

         if (!root.initError)
            passesModel.reload()

         initTimer.start()
      }

      MainPage {
         id: mainPage
         passesModel: passesModel
      }
   }
}
