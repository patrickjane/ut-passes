// **************************************************************************
// class PassesModel
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

#include "passesmodel.h"
#include <QStandardPaths>
#include <QDebug>
#include <QCryptographicHash>

namespace C {
#include <libintl.h>
}

namespace passes
{
   QByteArray fileMd5(const QString &fileName)
   {
      QFile f(fileName);

      if (f.open(QFile::ReadOnly))
      {
         QCryptographicHash hash(QCryptographicHash::Md5);

         if (hash.addData(&f))
            return hash.result().toHex();
      }

      return QByteArray();
   }

   // **************************************************************************
   // class PassesModel
   // **************************************************************************

   PassesModel* PassesModel::instance = nullptr;

   PassesModel::PassesModel(QObject *parent)
      : QAbstractListModel(parent), storageReady(false)
   {
      instance = this;
   }

   // **************************************************************************
   // init
   // **************************************************************************

   void PassesModel::init()
   {
      QString dataPath = getDataPath();

      if (!dataPath.size())
         return;

      qDebug() << "DATA PATH: " << dataPath;

      QDir dir(dataPath + "/passes");

      if (!dir.exists())
      {
         bool res = dir.mkpath(dataPath + "/passes");

         if (!res)
            return;
      }

      passesDir.setPath(dir.path());
      passesDir.setPath("/home/phablet/passes");
      passesDir.setSorting(QDir::Time);
      storageReady = true;
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
   // reload
   // **************************************************************************

   void PassesModel::reload()
   {
      if (!storageReady || !passesDir.path().size())
      {
         qDebug() << "Storage directory not initialized";
         return;
      }

      beginResetModel();

      mItems.clear();
      mItemMap.clear();

      QVariantList failed;

      for (const QFileInfo& info : passesDir.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Readable))
      {
         if (info.fileName().startsWith(".") || !info.fileName().endsWith(".pkpass"))
            continue;

         auto passResult = pkpass.openPass(info.absoluteFilePath());

         passResult.pass->id = fileMd5(info.absoluteFilePath());
         passResult.pass->modified = info.lastModified();
         passResult.pass->filePath = info.absoluteFilePath();

         if (!passResult.err.isEmpty())
         {
            qDebug() << "Pass open failed: " << passResult.err;

            QVariantMap failedPass;
            failedPass["filePath"] = passResult.pass->filePath;
            failedPass["error"] = passResult.err;
            failed.append(failedPass);
         }
         else
         {
            mItemMap[passResult.pass->id] = passResult.pass;
            mItems.push_back(passResult.pass);
         }
      }

      endResetModel();
      emit countChanged();

      if (failed.length())
      {
         qDebug() << failed.length() << " passed failed to open";
         emit failedPasses(failed);
      }
   }

   // **************************************************************************
   // importPass
   // **************************************************************************

   bool PassesModel::importPass(const QString& filePath)
   {
      QString fp = filePath;

      if (fp.startsWith("file://"))
         fp.remove("file://");

      if (!storageReady)
      {
         emit error(QString(C::gettext("Storage directory inaccessible, cannot import pass")) + " (" + passesDir.absolutePath() + ")");
         return false;
      }

      if (!QFile::exists(fp))
      {
         emit error(C::gettext("File path of the pass seems to be invalid"));
         return false;
      }

      QFileInfo info(fp);
      QString targetPath = passesDir.path() + "/" + info.fileName() + ".pkpass";

      qDebug() << "Importing pass: " << fp << " To: " << targetPath;

      if (QFile::exists(targetPath))
      {
         emit error(C::gettext("Same pass has already been imported"));
         return false;
      }

      QFile sourceFile(fp);
      bool res = sourceFile.copy(targetPath);

      if (!res)
      {
         emit error(QString(C::gettext("Failed to import pass into storage directory ")) + " (" + sourceFile.errorString() + ")");
         return false;
      }

      reload();
      return true;
   }

   // **************************************************************************
   // deletePass
   // **************************************************************************

   bool PassesModel::deletePass(QString filePath)
   {
      qDebug() << "Deleting pass " << filePath;

      auto it = std::find_if(mItems.begin(), mItems.end(), [&filePath](Pass* pass){ return pass->filePath == filePath; });

      if (it == mItems.end() || !QFile::exists(filePath) || !mItemMap.count((*it)->id))
      {
         emit error(C::gettext("Failed to delete pass (pass unknown)"));
         return false;
      }

      auto pass = *it;
      auto index = it - mItems.begin();
      QFile passFile(filePath);
      bool res = passFile.remove();

      if (!res)
      {
         emit error(QString(C::gettext("Failed to delete pass from storage directory ")) + " (" + passFile.errorString() + ")");
         return false;
      }

      beginRemoveRows(QModelIndex(), index, index);

      mItemMap.erase(pass->id);
      mItems.erase(it);
      delete pass;

      endRemoveRows();
      emit countChanged();
      return true;
   }

} // namespace passes
