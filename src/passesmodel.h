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

#ifndef PASSESMODEL_H
#define PASSESMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include <QDir>

#include "pkpass.h"

// **************************************************************************
// class PassesModel
// **************************************************************************

namespace passes
{
   class PassesModel : public QAbstractListModel
   {
         Q_OBJECT
         Q_PROPERTY(QVariantList passes READ getPasses NOTIFY passesChanged)

      public:
         explicit PassesModel(QObject *parent = nullptr);

         enum
         {
            FilePathRole = Qt::UserRole
         };

         // Basic functionality

         int rowCount(const QModelIndex &parent = QModelIndex()) const override;
         QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
         virtual QHash<int, QByteArray> roleNames() const override;

         // QML interaction

         Q_INVOKABLE void reload();
         Q_INVOKABLE void importPass(const QString& filePath);

         QVariantList getPasses() { return guiPasses; }

         static PassesModel* getInstace() { return instance; }
         Pass* getPass(QString id) { return mItemMap.count(id) ? mItemMap[id] : nullptr; }

      signals:
         void error(QString error);
         void passError(QString error);
         void passesChanged();

      private:
         static PassesModel* instance;

         bool storageReady;
         Pkpass pkpass;

         std::vector<Pass*> mItems;
         std::map<QString,Pass*> mItemMap;
         QVariantList guiPasses;
         QDir passesDir;
   };

} // namespace passes

#endif // PASSESMODEL_H
