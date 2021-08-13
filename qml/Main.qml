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

   width: units.gu(45)
   height: units.gu(75)

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



//MainView {
//   id: root
//   objectName: 'mainView'
//   applicationName: 'passes.s710'
//   automaticOrientation: true

//   width: units.gu(45)
//   height: units.gu(75)

//   Notification {
//      notificationId: "mapPageNotification"
//   }

//   PassesModel {
//      id: passesModel
//   }

//   Page {
//      anchors.fill: parent

//      header: PageHeader {
//         id: header
//         title: i18n.tr('Passes')
//      }

//      Component.onCompleted: {

//      }



//      ColumnLayout {
//         spacing: units.gu(2)
//         anchors {
//            margins: units.gu(2)
//            top: header.bottom
//            left: parent.left
//            right: parent.right
//            bottom: parent.bottom
//         }

//         Item {
//            Layout.fillHeight: true
//         }

//         Label {
//            id: label
//            Layout.alignment: Qt.AlignHCenter
//            text: i18n.tr('Press the button below and check the logs!')
//         }

//         Button {
//            Layout.alignment: Qt.AlignHCenter
//            text: i18n.tr('Press here!')
//            onClicked: Example.speak()
//         }

//         Item {
//            Layout.fillHeight: true
//         }
//      }


//   }
//}
