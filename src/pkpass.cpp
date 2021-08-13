// **************************************************************************
// class PkPass
// 02.07.2021
// Model for handling locally stored passes managed by this app
// **************************************************************************
// MIT License
// Copyright © 2021 Patrick Fial
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
// files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software
// is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// **************************************************************************
// includes
// **************************************************************************

#include "pkpass.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "quazip/quazipfile.h"

namespace C {
#include <libintl.h>
}

namespace passes
{
   // **************************************************************************
   // class PkpassParser
   // **************************************************************************

   Pass* Pkpass::openPass(QString filePath)
   {
      Pass* pass = nullptr;
      currentTranslation.clear();

      qDebug() << "OPEN PASS: " << filePath;

      QuaZip archive(filePath);

      bool res = archive.open(QuaZip::mdUnzip);

      if (!res)
      {
         qDebug() << "Failed to open pass: " << archive.getZipError();

         emit error(QString(archive.getZipError()));
         return pass;
      }

      auto archiveContents = archive.getFileNameList();

      if (!archiveContents.contains(filePassJson))
      {
         qDebug() << "Failed to open pass: no pass.json found";

         emit error(C::gettext("Archive does not contain a valid pass"));
         return pass;
      }

      pass = new Pass();

      if (res) res = readLocalization(pass, archive, archiveContents);

      qDebug() << "READ pass localization: " << res;

      res = readPass(pass, archive);

      qDebug() << "READ pass basic: " << res;

      if (res) res = readImages(pass, archive, archiveContents);

      qDebug() << "READ pass images: " << res;

      archive.close();
      return pass;
   }

   // **************************************************************************
   // readPassJson
   // **************************************************************************

   bool Pkpass::readPass(Pass* pass, QuaZip& archive)
   {
      archive.setCurrentFile(filePassJson);

      QuaZipFile file(&archive);

      file.open(QIODevice::ReadOnly);

      auto contents = file.readAll();
      auto doc = QJsonDocument::fromJson(contents);

      file.close();

      if (doc.isNull() || !doc.isObject() || doc.object().isEmpty())
      {
         emit error(C::gettext("Pass information is invalid (empty)"));
         return false;
      }

      auto root = doc.object();

      readPassStandard(pass, root);
      readPassStyle(pass, root);

      return true;
   }

   // **************************************************************************
   // readPassStandard
   // **************************************************************************

   bool Pkpass::readPassStandard(Pass* pass, QJsonObject& object)
   {
      if (!object.contains("description") || !object.contains("organizationName"))
      {
         emit error(C::gettext("Pass information is invalid (missing description/organization key(s))"));
         return false;
      }

      pass->standard.description = object["description"].toString();
      pass->standard.organization = object["organizationName"].toString();

      translate(pass->standard.description);
      translate(pass->standard.organization);

      if (object.contains("relevantDate"))
         pass->standard.relevantDate = object["relevantDate"].toString();

      if (object.contains("expirationDate"))
         pass->standard.expirationDate = object["expirationDate"].toString();

      if (object.contains("voided"))
         pass->standard.voided = object["voided"].toBool();

      if (object.contains("backgroundColor"))
         pass->standard.backgroundColor = parseColor(object["backgroundColor"].toString());

      if (object.contains("foregroundColor"))
         pass->standard.foregroundColor = parseColor(object["foregroundColor"].toString());

      if (object.contains("labelColor"))
         pass->standard.labelColor = parseColor(object["labelColor"].toString());

      if (object.contains("logoText"))
      {
         pass->standard.logoText = object["logoText"].toString();
         translate(pass->standard.logoText);
      }

      if (object.contains("barcode"))
         readPassBarcode(pass, object["barcode"].toObject());

      if (object.contains("barcodes"))
      {
         auto barcodes = object["barcodes"].toArray();

         for (auto barcode : barcodes)
            readPassBarcode(pass, barcode.toObject());
      }
   }

