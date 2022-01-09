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
#include <QFont>

#include "pkpass.h"

// **************************************************************************
// class PassesModel
// **************************************************************************

namespace passes
{
   class PassesModel : public QAbstractListModel
   {
         enum RoleNames
         {
            PassRole = Qt::UserRole + 1
         };

         Q_OBJECT
         Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
         Q_PROPERTY(QFont font READ getDefaultFont WRITE setDefaultFont)

      public:
         static PassesModel* getInstace() { return instance; }

      public:
         explicit PassesModel(QObject *parent = nullptr);

         // QAbstractListModel

         QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
         int rowCount(const QModelIndex &parent = QModelIndex()) const;
         QHash<int, QByteArray> roleNames() const;

         // QML interaction

         Q_INVOKABLE void init();
         Q_INVOKABLE void reload();
         Q_INVOKABLE bool importPass(const QString& filePath);
         Q_INVOKABLE bool deletePass(QString filePath);

         QFont getDefaultFont() { return defaultFont; }
         void setDefaultFont(QFont to) { defaultFont = to; pkpass.setDefaultFont(to); }

         Pass* getPass(QString id) { return mItemMap.count(id) ? mItemMap[id] : nullptr; }

      signals:
         void error(QString error);
         void countChanged();
         void failedPasses(QVariantList passes);

      private:
         QString getDataPath() const;

         static PassesModel* instance;

         bool storageReady;
         Pkpass pkpass;

         std::vector<Pass*> mItems;
         std::map<QString,Pass*> mItemMap;
         QDir passesDir;
         QFont defaultFont;
   };

} // namespace passes

#endif // PASSESMODEL_H
