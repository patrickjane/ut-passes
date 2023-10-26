// **************************************************************************
// class PkPass
// 02.07.2021
// Model for handling locally stored passes managed by this app
// **************************************************************************
// MIT License
// Copyright © 2021 Patrick Fial
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
// associated documentation files (the “Software”), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge, publish, distribute,
// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions: The above copyright notice and this
// permission notice shall be included in all copies or substantial portions of the Software. THE
// SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// **************************************************************************
// includes
// **************************************************************************

#ifndef PKPASS_H
#define PKPASS_H

#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QFont>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QRegExp>

#include "quazip/quazip.h"
#include <memory>

namespace passes {
static QString filePassJson = "pass.json";
static QString fileBackgroundPng = "background.png";
static QString fileFooterPng = "footer.png";
static QString fileIconPng = "icon.png";
static QString fileLogoPng = "logo.png";
static QString fileStripPng = "strip.png";
static QString fileThumbnailPng = "thumbnail.png";

using Translation = QMap<QString, QString>;

// **************************************************************************
// struct PassItem
// **************************************************************************

struct Barcode {
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

struct WebService {
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

struct Standard {
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

struct PassStyleField {
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

struct PassStyle {
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

struct Pass {
    QString id;
    QDateTime modified;
    QDateTime sortingDate;
    QString filePath;
    QString bundleName;
    bool bundleExpired;
    int bundleIndex;
    QString bundleId;
    std::vector<std::shared_ptr<Pass>> bundlePasses;

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

    ~Pass()
    {
        qDebug() << "DESTRUCT PASS";
    }

    explicit operator QVariant() const
    {
        QVariantMap m;
        m.insert("id", id);
        m.insert("bundleName", bundleName);
        m.insert("bundleIndex", bundleIndex);
        m.insert("bundleExpired", bundleExpired);
        m.insert("bundleId", bundleId);
        m.insert("modified", modified);
        m.insert("filePath", filePath);
        m.insert("standard", static_cast<QVariant>(standard));
        m.insert("details", static_cast<QVariant>(details));
        m.insert("webservice", static_cast<QVariant>(webservice));
        m.insert("updateError", updateError);
        m.insert("haveStripImage", haveStripImage);

        QVariantList subPasses;

        for (const auto& pass : bundlePasses) {
            subPasses << static_cast<QVariant>(*pass);
        }

        m.insert("bundlePasses", subPasses);

        return m;
    }
};

using PassPtr = std::shared_ptr<Pass>;
using PassList = std::vector<PassPtr>;
using PassMap = std::map<QString, PassPtr>;

using PassResult = std::variant<QString, PassPtr>;
using BundleResult = std::variant<QString, PassList>;

// **************************************************************************
// class Pkpass
// **************************************************************************

class Pkpass {
public:
    Pkpass();

    PassResult openPass(const QFileInfo& info);
    BundleResult extractBundle(const QFileInfo& info);

    void setDefaultFont(QFont to)
    {
        defaultFont = to;
    }

protected:
    QString readPass(PassPtr pass, QuaZip& archive);
    QJsonDocument readPassDocument(const QByteArray& data, QString& err);
    QString readImages(PassPtr pass, QuaZip& archive, const QStringList& archiveContents);
    QString readImage(QImage* dest, QuaZip& archive, const QStringList& archiveContents,
                      QString imageName);
    QString readLocalization(PassPtr pass, QuaZip& archive, const QStringList& archiveContents);
    QString readLocalization(PassPtr pass, QuaZip& archive, const QString& localization);

    QString readPassStandard(PassPtr pass, QJsonObject& object);
    QString readPassBarcode(PassPtr pass, QJsonObject object);
    QString readPassStyle(PassPtr pass, QJsonObject object);
    QString readPassStyleFields(QList<PassStyleField>& fields, QJsonArray object);

    const QString& translate(QString& other);
    QString parseColor(QString rgbString);

    Translation currentTranslation;
    QRegExp trailingCommaRegEx1, trailingCommaRegEx2;
    QFont defaultFont;
};

} // namespace passes

#endif // PKPASS_H