   // **************************************************************************
   // readPassBarcode
   // **************************************************************************

   bool Pkpass::readPassBarcode(Pass* pass, QJsonObject object)
   {
      if (object.isEmpty() || !object.contains("format") || !object.contains("message"))
         return false;

      QString format = object["format"].toString();
      QString message = object["message"].toString();
      QString encoding = object.contains("encoding") ? object["encoding"].toString() : QString();
      QString altText = object.contains("altText") ? object["altText"].toString() : QString();

      pass->standard.barcodes.push_back(Barcode{ format, message, encoding, altText });
   }

   // **************************************************************************
   // readPassStyle
   // **************************************************************************

   bool Pkpass::readPassStyle(Pass* pass, QJsonObject object)
   {
      QJsonObject styleObject;

      if (object.contains("boardingPass"))
      {
         styleObject = object["boardingPass"].toObject();
         pass->details.style = "boardingPass";
      }
      else if (object.contains("coupon"))
      {
         styleObject = object["coupon"].toObject();
         pass->details.style = "coupon";
      }
      else if (object.contains("eventTicket"))
      {
         styleObject = object["eventTicket"].toObject();
         pass->details.style = "eventTicket";
      }
      else if (object.contains("generic"))
      {
         styleObject = object["generic"].toObject();
         pass->details.style = "generic";
      }
      else if (object.contains("storeCard"))
      {
         styleObject = object["storeCard"].toObject();
         pass->details.style = "storeCard";
      }

      if (styleObject.isEmpty())
         return false;

      if (styleObject.contains("headerFields"))
         readPassStyleFields(pass->details.headerFields, styleObject["headerFields"].toArray());
      if (styleObject.contains("primaryFields"))
         readPassStyleFields(pass->details.primaryFields, styleObject["primaryFields"].toArray());
      if (styleObject.contains("secondaryFields"))
         readPassStyleFields(pass->details.secondaryFields, styleObject["secondaryFields"].toArray());
      if (styleObject.contains("auxiliaryFields"))
         readPassStyleFields(pass->details.auxiliaryFields, styleObject["auxiliaryFields"].toArray());
      if (styleObject.contains("backFields"))
         readPassStyleFields(pass->details.backFields, styleObject["backFields"].toArray());
   }

   // **************************************************************************
   // readPassStyleFields
   // **************************************************************************

   bool Pkpass::readPassStyleFields(QList<PassStyleField>& fields, QJsonArray jsonFields)
   {
      if (jsonFields.isEmpty())
         return false;

      for (auto v : jsonFields)
      {
         auto object = v.toObject();

         if (object.isEmpty())
            continue;

         QString key = object.contains("key") ? object["key"].toString() : QString();
         QString value = object.contains("value") ? object["value"].toString() : QString();
         QString label = object.contains("label") ? object["label"].toString() : QString();

         if (object.contains("dateStyle") || object.contains("timeStyle"))
         {
            QDateTime dt = QDateTime::fromString(value, Qt::ISODate);

            if (dt.isValid())
            {
               QString format;

               if (object.contains("dateStyle") && object.contains("timeStyle")
                   && object["dateStyle"] != "PKDateStyleNone"
                   && object["timeStyle"] != "PKDateStyleNone")
               {

                  if (object["dateStyle"] == "PKDateStyleShort")
                     format = QLocale::system().dateTimeFormat(QLocale::ShortFormat);
                  else
                     format = QLocale::system().dateTimeFormat(QLocale::LongFormat);
               }
               else if (object.contains("timeStyle") && object["timeStyle"] != "PKDateStyleNone")
               {
                  if (object["timeStyle"] == "PKDateStyleShort")
                     format = QLocale::system().timeFormat(QLocale::ShortFormat);
                  else
                     format = QLocale::system().timeFormat(QLocale::LongFormat);
               }
               else if (object.contains("dateStyle") && object["dateStyle"] != "PKDateStyleNone")
               {
                  if (object["dateStyle"] == "PKDateStyleShort")
                     format = QLocale::system().dateFormat(QLocale::ShortFormat);
                  else
                     format = QLocale::system().dateFormat(QLocale::LongFormat);
               }

               if (!format.isEmpty())
                  value = dt.toString(format);
            }
         }

         fields << PassStyleField{ translate(key), translate(value), translate(label) };
      }
   }

