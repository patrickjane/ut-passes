import QtQuick 2.7
import Lomiri.Components 1.3
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1
import Lomiri.Content 1.1

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
            iconName: "close"
            visible: !!passesView.selectedPass
            onTriggered: passesView.dismissCard()
         },
         Action {
            iconName: "import"
            visible: !passesView.selectedPass
            onTriggered: {
               var importPage = pageStack.push(Qt.resolvedUrl("ImportPage.qml"), {})
               importPage.imported.connect(importUrls)
            }
         },
         Action {
            iconName: "view-refresh"
            visible: !passesView.selectedPass
            enabled: passesModel.count > 0 && !passesView.selectedPass

            onTriggered: {
               var popup = Dialogs.showQuestionDialog(mainPage,
                                          i18n.tr("Fetch pass updates"),
                                          i18n.tr("Do you want to search for pass updates?"),
                                          i18n.tr("Fetch updates"),
                                          i18n.tr("Cancel"),
                                          LomiriColors.green)

               popup.accepted.connect(function() {
                  passesView.showActivity = true
                  passesModel.fetchPassUpdates()
               })
            }
         },
         Action {
            iconName: "settings"
            visible: !passesView.selectedPass
            onTriggered: {
               var settingsPage = pageStack.push(Qt.resolvedUrl("SettingsPage.qml"), {})

               settingsPage.updateIntervalChanged.connect(function(interval, enabled) {
                  fetchUpdatesTimer.running = enabled
                  fetchUpdatesTimer.interval = interval * 60000

                  if (enabled)
                     fetchUpdatesTimer.restart()
               })
            }
         },
         Action {
            iconName: "share"
            visible: !!passesView.selectedPass
            onTriggered: {
               if (passesView.selectedPass.bundleName.length) {
                  var res = passesModel.createExportBundle(passesView.selectedPass.id)

                  if (res.error) {
                     Dialogs.showErrorDialog(mainPage,
                           i18n.tr("Failed to export pass bundle"),
                           i18n.tr("Pass bundle could not be exported (%1).")
                           .arg(res.error))
                  } else if (res.filePath) {
                     var sharePage = Qt.resolvedUrl("SharePage.qml")
                     pageStack.push(sharePage, { url: "file://" + res.filePath, model: passesModel, cleanupFile: res.filePath })
                  }
               } else {
                  var passFile = passesView.selectedPass.filePath

                  if (passFile)
                     pageStack.push(Qt.resolvedUrl("SharePage.qml"), { url: "file://" + passFile })
               }
            }
         },
         Action {
            iconName: "delete"
            visible: !!passesView.selectedPass
            onTriggered: {
               var popup = Dialogs.showQuestionDialog(root,
                                                      i18n.tr("Delete pass"),
                                                      i18n.tr("Do you want to delete the pass from Passes? This operation cannot be undone."),
                                                      i18n.tr("Delete"),
                                                      i18n.tr("Cancel"),
                                                      LomiriColors.red)

               popup.accepted.connect(function() {
                  var passFile = passesView.selectedPass.filePath;
                  var err = passesModel.deletePass(passesView.selectedPass.id)
                  passesView.dismissCard()

                  if (err) {
                     var comps = (passFile || "").split("/")
                     var fileName = comps.length && comps[comps.length-1]

                     Dialogs.showErrorDialog(mainPage,
                                               i18n.tr("Failed to delete pass"),
                                               i18n.tr("Pass '%1' could not be deleted (%2).")
                                               .arg(fileName)
                                               .arg(err))
                  }
               })
            }
         }
      ]
   }

   Connections {
      target: passesModel
      onPassUpdatesFetched: {
         passesView.showActivity = false
      }
   }

   Settings {
      id: settings
      property bool updateAtInterval: true
      property double updateInterval: 15
   }

   Timer {
      id: fetchUpdatesTimer
      interval: settings.updateInterval * 60000
      repeat: true
      running: settings.updateAtInterval

      onTriggered: {
         passesModel.fetchPassUpdates()
      }
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
         width: parent.width
         spacing: units.gu(2)

         Text {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width*0.85
            wrapMode: Text.WordWrap
            text: passesModel.countExpired > 0
                  ? i18n.tr("All passes have expired")
                  : i18n.tr("No passes have been added yet")
         }

         Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: i18n.tr("Add new pass")
            onClicked: {
               var importPage = pageStack.push(Qt.resolvedUrl("ImportPage.qml"), {})
               importPage.imported.connect(importUrls)
            }
         }
      }
   }

   function importUrls(urls) {
      urls.forEach(function(fileUrl) {
         var err = passesModel.importPass(fileUrl, passesView.showExpiredPasses)

         if (err) {
            var comps = ((fileUrl || "") + '').split("/")
            var fileName = comps.length && comps[comps.length-1]

            Dialogs.showErrorDialog(mainPage,
                                      i18n.tr("Failed to import pass"),
                                      i18n.tr("Pass '%1' could not be imported (%2).")
                                      .arg(fileName)
                                      .arg(err))
         }
      })
   }
}
