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

#include "pkpass.h"
#include <QBuffer>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFontMetrics>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "barcode.h"
#include "quazip/quazipfile.h"

namespace C {
#include <libintl.h>
}

#include <cmath>
namespace colors {

// luminosity calculation as per
// https://stackoverflow.com/questions/596216/formula-to-determine-perceived-brightness-of-rgb-color

double sRGBtoLin(double colorChannel)
{
    if (colorChannel <= 0.04045)
        return colorChannel / 12.92;

    return pow(((colorChannel + 0.055) / 1.055), 2.4);
}

double getLuminance(const QColor& color)
{
    double vR = static_cast<double>(color.red()) / 255.0;
    double vG = static_cast<double>(color.green()) / 255.0;
    double vB = static_cast<double>(color.blue()) / 255.0;

    return (0.2126 * sRGBtoLin(vR) + 0.7152 * sRGBtoLin(vG) + 0.0722 * sRGBtoLin(vB));
}
} // namespace colors

namespace passes {
QByteArray fileMd5(const QString& fileName)
{
    QFile f(fileName);

    if (f.open(QFile::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Md5);

        if (hash.addData(&f))
            return hash.result().toHex();
    }

    return QByteArray();
}

QByteArray dataMd5(const QByteArray& data)
{
    QCryptographicHash hash(QCryptographicHash::Md5);

    hash.addData(data);
    return hash.result().toHex();
}

// **************************************************************************
// class PkpassParser
// **************************************************************************

Pkpass::Pkpass()
  : trailingCommaRegEx1(",[\\s\r\n]*\\]"),
    trailingCommaRegEx2(",[\\s\r\n]*\\}"),
    defaultFont(QFont())
{
}

// **************************************************************************
// extractBundle
// **************************************************************************

std::optional<QString> Pkpass::extractBundle(const QFileInfo& info)
{
    QuaZip archive(info.absoluteFilePath());

    if (!archive.open(QuaZip::mdUnzip))
        return QString(C::gettext("Can't open passes bundle (%1)")).arg(archive.getZipError());

    auto passBundlePrefix = "BUNDLE_" + info.baseName() + "_BUNDLE_";
    auto archiveContents = archive.getFileNameList();
    std::optional<QString> res = std::nullopt;

    for (auto& fileName : archiveContents) {
        QFileInfo extractedFileName;
        extractedFileName.setFile(info.dir(), passBundlePrefix + fileName);

        if (QFile::exists(extractedFileName.absoluteFilePath())) {
            res = QString(C::gettext("Contained bundle pass already exists, can't extract bundle"));
            break;
        }

        qDebug() << "Contained pass: " << fileName << " from " << info.baseName() << " save to "
                 << extractedFileName.absoluteFilePath();

        archive.setCurrentFile(fileName);

        QString err;
        QuaZipFile archiveFile(&archive);
        QFile targetFile(extractedFileName.absoluteFilePath());

        if (!archiveFile.open(QIODevice::ReadOnly)) {
            res = QString(
                    C::gettext("Failed to open bundle/bundle contents, can't extract bundle (%1)"))
                    .arg(archive.getZipError());
            break;
        }

        if (!targetFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            archiveFile.close();
            res = QString(C::gettext(
                            "Failed to open extracted pass for writing, can't extract bundle (%1)"))
                    .arg(targetFile.errorString());
            break;
        }

        auto contents = archiveFile.readAll();
        auto written = targetFile.write(contents);

        targetFile.close();
        archiveFile.close();

        if (written < contents.size()) {
            res = QString(C::gettext("Failed extract pass from bundle (%1)"))
                    .arg(targetFile.errorString());
            break;
        }

        auto passResult = openPass(extractedFileName.absoluteFilePath());

        delete passResult.pass;

        if (!passResult.err.isEmpty()) {
            qDebug() << "Unable to open extracted pass: " << passResult.err;

            res = QString(C::gettext("Unable to open extracted pass from bundle (%1)"))
                    .arg(passResult.err);
            break;
        }
    }

    archive.close();

    if (!res) {
        if (!QFile::remove(info.absoluteFilePath())) {
            res = QString(C::gettext("Failed to delete bundle after extraction"));
        }
    } else {
        for (const QFileInfo& info : info.dir().entryInfoList(
               QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Readable)) {
            if (info.fileName().startsWith(".") || !info.fileName().endsWith(".pkpass")
                || !info.fileName().startsWith(passBundlePrefix))
                continue;

            qDebug() << "Delete extracted pass from incomplete/corrupt bundle "
                     << info.absoluteFilePath();

            if (!QFile::remove(info.absoluteFilePath())) {
                res = QString(C::gettext(
                  "Failed to delete already extracted pass(es) of incomplete/corrupt bundle"));
            }
        }
    }

    return res;
}

// **************************************************************************
// openPass
// **************************************************************************

PassResult Pkpass::openPass(const QFileInfo& info)
{
    Pass* pass = new Pass();
    currentTranslation.clear();

    QuaZip archive(info.absoluteFilePath());

    bool res = archive.open(QuaZip::mdUnzip);

    if (!res)
        return {pass, QString(archive.getZipError())};

    auto archiveContents = archive.getFileNameList();

    if (!archiveContents.contains(filePassJson))
        return {pass, C::gettext("Archive does not contain a valid pass")};

    QString err = readLocalization(pass, archive, archiveContents);

    pass->haveStripImage = false;

    if (err.isEmpty())
        err = readPass(pass, archive);
    if (err.isEmpty())
        err = readImages(pass, archive, archiveContents);

    QFontMetrics fm(defaultFont);

    qreal maxWidth = 0.0;

    for (auto f : pass->details.secondaryFields) {
        auto width = fm.tightBoundingRect(f.label).width();

        maxWidth = maxWidth > width ? maxWidth : width;
    }

    pass->details.maxFieldLabelWidth = maxWidth;
    pass->id = fileMd5(info.absoluteFilePath());
    pass->modified = info.lastModified();
    pass->filePath = info.absoluteFilePath();
    pass->bundleExpired = false;
    pass->bundleIndex = -1;

    if (info.baseName().startsWith("BUNDLE_") && info.baseName().contains("_BUNDLE")) {
        int endIdx = info.baseName().indexOf("_BUNDLE");

        pass->bundleName = info.baseName().mid(7, endIdx - 7);
    }

    if (!pass->sortingDate.isValid())
        pass->sortingDate = pass->modified;

    archive.close();
    return {pass, err};
}

// **************************************************************************
// readPassJson
// **************************************************************************

QString Pkpass::readPass(Pass* pass, QuaZip& archive)
{
    archive.setCurrentFile(filePassJson);

    QString err;
    QuaZipFile file(&archive);

    file.open(QIODevice::ReadOnly);

    auto contents = file.readAll();
    auto doc = readPassDocument(contents, err);

    file.close();

    if (!err.isEmpty())
        return err;

    if (doc.isNull() || !doc.isObject() || doc.object().isEmpty())
        return C::gettext("Pass information is invalid (empty)");

    auto root = doc.object();

    err = readPassStandard(pass, root);

    if (err.isEmpty())
        err = readPassStyle(pass, root);

    return err;
}

// **************************************************************************
// readPassDocument
// **************************************************************************

QJsonDocument Pkpass::readPassDocument(const QByteArray& data, QString& err)
{
    // fix trailing commas, remove special characters

    QString stringData = QString::fromUtf8(data)
                           .replace('\t', "")
                           .replace('\n', "")
                           .replace('\r', "")
                           .replace(trailingCommaRegEx1, "]")
                           .replace(trailingCommaRegEx2, "}");

    // remove garbage at end of JSON

    stringData.remove(stringData.lastIndexOf('}') + 1, stringData.length() + 1);

    QJsonParseError jsonErr;
    auto doc = QJsonDocument::fromJson(stringData.toUtf8(), &jsonErr);

    if (jsonErr.error == QJsonParseError::NoError)
        return doc;

    // try UTF-16 (e.g. subway card uses this)
    // fix trailing commas, remove special characters

    auto utf16String = QString::fromUtf16((const ushort*) data.data())
                         .replace('\t', "")
                         .replace('\n', "")
                         .replace('\r', "")
                         .replace(trailingCommaRegEx1, "]")
                         .replace(trailingCommaRegEx2, "}");

    // remove garbage at end of JSON

    utf16String.remove(utf16String.lastIndexOf('}') + 1, utf16String.length() + 1);

    doc = QJsonDocument::fromJson(utf16String.toUtf8(), &jsonErr);

    if (jsonErr.error == QJsonParseError::NoError)
        return doc;

    // try UTF-32 (e.g. subway card uses this)
    // fix trailing commas, remove special characters

    auto utf32String = QString::fromUcs4((const uint*) data.data())
                         .replace('\t', "")
                         .replace('\n', "")
                         .replace('\r', "")
                         .replace(trailingCommaRegEx1, "]")
                         .replace(trailingCommaRegEx2, "}");

    // remove garbage at end of JSON

    utf32String.remove(utf32String.lastIndexOf('}') + 1, utf32String.length() + 1);

    doc = QJsonDocument::fromJson(utf32String.toUtf8(), &jsonErr);

    if (jsonErr.error != QJsonParseError::NoError)
        err = QString(C::gettext("Pass information is invalid")) + " (" + jsonErr.errorString()
              + ")";

    return doc;
}

// **************************************************************************
// readPassStandard
// **************************************************************************

QString Pkpass::readPassStandard(Pass* pass, QJsonObject& object)
{
    if (!object.contains("description") || !object.contains("organizationName"))
        return C::gettext("Pass information is invalid (missing description/organization key(s))");

    pass->standard.expired = false;

    pass->standard.description = object["description"].toString();
    pass->standard.organization = object["organizationName"].toString();

    translate(pass->standard.description);
    translate(pass->standard.organization);

    if (object.contains("expirationDate")) {
        pass->standard.expirationDate = object["expirationDate"].toString();
        QDateTime dt = QDateTime::fromString(pass->standard.expirationDate, Qt::ISODate);

        if (QDateTime::currentDateTime().secsTo(dt) <= 0)
            pass->standard.expired = true;
    }

    if (object.contains("relevantDate")) {
        pass->standard.relevantDate = object["relevantDate"].toString();
        pass->sortingDate = QDateTime::fromString(pass->standard.relevantDate, Qt::ISODate);

        if (!object.contains("expirationDate")
            && QDateTime::currentDateTime().secsTo(pass->sortingDate) <= 0) {
            pass->standard.expired = true;
        }
    }

    if (object.contains("voided"))
        pass->standard.voided = object["voided"].toBool();

    if (object.contains("backgroundColor"))
        pass->standard.backgroundColor = parseColor(object["backgroundColor"].toString());

    if (object.contains("foregroundColor"))
        pass->standard.foregroundColor = parseColor(object["foregroundColor"].toString());

    if (object.contains("labelColor"))
        pass->standard.labelColor = parseColor(object["labelColor"].toString());

    if (object.contains("logoText")) {
        pass->standard.logoText = object["logoText"].toString();
        translate(pass->standard.logoText);
    }

    // parse webservice block

    pass->webservice.webserviceBroken = false;

    if (object.contains("authenticationToken") && object.contains("webServiceURL")
        && object.contains("passTypeIdentifier") && object.contains("serialNumber")) {
        pass->webservice.accessToken = object["authenticationToken"].toString();
        pass->webservice.url = object["webServiceURL"].toString() + "/v1/passes/"
                               + object["passTypeIdentifier"].toString() + "/"
                               + object["serialNumber"].toString();
    }

    // parse all contained barcodes

    if (object.contains("barcode"))
        return readPassBarcode(pass, object["barcode"].toObject());

    if (object.contains("barcodes")) {
        auto barcodes = object["barcodes"].toArray();

        for (int i = 0; i < barcodes.size(); i++) {
            auto errString = readPassBarcode(pass, barcodes[i].toObject());

            if (!errString.isEmpty())
                return errString;
        }
    }

    return "";
}

// **************************************************************************
// readPassBarcode
// **************************************************************************

QString Pkpass::readPassBarcode(Pass* pass, QJsonObject object)
{
    if (object.isEmpty() || !object.contains("format") || !object.contains("message"))
        return C::gettext("Pass contains invalid/incomplete barcode information");

    Barcode bc;

    QString format = object["format"].toString();
    QString message = object["message"].toString();
    QString encoding = object.contains("encoding") ? object["encoding"].toString() : QString();
    QString altText = object.contains("altText") ? object["altText"].toString() : QString();

    bc.format = format;
    bc.message = message;
    bc.encoding = encoding;
    bc.altText = altText;

    auto errString = BarcodeGenerator::generate(message, format, &bc.image);

    if (!errString.isEmpty())
        return errString;

    pass->standard.barcodes.push_back(std::move(bc));

    if (pass->standard.barcodeFormat.isEmpty())
        pass->standard.barcodeFormat = format;

    return errString;
}

// **************************************************************************
// readPassStyle
// **************************************************************************

QString Pkpass::readPassStyle(Pass* pass, QJsonObject object)
{
    QString err;
    QJsonObject styleObject;

    if (object.contains("boardingPass")) {
        styleObject = object["boardingPass"].toObject();
        pass->details.style = "boardingPass";
    } else if (object.contains("coupon")) {
        styleObject = object["coupon"].toObject();
        pass->details.style = "coupon";
    } else if (object.contains("eventTicket")) {
        styleObject = object["eventTicket"].toObject();
        pass->details.style = "eventTicket";
    } else if (object.contains("generic")) {
        styleObject = object["generic"].toObject();
        pass->details.style = "generic";
    } else if (object.contains("storeCard")) {
        styleObject = object["storeCard"].toObject();
        pass->details.style = "storeCard";
    }

    if (styleObject.isEmpty())
        return err;

    if (styleObject.contains("headerFields"))
        err = readPassStyleFields(pass->details.headerFields,
                                  styleObject["headerFields"].toArray());

    if (err.isEmpty() && styleObject.contains("primaryFields"))
        err = readPassStyleFields(pass->details.primaryFields,
                                  styleObject["primaryFields"].toArray());

    if (err.isEmpty() && styleObject.contains("secondaryFields"))
        err = readPassStyleFields(pass->details.secondaryFields,
                                  styleObject["secondaryFields"].toArray());

    if (err.isEmpty() && styleObject.contains("auxiliaryFields"))
        err = readPassStyleFields(pass->details.auxiliaryFields,
                                  styleObject["auxiliaryFields"].toArray());

    if (err.isEmpty() && styleObject.contains("backFields"))
        err = readPassStyleFields(pass->details.backFields, styleObject["backFields"].toArray());

    return err;
}

// **************************************************************************
// readPassStyleFields
// **************************************************************************

QString Pkpass::readPassStyleFields(QList<PassStyleField>& fields, QJsonArray jsonFields)
{
    if (jsonFields.isEmpty())
        return "";

    for (auto v : jsonFields) {
        auto object = v.toObject();

        if (object.isEmpty())
            continue;

        QString key = object.contains("key") ? object["key"].toString() : QString();
        QString value = object.contains("value") ? object["value"].toString() : QString();
        QString label = object.contains("label") ? object["label"].toString() : QString();

        if (object.contains("dateStyle") || object.contains("timeStyle")) {
            QDateTime dt = QDateTime::fromString(value, Qt::ISODate).toLocalTime();

            if (dt.isValid()) {
                QString format;

                if (object.contains("dateStyle") && object.contains("timeStyle")
                    && object["dateStyle"] != "PKDateStyleNone"
                    && object["timeStyle"] != "PKDateStyleNone") {
                    if (object["dateStyle"] == "PKDateStyleShort")
                        format = QLocale::system().dateTimeFormat(QLocale::ShortFormat);
                    else
                        format = "dddd, " + QLocale::system().dateTimeFormat(QLocale::ShortFormat);
                } else if (object.contains("timeStyle")
                           && object["timeStyle"] != "PKDateStyleNone") {
                    if (object["timeStyle"] == "PKDateStyleShort")
                        format = QLocale::system().timeFormat(QLocale::ShortFormat);
                    else
                        format = QLocale::system().timeFormat(QLocale::ShortFormat);
                } else if (object.contains("dateStyle")
                           && object["dateStyle"] != "PKDateStyleNone") {
                    if (object["dateStyle"] == "PKDateStyleShort")
                        format = QLocale::system().dateFormat(QLocale::ShortFormat);
                    else
                        format = QLocale::system().dateFormat(QLocale::LongFormat);
                }

                if (!format.isEmpty())
                    value = dt.toString(format);
            }
        }

        fields << PassStyleField {translate(key), translate(value), translate(label)};
    }

    return "";
}

// **************************************************************************
// readImages
// **************************************************************************

QString Pkpass::readImages(Pass* pass, QuaZip& archive, const QStringList& archiveContents)
{
    QString err;

    if (err.isEmpty())
        err = readImage(&pass->imgBackground, archive, archiveContents, "background");
    if (err.isEmpty())
        err = readImage(&pass->imgFooter, archive, archiveContents, "footer");
    if (err.isEmpty())
        err = readImage(&pass->imgIcon, archive, archiveContents, "icon");
    if (err.isEmpty())
        err = readImage(&pass->imgLogo, archive, archiveContents, "logo");
    if (err.isEmpty())
        err = readImage(&pass->imgStrip, archive, archiveContents, "strip");
    if (err.isEmpty())
        err = readImage(&pass->imgThumbnail, archive, archiveContents, "thumbnail");

    if (!pass->imgStrip.isNull()) {
        pass->haveStripImage = true;

        // check if we need different color for the strip foreground text in case the
        // strip color does not match well with the passes foreground text color

        QColor colorOfStrip = QColor::fromRgb(pass->imgStrip.pixel(10, 10));
        QColor passForegroundColor(pass->standard.foregroundColor);
        QColor passLabelColor(pass->standard.labelColor);

        double lumStrip = colors::getLuminance(colorOfStrip);
        double lumForground = colors::getLuminance(passForegroundColor);
        double lumLabel = colors::getLuminance(passLabelColor);

        if (lumStrip < 0.25 && lumForground < 0.25)
            pass->standard.stripExtraForegroundColor = "#EDEDED";
        else if (lumStrip > 0.25 && lumForground > 0.25)
            pass->standard.stripExtraForegroundColor = "#3A3A3A";

        if (lumStrip < 0.25 && lumLabel < 0.25)
            pass->standard.stripExtraLabelColor = "#EDEDED";
        else if (lumStrip > 0.25 && lumLabel > 0.25)
            pass->standard.stripExtraLabelColor = "#3A3A3A";
    }

    return err;
}

// **************************************************************************
// readImage
// **************************************************************************

QString Pkpass::readImage(QImage* dest, QuaZip& archive, const QStringList& archiveContents,
                          QString imageName)
{
    static QStringList extensions {"@3x.png", "@2x.png", ".png"};

    bool res = true;

    foreach (const QString ext, extensions) {
        if (!archiveContents.contains(imageName + ext))
            continue;

        archive.setCurrentFile(imageName + ext);

        QuaZipFile file(&archive);

        file.open(QIODevice::ReadOnly);

        res = dest->loadFromData(file.readAll());

        file.close();
        break;
    }

    return res ? "" : C::gettext("Pass contains invalid/incomplete image data");
}

// **************************************************************************
// readLocalization
// **************************************************************************

QString Pkpass::readLocalization(Pass* pass, QuaZip& archive, const QStringList& archiveContents)
{
    QString localizationLocale = QLocale::system().name().mid(0, 2) + ".lproj/pass.strings";
    static QString localizationEnglish = "en.lproj/pass.strings";

    currentTranslation.clear();

    if (archiveContents.contains(localizationLocale))
        return readLocalization(pass, archive, localizationLocale);

    if (archiveContents.contains(localizationEnglish))
        return readLocalization(pass, archive, localizationEnglish);

    return "";
}

// **************************************************************************
// readLocalization
// **************************************************************************

QString Pkpass::readLocalization(Pass* pass, QuaZip& archive, const QString& localization)
{
    archive.setCurrentFile(localization);

    QuaZipFile file(&archive);

    file.open(QIODevice::ReadOnly);

    QByteArray line;

    do {
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

    return "";
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
        return rgbString;

    QStringList comps = rgbString.replace("rgb(", "").replace(")", "").split(",");

    if (comps.size() < 3)
        return "";

    return QColor(comps[0].toInt(), comps[1].toInt(), comps[2].toInt()).name();
}

} // namespace passes
