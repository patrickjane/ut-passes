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

      QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

      if (!dataPath.size())
         return;

      QDir dir(dataPath + "/passes");

      if (!dir.exists())
      {
         bool res = dir.mkpath(dataPath + "/passes");

         if (!res)
            return;
      }

      passesDir.setPath(dir.path());
      //passesDir.setPath("/home/phablet/passes");
      passesDir.setSorting(QDir::Time);
      storageReady = true;
   }

   // **************************************************************************
   // rowCount
   // **************************************************************************

   int PassesModel::rowCount(const QModelIndex &parent) const
   {
      if (parent.isValid())
         return 0;

      return mItems.size();
   }

   // **************************************************************************
   // data
   // **************************************************************************

   QVariant PassesModel::data(const QModelIndex &index, int role) const
   {
      if (!index.isValid())
         return QVariant();

      Pass* item = mItems[(index.row())];

      switch (role)
      {
         case FilePathRole:
            return QVariant(item->filePath);
            //      case PathRole:
            //         return QVariant(item.path);
            //      case PreviewPathRole:
            //         return QVariant(item.previewPath);
            //      case ImageCountRole:
            //         return QVariant(item.imageCount);
      }

      return QVariant();
   }

   // **************************************************************************
   // roleNames
   // **************************************************************************

   QHash<int, QByteArray> PassesModel::roleNames() const
   {
      QHash<int, QByteArray> names;
      names[FilePathRole] = "filePath";
      //   names[PathRole] = "path";
      //   names[PreviewPathRole] = "previewPath";
      //   names[ImageCountRole] = "imageCount";
      return names;
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

      guiPasses.clear();

      for (auto& pass : mItems)
         guiPasses << static_cast<QVariant>(*pass);

      emit passesChanged();
   }

   // **************************************************************************
   // importPass
   // **************************************************************************

   void PassesModel::importPass(const QString& filePath)
   {
      if (!storageReady)
      {
         qDebug() << "Storage directory not initialized";
         return;
      }

      if (!QFile::exists(filePath))
      {
         emit error(C::gettext("File path of the pass seems to be invalid)"));
         return;
      }

      QString md5 = fileMd5(filePath);

      if (md5.isEmpty())
      {
         emit error(C::gettext("Failed to create MD5 hash of pass"));
         return;
      }

      QString targetPath = passesDir.path() + "/" + md5 + ".pkpass";

      qDebug() << "Importing pass: " << filePath << " To: " << targetPath;

      if (QFile::exists(targetPath))
      {
         emit error(C::gettext("Same pass has already been imported"));
         return;
      }

      QFile sourceFile(filePath);
      bool res = sourceFile.copy(targetPath);

      if (!res)
      {
         emit error(QString(C::gettext("Failed to import pass into storage directory ")) + "(" + sourceFile.errorString() + ")");
         return;
      }

      auto pass = pkpass.openPass(targetPath);

      pass->id = md5;
      pass->modified = QDateTime::currentDateTime();

      if (!pass)
         QFile::remove(targetPath);

      //   auto exists = std::find_if(mItems.begin(), mItems.end(), [&name](const PassItem& x) { return x.name == name; });

      //   if (exists != mItems.end())
      //      return tr("Album already exists");

      //   QDir rootDir(mRootPath);

      //   bool res = rootDir.mkdir(name);

      //   if (!res)
      //      return tr("Failed to create album");

      //   beginResetModel();

      //   mItems.push_back({ name, mRootPath + "/" + name, "", 0 });

      //   std::sort(mItems.begin(), mItems.end(), [](const PassItem& a, const PassItem& b) -> bool { return a.name.toStdString() < b.name.toStdString(); });

      //   endResetModel();
   }

//   // **************************************************************************
//   // importPass
//   // **************************************************************************

//   QVariantList PassesModel::getPasses()
//   {
//      QVariantList res;

//      for (auto& pass : mItems)
//         res << static_cast<QVariant>(*pass);

//      return res;
//   }

} // namespace passes
