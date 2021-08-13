import QtQuick 2.7
import Ubuntu.Components 1.3
//import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1
import Ubuntu.Content 1.1
import Ubuntu.Components.Popups 1.3

import "../notify"
import "../pass"

import PassesModel 1.0

Page {
   id: mainPage
   property var passesModel

   anchors.fill: parent

   header: PageHeader {
      id: header
      title: i18n.tr('Passes')
   }

   Rectangle {
      id: passCard
      property var pass: passesModel.passes.length && passesModel.passes[0] || undefined
//      property var pass: ({"details":{"auxiliaryFields":[{label: "Zimmerkategorie", value: "Vierbettenbude"}],"backFields":[],"headerFields":[{label: "2 Tage", value: "03.10.2020 - 05.10.2020"}],"primaryFields":[{"key": "checkin", "label": "Check-in", "value": "15:00 Uhr 03.10.2020" }],"secondaryFields":[{label: "Ihre Buchungsnummer", value: "GEORG-1ABB9B-6660"}, {label: "Buchende Person", value: "Patrick Fial"}],"style":"","transitType":""},"standard":{"backgroundColor":"rgb(226,196,73)","barcodes":[{"altText":"","encoding":"","format":"PKBarcodeFormatPDF417","message":"GEORG-1ABB9B-6660"}],"description":"OnePageBooking","expirationDate":"","foregroundColor":"rgb(0,0,0)","labelColor":"","logoText":"Superbude Hamburg ...","organization":"Superbude Hamburg St. Georg","relevantDate":"2020-10-02T02:18:34+02:00","voided":false}})
      property color foregroundColor: pass.standard.foregroundColor || "black"
      property color labelColor: pass.standard.labelColor || foregroundColor
      property color backgroundColor: pass.standard.backgroundColor || "white"
      radius: units.gu(4)

      color: backgroundColor
      border.width: 1
      border.color: Qt.rgba(backgroundColor.r * 0.6, backgroundColor.g * 0.6, backgroundColor.b * 0.6, backgroundColor.a * 0.6)

      width: parent.width * 0.9
      height: parent.height * 0.7
      anchors.centerIn: parent

      // header line (Logo text + Header fields)

      Rectangle {
         id: logoTextContainer
         anchors.left: parent.left
         anchors.leftMargin: units.gu(2)
         anchors.top: parent.top

         height: units.gu(7)
         width: parent.width - units.gu(2)

         color: "transparent"

         Text {
            id: logoText
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            font.pointSize: units.gu(1.5)
            text: passCard.pass &&
                  (passCard.pass.standard.logoText || "")
                  || ""
            color: passCard.foregroundColor
         }

         Image {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            height: units.gu(7)
            fillMode: Image.PreserveAspectFit
            source: "image://passes/" + passCard.pass.id + "/logo"
            visible: !logoText.text
         }
      }

      Row {
         id: headerFields
         anchors.right: parent.right
         anchors.rightMargin: units.gu(2)
         anchors.top: parent.top

         height: units.gu(7)
         width: childrenRect.width
         spacing: units.gu(1.5)

         Repeater {
            model: passCard.pass.details.headerFields.reverse()

            delegate: PassField {
               field: modelData
               type: "header"
               foregroundColor: passCard.foregroundColor
               labelColor: passCard.labelColor
            }
         }
      }

      Column {
         anchors.top: logoTextContainer.bottom
         anchors.topMargin: units.gu(4)
         anchors.left: parent.left
         anchors.leftMargin: units.gu(1)
         anchors.right: parent.right
         anchors.rightMargin: units.gu(2)
         anchors.bottom: parent.bottom
         anchors.bottomMargin: units.gu(4)
         spacing: units.gu(4)

         // primary fields

         PassPrimaryFields {
            anchors.left: parent.left
            anchors.right: parent.right

            passId: passCard.pass.id
            style: passCard.pass.details.style
            primaryFields: passCard.pass.details.primaryFields
            foregroundColor: passCard.foregroundColor
            labelColor: passCard.labelColor
         }

         Grid {
            anchors.left: parent.left
            anchors.right: parent.right
            height: childrenRect.height

            columns: passCard.pass.details.style === "boardingPass" ? 3 : 2
            spacing: units.gu(1.5)

            Repeater {
               model: passCard.pass.details.secondaryFields

               delegate: PassField {
                  field: modelData
                  type: "content"
                  foregroundColor: passCard.foregroundColor
                  labelColor: passCard.labelColor
               }
            }

            Repeater {
               model: passCard.pass.details.auxiliaryFields

               delegate: PassField {
                  field: modelData
                  type: "content"
                  foregroundColor: passCard.foregroundColor
                  labelColor: passCard.labelColor
               }
            }
         }
      }

      // primary fields


   }

}
