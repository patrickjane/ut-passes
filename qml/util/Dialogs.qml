pragma Singleton

import QtQuick 2.4
import Lomiri.Components.Popups 1.3
import Lomiri.Components 1.3

Item {
   Component {
      id: questionDialogComponent

      Dialog {
         id: questionDialog
         property string acceptButtonTitle: i18n.tr("Okay")
         property string cancelButtonTitle: i18n.tr("Cancel")
         property color acceptButtonColor: LomiriColors.green

         signal accepted();
         signal rejected();

         Button {
            text: acceptButtonTitle
            color: acceptButtonColor
            onClicked: {
               questionDialog.accepted()
               PopupUtils.close(questionDialog)
            }
         }
         Button {
            text: cancelButtonTitle
            onClicked: {
               questionDialog.rejected()
               PopupUtils.close(questionDialog)
            }
         }
      }
   }

   Component {
      id: errorDialogComponent

      Dialog {
         id: errorDialog

         Button {
            text: i18n.tr("Close")
            onClicked: {
               PopupUtils.close(errorDialog)
            }
         }
      }
   }

   Component {
      id: storageErrorDialogComponent

      Dialog {
         id: storageErrorDialog
         title: i18n.tr("Failed to init storage directory")
         text: i18n.tr("Storage directory could not be initialized (%1).").arg(errorString)

         property string errorString

         Button {
            text: i18n.tr("Close")
            onClicked: {
               PopupUtils.close(storageErrorDialog)
            }
         }
      }
   }

   function showQuestionDialog(parent, title, text, acceptButtonTitle, cancelButtonTitle, acceptButtonColor) {
      return PopupUtils.open(questionDialogComponent, parent, {
                                title: title,
                                text: text,
                                acceptButtonTitle: acceptButtonTitle,
                                cancelButtonTitle: cancelButtonTitle,
                                acceptButtonColor: acceptButtonColor
                             });
   }

   function showErrorDialog(parent, title, text) {
      return PopupUtils.open(errorDialogComponent, parent, { title: title, text: text });
   }
}
