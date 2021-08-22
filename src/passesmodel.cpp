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

      connect(&pkpass, &passes::Pkpass::error, this, [this](QString text)
      {
         emit passError(text);
      });

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

      for (const QFileInfo& info : passesDir.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Readable))
      {
         if (info.fileName().startsWith(".") || !info.fileName().endsWith(".pkpass"))
            continue;

         auto pass = pkpass.openPass(info.absoluteFilePath());

         pass->id = info.baseName();
         pass->modified = info.lastModified();

         mItemMap[pass->id] = pass;

         if (pass)
            mItems.push_back(pass);
      }

      endResetModel();
      emit countChanged();
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
         qDebug() << "Storage directory not initialized";
         return false;
      }

      if (!QFile::exists(fp))
      {
         emit error(C::gettext("File path of the pass seems to be invalid"));
         return false;
      }

      QString md5 = fileMd5(fp);

      if (md5.isEmpty())
      {
         emit error(C::gettext("Failed to create MD5 hash of pass"));
         return false;
      }

      QString targetPath = passesDir.path() + "/" + md5 + ".pkpass";

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
         emit error(QString(C::gettext("Failed to import pass into storage directory ")) + "(" + sourceFile.errorString() + ")");
         return false;
      }

      reload();
      return true;
   }

   // **************************************************************************
   // deletePass
   // **************************************************************************

   bool PassesModel::deletePass(QString id)
   {
      qDebug() << "Deleting pass " << id;

      QString passPath = passesDir.path() + "/" + id + ".pkpass";

      auto it = std::find_if(mItems.begin(), mItems.end(), [&id](Pass* pass){ return pass->id == id; });

      if (!mItemMap.count(id) || it == mItems.end() || !QFile::exists(passPath))
      {
         emit error(C::gettext("Failed to delete pass (pass unknown)"));
         return false;
      }

      auto index = it - mItems.begin();
      QFile passFile(passPath);
      bool res = passFile.remove();

      if (!res)
      {
         emit error(QString(C::gettext("Failed to delete pass from storage directory ")) + "(" + passFile.errorString() + ")");
         return false;
      }

      beginRemoveRows(QModelIndex(), index, index);

      mItemMap.erase(id);
      mItems.erase(it);

      endRemoveRows();
      emit countChanged();

//      guiPasses.clear();

//      for (auto& pass : mItems)
//         guiPasses << static_cast<QVariant>(*pass);

//      emit passesChanged();

//      qDebug() << mItemMap.count(id) << " " << mItemMap.size() << " " << mItems.size();

      return true;
   }

} // namespace passes
