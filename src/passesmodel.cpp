// **************************************************************************
// class PassesModel
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

#include "passesmodel.h"
#include <QDebug>
#include <QStandardPaths>

#include "async.hpp"
#include "quazip/quazipfile.h"

namespace C {
#include <libintl.h>
}

namespace passes {
// **************************************************************************
// class PassesModel
// **************************************************************************

PassesModel* PassesModel::instance = nullptr;

PassesModel::PassesModel(QObject* parent)
  : QAbstractListModel(parent), storageReady(false), countExpired(0)
{
    instance = this;
}

// **************************************************************************
// init
// **************************************************************************

QString PassesModel::init()
{
    QString dataPath = getDataPath();

    if (!dataPath.size())
        return C::gettext("Failed to determine writable app data storage location");

    QDir dir(dataPath + "/passes");

    if (!dir.exists()) {
        bool res = dir.mkpath(dataPath + "/passes");

        if (!res)
            return QString(C::gettext("App data storage location inaccessible")) + " ("
                   + QString(C::gettext("can't create subfolder:")) + " " + dir.absolutePath()
                   + ")";
    }

    passesDir.setPath(dir.path());
    passesDir.setSorting(QDir::Time);
    storageReady = true;

    return "";
}

// **************************************************************************
// data
// **************************************************************************

QVariant PassesModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole || role == PassRole)
        return static_cast<QVariant>(*(mItems[index.row()]));

    return QVariant();
}

// **************************************************************************
// rowCount
// **************************************************************************

int PassesModel::rowCount(const QModelIndex& /*parent*/) const
{
    return mItems.size();
}

// **************************************************************************
// roleNames
// **************************************************************************

QHash<int, QByteArray> PassesModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[PassRole] = "pass";
    return roles;
}

// **************************************************************************
// getDataPath
// **************************************************************************

QString PassesModel::getDataPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

// **************************************************************************
// readPasses
// **************************************************************************

void PassesModel::readPasses(QVariantList& failed, QMap<QString, QList<Pass*>>& bundles,
                             bool doShowExpired)
{
    for (const QFileInfo& info : passesDir.entryInfoList(QDir::Files | QDir::NoSymLinks
                                                         | QDir::NoDotAndDotDot | QDir::Readable)) {
        if (info.fileName().startsWith(".") || !info.fileName().endsWith(".pkpass"))
            continue;

        if (doShowExpired && isOpen(info.absoluteFilePath()))
            continue;

        auto passResult = pkpass.openPass(info.absoluteFilePath());

        if (!passResult.err.isEmpty()) {
            qDebug() << "Pass open failed: " << passResult.err;

            QVariantMap failedPass;
            failedPass["filePath"] = passResult.pass->filePath;
            failedPass["error"] = passResult.err;
            failed.append(failedPass);

            delete passResult.pass;
        } else if (!passResult.pass->bundleName.isEmpty()) {
            if (!bundles.contains(passResult.pass->bundleName)) {
                bundles[passResult.pass->bundleName] = QList<Pass*>() << passResult.pass;
            } else {
                auto& list = bundles[passResult.pass->bundleName];

                list << passResult.pass;
            }
        } else if (doShowExpired != passResult.pass->standard.expired) {
            delete passResult.pass;

            if (!doShowExpired)
                countExpired++;
        } else {
            mItemMap[passResult.pass->id] = passResult.pass;
            mItems.push_back(passResult.pass);
        }
    }
}

// **************************************************************************
// readPasses
// **************************************************************************

void PassesModel::addBundlePasses(QMap<QString, QList<Pass*>>& bundles, bool doShowExpired)
{
    int idx = 0;

    for (auto bundleName : bundles.keys()) {
        auto bundlePass = new Pass();
        bundlePass->id = "";
        bundlePass->bundleName = bundleName;
        bundlePass->bundleExpired = true;

        for (auto pass : bundles[bundleName]) {
            if (bundlePass->id == "")
                bundlePass->id = pass->id;

            if (!pass->standard.expired)
                bundlePass->bundleExpired = false;

            if (!bundlePass->modified.isValid() || bundlePass->modified.secsTo(pass->modified)) {
                bundlePass->modified = pass->modified;
            }

            if (!bundlePass->sortingDate.isValid()
                || bundlePass->sortingDate.secsTo(pass->sortingDate)) {
                bundlePass->sortingDate = pass->sortingDate;
            }

            pass->bundleIndex = idx++;
            pass->bundleId = bundlePass->id;

            bundlePass->bundlePasses << pass;
        }

        if (doShowExpired != bundlePass->bundleExpired) {
            delete bundlePass;

            if (!doShowExpired)
                countExpired++;
        } else {
            mItemMap[bundlePass->id] = bundlePass;
            mItems.push_back(bundlePass);
        }
    }
}