   // **************************************************************************
   // readImages
   // **************************************************************************

   bool Pkpass::readImages(Pass* pass, QuaZip& archive, const QStringList& archiveContents)
   {
      bool res = true;

      if (res) res = readImage(&pass->imgBackground, archive, archiveContents, "background@2x.png", "background.png");
      if (res) res = readImage(&pass->imgFooter,     archive, archiveContents, "footer@2x.png",     "footer.png");
      if (res) res = readImage(&pass->imgIcon,       archive, archiveContents, "icon@2x.png",       "icon.png");
      if (res) res = readImage(&pass->imgLogo,       archive, archiveContents, "logo@2x.png",       "logo.png");
      if (res) res = readImage(&pass->imgStrip,      archive, archiveContents, "strip@2x.png",      "strip.png");
      if (res) res = readImage(&pass->imgThumbnail,  archive, archiveContents, "thumbnail@2x.png",  "thumbnail.png");

      return res;
   }

   // **************************************************************************
   // readImage
   // **************************************************************************

   bool Pkpass::readImage(QImage* dest, QuaZip& archive, const QStringList& archiveContents, QString fileName2x, QString fileName)
   {
      QString fn = archiveContents.contains(fileName2x) ? fileName2x : fileName;

      if (!archiveContents.contains(fn))
        return true;

      archive.setCurrentFile(fn);

      QuaZipFile file(&archive);

      file.open(QIODevice::ReadOnly);

      bool res = dest->loadFromData(file.readAll());

      file.close();

      return res;
   }

   // **************************************************************************
   // readLocalization
   // **************************************************************************

   bool Pkpass::readLocalization(Pass* pass, QuaZip& archive, const QStringList& archiveContents)
   {
      QString localizationLocale = QLocale::system().name().mid(0, 2) + ".lproj/pass.strings";
      QString localizationEnglish = "en.lproj/pass.strings";

      currentTranslation.clear();

      if (archiveContents.contains(localizationLocale))
         return readLocalization(pass, archive, localizationLocale);
      else if (archiveContents.contains(localizationEnglish))
         return readLocalization(pass, archive, localizationEnglish);

      return true;
   }

   // **************************************************************************
   // readLocalization
   // **************************************************************************

   bool Pkpass::readLocalization(Pass* pass, QuaZip& archive, const QString& localization)
   {
      archive.setCurrentFile(localization);

      QuaZipFile file(&archive);

      file.open(QIODevice::ReadOnly);

      QByteArray line;

      do
      {
         line = file.readLine();

         QString lineString = QString::fromUtf8(line);
         lineString.replace("\n", "").chop(2);
         lineString.remove(0, 1);

         if (lineString.isEmpty())
            continue;

         auto comps = lineString.split("\" = \"");

         if (comps.size() == 2)
            currentTranslation[comps[0]] = comps[1];
      } while (line.size());


      file.close();

      return true;
   }

   // **************************************************************************
   // readImages
   // **************************************************************************

   const QString& Pkpass::translate(QString& other)
   {
      if (!currentTranslation.contains(other))
         return other;

      return currentTranslation[other];
   }

   // **************************************************************************
   // parseColor
   // **************************************************************************

   QString Pkpass::parseColor(QString rgbString)
   {
      if (!rgbString.startsWith("rgb(") || !rgbString.endsWith(")"))
         return "";

      QStringList comps = rgbString.replace("rgb(", "").replace(")", "").split(",");

      if (comps.size() < 3)
         return "";

      return QColor(comps[0].toInt(), comps[1].toInt(), comps[2].toInt()).name();
   }

} // namespace passes
