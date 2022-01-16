pragma Singleton

import QtQuick 2.4
import Ubuntu.Components.Popups 1.3
import Ubuntu.Components 1.3

Item {
   Component {
      id: deleteDialogComponent

      Dialog {
         id: deleteQuestionDialog
         title: i18n.tr("Delete pass")
         text: i18n.tr("Do you want to delete the pass from Passes? This operation cannot be undone.")

         signal accepted();
         signal rejected();

         Button {
            text: i18n.tr("Delete")
            color: UbuntuColors.red
            onClicked: {
               deleteQuestionDialog.accepted()
               PopupUtils.close(deleteQuestionDialog)
            }
         }
         Button {
            text: i18n.tr("Cancel")
            onClicked: {
               deleteQuestionDialog.rejected()
               PopupUtils.close(deleteQuestionDialog)
            }
         }
      }
   }

   Component {
      id: addDialogComponent

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

   Component {
      id: invalidPassDialogComponent

      Dialog {
         id: invalidPassDialog
         title: i18n.tr("Failed to open pass")
         text: i18n.tr("Pass '%1' could not be opened (%2). Do you want to delete the pass from storage? This operation cannot be undone.")
         .arg(fileName)
         .arg(errorString)

         property string fileName
         property string errorString
         signal accepted();
         signal rejected();

         Button {
            text: i18n.tr("Add")
            color: UbuntuColors.green
            onClicked: {
               invalidPassDialog.accepted()
               PopupUtils.close(invalidPassDialog)
            }
         }
         Button {
            text: i18n.tr("Cancel")
            onClicked: {
               invalidPassDialog.rejected()
               PopupUtils.close(invalidPassDialog)
            }
         }
      }
   }

   Component {
      id: errorDialogComponent

      Dialog {
         id: errorDialog
//         title: i18n.tr("Failed to import pass")
//         text: i18n.tr("Pass '%1' could not be imported (%2).")
//         .arg(fileName)
//         .arg(errorString)

//         property string fileName
//         property string errorString

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

   function deleteDialog(parent) {
      return PopupUtils.open(deleteDialogComponent, parent);
   }

   function addDialog(parent, fileName) {
      return PopupUtils.open(addDialogComponent, parent, { fileName: fileName });
   }

   function invalidPassDialog(parent, filePath, errorString) {
      var comps = (filePath || "").split("/")
      var fileName = comps.length && comps[comps.length-1]

      return PopupUtils.open(invalidPassDialogComponent, null, { fileName: fileName, errorString: errorString });
   }

   function storageErrorDialog(parent, errorString) {
      return PopupUtils.open(storageErrorDialogComponent, parent, { errorString: errorString });
   }

   function simpleErrorDialog(parent, title, text) {
      return PopupUtils.open(errorDialogComponent, parent, { title: title, text: text });
   }
}