// **************************************************************************
// reload
// **************************************************************************

void PassesModel::reload()
{
    if (!storageReady || !passesDir.path().size()) {
        qDebug() << "Storage directory not initialized";
        return;
    }

    beginResetModel();

    for (auto p : mItems)
        delete p;

    mItems.clear();
    mItemMap.clear();
    countExpired = 0;

    QVariantList failed;
    QMap<QString, QList<Pass*>> bundles;

    // loop bundles first, and extract the .pkpass files contained, so they will be picked up by the
    // next loop

    for (const QFileInfo& info : passesDir.entryInfoList(QDir::Files | QDir::NoSymLinks
                                                         | QDir::NoDotAndDotDot | QDir::Readable)) {
        if (info.fileName().startsWith(".") || !info.fileName().endsWith(".pkpasses"))
            continue;

        auto err = pkpass.extractBundle(info);

        if (err) {
            qDebug() << "Bundle extract failed: " << *err;

            QVariantMap failedPass;
            failedPass["filePath"] = info.absoluteFilePath();
            failedPass["error"] = *err;
            failed.append(failedPass);
        }
    };

    // actually open all available .pkpass files

    readPasses(failed, bundles, false);
    addBundlePasses(bundles, false);

    std::sort(mItems.begin(), mItems.end(), passSorter);

    endResetModel();
    emit countExpiredChanged();
    emit countChanged();

    if (failed.length()) {
        qDebug() << failed.length() << " passed failed to open";
        emit failedPasses(failed);
    }
}

// **************************************************************************
// openPasses
// **************************************************************************

void PassesModel::showExpired()
{
    size_t oldCount = mItems.size();
    QVariantList failed;
    QMap<QString, QList<Pass*>> bundles;

    readPasses(failed, bundles, true);
    addBundlePasses(bundles, true);

    if (failed.length()) {
        qDebug() << failed.length() << " passed failed to open";
        emit failedPasses(failed);
    }

    if (mItems.size() != oldCount) {
        std::sort(mItems.begin(), mItems.end(), passSorter);
        beginResetModel();
        endResetModel();
        emit countChanged();
    }
}

// **************************************************************************
// hideExpired
// **************************************************************************

void PassesModel::hideExpired()
{
    size_t oldCount = mItems.size();

    for (auto it = mItems.begin(); it != mItems.end();) {
        if ((*it)->standard.expired || (*it)->bundleExpired) {
            mItemMap.erase((*it)->id);
            delete *it;
            it = mItems.erase(it);
        } else {
            it++;
        }
    }

    if (mItems.size() != oldCount) {
        beginResetModel();
        endResetModel();
        emit countChanged();
    }
}

// **************************************************************************
// isOpen
// **************************************************************************

bool PassesModel::isOpen(const QString& filePath)
{
    auto it = std::find_if(mItems.begin(), mItems.end(),
                           [&filePath](Pass* pass) { return pass->filePath == filePath; });

    return it != mItems.end();
}

// **************************************************************************
// importPass
// **************************************************************************

