import QtQuick 2.7
import Ubuntu.Components 1.3
//import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1
import Ubuntu.Content 1.1
import Ubuntu.Components.Popups 1.3

import "./notify"

import PassesModel 1.0

MainView {
   id: root
   objectName: 'mainView'
   applicationName: 'passes.s710'
   automaticOrientation: false

//   width: units.gu(45)
//   height: units.gu(75)

   width: units.gu(100)
   height: units.gu(200)

   Notification {
      notificationId: "mainNotification"
   }

   PassesModel {
      id: passesModel

      onError: Notify.error(i18n.tr("Error"), error)
      onPassError: Notify.error(i18n.tr("Pass Error"), error)
   }

   Connections {
      target: ContentHub

      onImportRequested: {
         var filePath = String(transfer.items[0].url).replace('file://', '')
         var fileName = filePath.split("/").pop();
         var popup = PopupUtils.open(addQuestion, root, {fileName: fileName});

         popup.accepted.connect(function() {
            passesModel.importPass(filePath)
         })
      }
   }

   Component {
      id: addQuestion

      Dialog {
         id: addQuestionDialog
         title: i18n.tr("Add pass")
         text: i18n.tr("Do you want to add '%1' to Passes?").arg(fileName)

         property string fileName
         signal accepted();
         signal rejected();

         Button {
            text: i18n.tr("Add")
            color: UbuntuColors.green
            onClicked: {
               addQuestionDialog.accepted()
               PopupUtils.close(addQuestionDialog)
            }
         }
         Button {
            text: i18n.tr("Cancel")
            onClicked: {
               addQuestionDialog.rejected()
               PopupUtils.close(addQuestionDialog)
            }
         }
      }
   }

   PageStack {
      id: pageStack
      anchors {
         fill: parent
      }
   }

   Component.onCompleted: {
      passesModel.reload()
      pageStack.push(Qt.resolvedUrl("pages/MainPage.qml"), { passesModel: passesModel })
   }
}
