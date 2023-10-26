import QtQuick 2.4
import Lomiri.Components 1.3
import Lomiri.Content 1.3

Page {
   id: picker
   property var activeTransfer
   property string url
   property string cleanupFile: ""
   property var model

   header: PageHeader {
      title: i18n.tr("Share pass")
   }

   function cleanupFn() {
      if (cleanupFile != "") {
         model.deleteFile(url.replace("file://", ""))
      }
   }

   ContentPeerPicker {
      anchors.fill: parent
      anchors.topMargin: picker.header.height

      showTitle: false
      contentType: ContentType.All
      handler: ContentHandler.Share

      onPeerSelected: {
         peer.selectionType = ContentTransfer.Single
         picker.activeTransfer = peer.request()

         picker.activeTransfer.stateChanged.connect(function() {
            if (!picker || !picker.activeTransfer)
               return

            if (picker.activeTransfer.state === ContentTransfer.InProgress) {
               picker.activeTransfer.items = [ resultComponent.createObject(parent, {"url": url}) ];
               picker.activeTransfer.state = ContentTransfer.Charged;
               pageStack.pop()
            }
         })
      }

      onCancelPressed: {
         pageStack.pop()
      }
   }

   ContentTransferHint {
      id: transferHint
      anchors.fill: parent
      activeTransfer: picker.activeTransfer
   }

   Component {
      id: resultComponent
      ContentItem {}
   }

   Component.onDestruction: {
      cleanupFn()
   }
}