QString PassesModel::importPass(const QString& filePath, bool expiredShown)
{
    QString fp = filePath;

    if (fp.startsWith("file://"))
        fp.remove("file://");

    if (!storageReady)
        return C::gettext("Storage directory inaccessible, cannot import pass");

    if (!QFile::exists(fp))
        return C::gettext("File path of the pass seems to be invalid");

    QFileInfo info(fp);
    QString targetPath = passesDir.path() + "/" + info.fileName();

    if (QFile::exists(targetPath))
        return C::gettext("Same pass has already been imported");

    QFile sourceFile(fp);
    bool res = sourceFile.copy(targetPath);

    if (!res)
        return QString(C::gettext("Failed to import pass into storage directory ")) + " ("
               + sourceFile.errorString() + ")";

    info.setFile(targetPath);

    auto passResult = pkpass.openPass(info.absoluteFilePath());
    auto it = std::find_if(mItems.begin(), mItems.end(),
                           [&passResult](Pass* pass) { return pass->id == passResult.pass->id; });

    if (!passResult.err.isEmpty()) {
        qDebug() << "Pass open failed: " << passResult.err;

        QFile::remove(targetPath);

        delete passResult.pass;
        return QString(C::gettext("Failed to open pass")) + " (" + passResult.err + ")";
    } else if (it != mItems.end()) {
        QFile::remove(targetPath);

        delete passResult.pass;
        return C::gettext("Pass with the same barcode has already been imported");
    } else if (passResult.pass->standard.expired && !expiredShown) {
        countExpired++;
        delete passResult.pass;
    } else {
        if (passResult.pass->standard.expired)
            countExpired++;

        mItemMap[passResult.pass->id] = passResult.pass;
        mItems.push_back(passResult.pass);
    }

    std::sort(mItems.begin(), mItems.end(), passSorter);
    beginResetModel();
    endResetModel();
    emit countChanged();
    emit countExpiredChanged();

    return "";
}

// **************************************************************************
// deleteFile
// **************************************************************************

QString PassesModel::deleteFile(QString filePath)
{
    if (!QFile::exists(filePath))
        return C::gettext("Failed to delete pass (pass unknown)");

    QFile passFile(filePath);
    bool res = passFile.remove();

    if (!res)
        return QString(C::gettext("Failed to delete pass from storage directory (%1)"))
          .arg(passFile.errorString());

    return "";
}

// **************************************************************************
// deletePass
// **************************************************************************

QString PassesModel::deletePass(QString id)
{
    auto it = std::find_if(mItems.begin(), mItems.end(),
                           [&id](Pass* pass) { return pass->id == id; });

    if (it == mItems.end() || !mItemMap.count(id))
        return C::gettext("Failed to delete pass (pass unknown)");

    auto pass = mItemMap[id];

    if (pass->bundlePasses.size() > 0) {
        for (auto bundlePass : pass->bundlePasses) {
            QFile passFile(bundlePass->filePath);
            bool res = passFile.remove();

            if (!res)
                return QString(C::gettext("Failed to delete pass from storage directory (%1)"))
                  .arg(passFile.errorString());
        }
    } else {
        QFile passFile(pass->filePath);
        bool res = passFile.remove();

        if (!res)
            return QString(C::gettext("Failed to delete pass from storage directory (%1)"))
              .arg(passFile.errorString());
    }

    if (pass->standard.expired) {
        countExpired--;
        emit countExpiredChanged();
    }

    beginResetModel();

    mItemMap.erase(pass->id);
    mItems.erase(it);
    delete pass;

    endResetModel();
    emit countChanged();
    return "";
}

// **************************************************************************
// fetchPassUpdates (ALL passes)
// **************************************************************************

void PassesModel::fetchPassUpdates()
{
    async::eachSeries<Pass*>(
      mItems,
      [this](Pass* pass, auto next, int index) {
          if (pass->webservice.accessToken.isEmpty() || pass->webservice.webserviceBroken)
              return next("");

          this->fetchPassUpdate(pass, [this, pass, index, next](QString err, Pass* newPass) {
              auto modelIndex = this->createIndex(index, 0);

              pass->updateError = "";

              if (err.isEmpty() && newPass) {
                  mItemMap.erase(pass->id);
                  mItemMap[newPass->id] = newPass;
                  mItems[index] = newPass;
                  delete pass;

                  this->emit dataChanged(modelIndex, modelIndex);
              } else if (!err.isEmpty()) {
                  pass->updateError = err;
                  this->emit dataChanged(modelIndex, modelIndex);
              }

              return next("");
          });
      },
      [this](QString err) { emit passUpdatesFetched(err); });
}

// **************************************************************************
// fetchPassUpdate
// **************************************************************************

