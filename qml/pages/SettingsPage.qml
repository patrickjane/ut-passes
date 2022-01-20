import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3
import QtQuick 2.7
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0

Page {
    id: settingsPage
    anchors.fill: parent
    signal updateIntervalChanged(var interval, var enabled)

    Settings {
       id: settings
       property bool updateAtStartup: true
       property bool updateAtInterval: true
//       property double updateInterval
    }

    header: PageHeader {
       id: header
       title: i18n.tr("Settings")
    }

    Flickable {
       anchors.top: header.bottom
       anchors.left: parent.left
       anchors.right: parent.right
       anchors.bottom: parent.bottom

       contentWidth: parent.width
       contentHeight: childrenRect.height

       clip: true

       Column {
          anchors.left: parent.left
          anchors.right: parent.right

          ListItem {
             height: l1.height + (divider.visible ? divider.height : 0)

             ListItemLayout {
                id: l1
                title.text: i18n.tr("Updates")
                title.font.bold: true
             }
          }

          ListItem {
              anchors.left: parent.left
              anchors.right: parent.right
              height: l2.height + (divider.visible ? divider.height : 0)

              SlotsLayout {
                  id: l2
                  mainSlot: Text {
                     anchors.verticalCenter: parent.verticalCenter
                     text: i18n.tr("Fetch pass updates upon app start")
                  }
                  Switch {
                     checked: settings.updateAtStartup
                     SlotsLayout.position: SlotsLayout.Trailing

                     onClicked: {
                        settings.updateAtStartup = checked
                     }
                  }
              }
          }

          ListItem {
              anchors.left: parent.left
              anchors.right: parent.right
              height: l3.height + (divider.visible ? divider.height : 0)

              SlotsLayout {
                  id: l3
                  mainSlot: Text {
                     anchors.verticalCenter: parent.verticalCenter
                     text: i18n.tr("Fetch pass updates at regular intervals")
                  }
                  Switch {
                     id: switchUpdateAtInterval
                     checked: settings.updateAtInterval
                     SlotsLayout.position: SlotsLayout.Trailing

                     onClicked: {
                        settings.updateAtInterval = checked
                        emit: updateIntervalChanged(updateIntervalSlider.value || 15, checked)
                     }
                  }
              }
          }

          ListItem {
              anchors.left: parent.left
              anchors.right: parent.right
              height: l4.height + (divider.visible ? divider.height : 0)

              SlotsLayout {
                 id: l4
                 mainSlot: Column {
                    Slider {
                       id: updateIntervalSlider
                       anchors.left: parent.left
                       anchors.right: parent.right
                       anchors.rightMargin: units.gu(1)
                       enabled: switchUpdateAtInterval.checked

                       function formatValue(v) { return null }

                       minimumValue: 1.0
                       maximumValue: 60.0
                       stepSize: 1.0
                       value: 15.0
                       live: true

                       Settings {
                          property alias updateInterval: updateIntervalSlider.value
                       }

                       onValueChanged: {
                          emit: updateIntervalChanged(value || 15, settings.updateAtInterval)
                       }
                    }
                    Text {
                       text: i18n.tr(updateIntervalSlider.value === 1.0 ? "%1 minute" : "%1 minutes").arg(updateIntervalSlider.value.toFixed(0))
                       enabled: switchUpdateAtInterval.checked
                       color: switchUpdateAtInterval.checked && "black" || "gray"
                    }
                 }
              }
          }
       }
    }
}
