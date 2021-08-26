import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Content 1.3

Page {
   id: picker
   property var activeTransfer
   property var url

   signal imported(string fileUrl)

   header: PageHeader {
      title: i18n.tr("Import pass")
   }

   ContentPeerPicker {
      anchors.fill: parent
      anchors.topMargin: picker.header.height

      showTitle: false
      contentType: ContentType.Documents
      handler: ContentHandler.Source

      onPeerSelected: {
         peer.selectionType = ContentTransfer.Single
         picker.activeTransfer = peer.request()
         picker.activeTransfer.stateChanged.connect(function() {
            if (!picker.activeTransfer)
               return

            if (picker.activeTransfer.state === ContentTransfer.InProgress) {
               picker.activeTransfer.items = picker.activeTransfer.items[0].url = url;
               picker.activeTransfer.state = ContentTransfer.Charged;
            }
            if (picker.activeTransfer.state === ContentTransfer.Charged) {
               picker.imported(picker.activeTransfer.items[0].url)
               picker.activeTransfer = null
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
}