void PassesModel::fetchPassUpdate(Pass* pass, ResultCallback<Pass*> callback)
{
    if (!pass || pass->webservice.url.isEmpty() || pass->webservice.accessToken.isEmpty())
        return callback(C::gettext("Failed to fetch pass updates (no webservice info available)"),
                        nullptr);

    network::ReqHeaders headers;
    headers["Authorization"] = "ApplePass " + pass->webservice.accessToken;

    net.get<network::ReqCallback>(
      QUrl(pass->webservice.url), headers,
      [this, callback, pass](int err, int code, QByteArray body) {
          if (err != QNetworkReply::NoError || code != 200 || body.isEmpty()) {
              if (err == QNetworkReply::NoError && (code <= 299 || code >= 200))
                  return callback("", nullptr);

              pass->webservice.webserviceBroken = true;

              return callback(QString(
                                C::gettext("Loading pass update failed (Network error: %1/%2)"))
                                .arg(err)
                                .arg(code),
                              nullptr);
          }

          // try to open the payload (= new pass)
          // if successful, swap pass objects in model & on filesystem and delete the old one.

          PassResult res;
          res.pass = nullptr;

          auto tempFileName = pass->filePath + ".tmp";
          QFile tempFile(tempFileName);

          auto fileRes = tempFile.open(QIODevice::WriteOnly | QIODevice::Truncate);

          if (!fileRes)
              return callback(
                QString(
                  C::gettext("Failed to save pass update to storage / could not open file (%1)"))
                  .arg(tempFile.errorString()),
                nullptr);

          fileRes = tempFile.write(body);

          if (!fileRes)
              return callback(
                QString(
                  C::gettext("Failed to save pass update to storage / could not write file (%1)"))
                  .arg(tempFile.errorString()),
                nullptr);

          tempFile.close();

          QFileInfo info(tempFileName);

          res = pkpass.openPass(info);

          if (res.err.isEmpty() && res.pass) {
              QFile::remove(pass->filePath);

              fileRes = tempFile.rename(pass->filePath);

              if (!fileRes) {
                  delete res.pass;
                  return callback(
                    QString(
                      C::gettext(
                        "Failed to save pass update to storage / could replace existing pass (%1)"))
                      .arg(tempFile.errorString()),
                    nullptr);
              }
          } else
              QFile::remove(tempFileName);

          return callback(res.err, res.pass);
      });
}

// **************************************************************************
// createExportBundle
// **************************************************************************

QVariant PassesModel::createExportBundle(const QString& bundleId)
{
    QVariantMap result;

    if (!mItemMap.count(bundleId)) {
        result.insert("error", C::gettext("Failed to export pass bundle (pass unknown)"));
        return result;
    }

    auto bundlePass = mItemMap[bundleId];

    QString targetPath = passesDir.path() + "/" + bundlePass->bundleName + ".pkpasses";

    int n = 0;

    while (QFile::exists(targetPath) && ++n < 100) {
        targetPath = passesDir.path() + "/" + bundlePass->bundleName + "_" + QString::number(n)
                     + ".pkpasses";
    }

    if (QFile::exists(targetPath)) {
        result.insert("error",
                      C::gettext("Failed to export pass bundle (cant create bundle file)"));
        return result;
    }

    QuaZip archive(targetPath);

    bool res = archive.open(QuaZip::mdCreate);

    if (!res) {
        result.insert("error",
                      C::gettext("Failed to export pass bundle (failed to create archive))"));
        return result;
    }

    for (auto pass : bundlePass->bundlePasses) {
        QFileInfo fileInfo(pass->filePath);
        QString passFileName
          = fileInfo.fileName().remove("BUNDLE_" + bundlePass->bundleName + "_BUNDLE_");

        QuaZipNewInfo info(passFileName);
        info.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner
                            | QFileDevice::WriteUser | QFileDevice::ReadUser
                            | QFileDevice::ReadGroup | QFileDevice::ReadOther);

        QuaZipFile zipFile(&archive);

        res = zipFile.open(QIODevice::WriteOnly, info);

        if (!res) {
            archive.close();
            QFile::remove(targetPath);
            result.insert(
              "error", C::gettext("Failed to export pass bundle (failed to add file to archive))"));
            return result;
        }

        QFile inputFile(pass->filePath);

        res = inputFile.open(QIODevice::ReadOnly);

        if (!res) {
            zipFile.close();
            archive.close();
            QFile::remove(targetPath);
            result.insert("error",
                          C::gettext("Failed to export pass bundle (failed to read input file))"));
            return result;
        }

        auto contents = inputFile.readAll();
        auto written = zipFile.write(contents);

        inputFile.close();
        zipFile.close();

        if (written < 0 || written < contents.size()) {
            archive.close();
            QFile::remove(targetPath);
            result.insert("error",
                          C::gettext("Failed to export pass bundle (failed to read input file))"));
            return result;
        }
    }

    archive.close();
    result.insert("filePath", targetPath);

    return result;
}

} // namespace passes
