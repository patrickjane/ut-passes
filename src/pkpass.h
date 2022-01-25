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

#ifndef PKPASS_H
#define PKPASS_H

#include <QObject>
#include <QDateTime>
#include <QJsonObject>
#include <QImage>
#include <QJsonDocument>
#include <QRegExp>
#include <QFont>
#include <QFileInfo>

#include "quazip/quazip.h"

namespace passes
{
   static QString filePassJson = "pass.json";
   static QString fileBackgroundPng = "background.png";
   static QString fileFooterPng = "footer.png";
   static QString fileIconPng = "icon.png";
   static QString fileLogoPng = "logo.png";
   static QString fileStripPng = "strip.png";
   static QString fileThumbnailPng = "thumbnail.png";

   using Translation = QMap<QString,QString>;

   // **************************************************************************
   // struct PassItem
   // **************************************************************************

   struct Barcode
   {
         QString format;
         QString message;
         QString encoding;
         QString altText;
         QImage image;

         explicit operator QVariant() const
         {
            QVariantMap m;
            m.insert("format", format);
            m.insert("message", message);
            m.insert("encoding", encoding);
            m.insert("altText", altText);
            return m;
         }
   };

   struct WebService
   {
         QString accessToken;
         QString url;
         bool webserviceBroken;

         explicit operator QVariant() const
         {
            QVariantMap m;
            m.insert("url", url);
            m.insert("webserviceBroken", webserviceBroken);

            return m;
         }
   };

   struct Standard
   {
         QString description;
         QString organization;
         QString expirationDate;
         QString relevantDate;
         bool voided;
         bool expired;

         QList<Barcode> barcodes;
         QString backgroundColor;
         QString foregroundColor;
         QString labelColor;
         QString logoText;
         QString barcodeFormat;
         QString stripExtraForegroundColor;
         QString stripExtraLabelColor;

         explicit operator QVariant() const
         {
            QVariantMap m;
            m.insert("description", description);
            m.insert("organization", organization);
            m.insert("expirationDate", expirationDate);
            m.insert("relevantDate", relevantDate);
            m.insert("voided", voided);
            m.insert("expired", expired);
            m.insert("backgroundColor", backgroundColor);
            m.insert("foregroundColor", foregroundColor);
            m.insert("labelColor", labelColor);
            m.insert("logoText", logoText);
            m.insert("organization", organization);
            m.insert("barcodeFormat", barcodeFormat);
            m.insert("stripExtraForegroundColor", stripExtraForegroundColor);
            m.insert("stripExtraLabelColor", stripExtraLabelColor);

            QVariantList codes;

            for (const auto& barcode : barcodes)
               codes << static_cast<QVariant>(barcode);

            m.insert("barcodes", codes);

            return m;
         }
   };

   struct PassStyleField
   {
         QString key;
         QString value;
         QString label;

         explicit operator QVariant() const
         {
            QVariantMap m;
            m.insert("key", key);
            m.insert("value", value);
            m.insert("label", label);
            return m;
         }
   };

   struct PassStyle
   {
         QString style;
         QList<PassStyleField> headerFields;
         QList<PassStyleField> primaryFields;
         QList<PassStyleField> secondaryFields;
         QList<PassStyleField> auxiliaryFields;
         QList<PassStyleField> backFields;
         QString transitType;
         qreal maxFieldLabelWidth;

         explicit operator QVariant() const
         {
            QVariantMap m;
            m.insert("style", style);
            m.insert("transitType", transitType);
            m.insert("maxFieldLabelWidth", maxFieldLabelWidth);

            QVariantList fields;

            for (const auto& f : headerFields)
               fields << static_cast<QVariant>(f);

            m.insert("headerFields", fields);

            fields.clear();

            for (const auto& f : primaryFields)
               fields << static_cast<QVariant>(f);

            m.insert("primaryFields", fields);

            fields.clear();

            for (const auto& f : secondaryFields)
               fields << static_cast<QVariant>(f);

            m.insert("secondaryFields", fields);

            fields.clear();

            for (const auto& f : auxiliaryFields)
               fields << static_cast<QVariant>(f);

            m.insert("auxiliaryFields", fields);

            fields.clear();

            for (const auto& f : backFields)
               fields << static_cast<QVariant>(f);

            m.insert("backFields", fields);

            return m;
         }
   };

   struct Pass
   {
         QString id;
         QDateTime modified;
         QDateTime sortingDate;
         QString filePath;

         Standard standard;
         PassStyle details;
         WebService webservice;
         QString updateError;

         QImage imgBackground;
         QImage imgFooter;
         QImage imgIcon;
         QImage imgLogo;
         QImage imgStrip;
         QImage imgThumbnail;         
         bool haveStripImage;

         explicit operator QVariant() const
         {
            QVariantMap m;
            m.insert("id", id);
            m.insert("modified", modified);
            m.insert("filePath", filePath);
            m.insert("standard", static_cast<QVariant>(standard));
            m.insert("details", static_cast<QVariant>(details));
            m.insert("webservice", static_cast<QVariant>(webservice));
            m.insert("updateError", updateError);
            m.insert("haveStripImage", haveStripImage);

            return m;
         }
   };

   struct PassResult
   {
         Pass* pass;
         QString err;
   };

   // **************************************************************************
   // class Pkpass
   // **************************************************************************

   class Pkpass
   {
      public:
         Pkpass();
         PassResult openPass(const QFileInfo& info);

         void setDefaultFont(QFont to) { defaultFont = to; }

      protected:
         QString readPass(Pass* pass, QuaZip& archive);
         QJsonDocument readPassDocument(const QByteArray& data, QString& err);
         QString readImages(Pass* pass, QuaZip& archive, const QStringList& archiveContents);
         QString readImage(QImage* dest, QuaZip& archive, const QStringList& archiveContents, QString imageName);
         QString readLocalization(Pass* pass, QuaZip& archive, const QStringList& archiveContents);
         QString readLocalization(Pass* pass, QuaZip& archive, const QString& localization);

         QString readPassStandard(Pass* pass, QJsonObject& object);
         QString readPassBarcode(Pass* pass, QJsonObject object);
         QString readPassStyle(Pass* pass, QJsonObject object);
         QString readPassStyleFields(QList<PassStyleField>& fields, QJsonArray object);

         const QString& translate(QString& other);
         QString parseColor(QString rgbString);

         Translation currentTranslation;
         QRegExp trailingCommaRegEx1, trailingCommaRegEx2;
         QFont defaultFont;
   };   

} // namespace passes

#endif // PKPASS_H
