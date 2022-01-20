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
#include "network.h"

// **************************************************************************
// class PassesModel
// **************************************************************************

namespace passes
{
   struct PassSorter
   {
         bool operator()(Pass* a, Pass* b) const
         {
            return a->sortingDate.toSecsSinceEpoch() > b->sortingDate.toSecsSinceEpoch();
         }
   };

   template<typename T>
   using ResultCallback = std::function<void(QString, T)>;

   class PassesModel : public QAbstractListModel
   {
         enum RoleNames
         {
            PassRole = Qt::UserRole + 1
         };

         Q_OBJECT
         Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
         Q_PROPERTY(int countExpired READ getCountExpired NOTIFY countExpiredChanged)
         Q_PROPERTY(QFont defaultFont READ getDefaultFont WRITE setDefaultFont)

      public:
         static PassesModel* getInstace() { return instance; }

      public:
         explicit PassesModel(QObject *parent = nullptr);

         // QAbstractListModel

         QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
         int rowCount(const QModelIndex& parent = QModelIndex()) const;
         QHash<int, QByteArray> roleNames() const;

         // QML interaction

         Q_INVOKABLE QString init();
         Q_INVOKABLE void reload();

         Q_INVOKABLE void fetchPassUpdates();

         Q_INVOKABLE void showExpired();
         Q_INVOKABLE void hideExpired();

         Q_INVOKABLE QString importPass(const QString& filePath, bool expiredShown);
         Q_INVOKABLE QString deletePass(QString filePath, bool failedPass);

         QFont getDefaultFont() { return QFont(); }
         void setDefaultFont(QFont to) { pkpass.setDefaultFont(to); }
         int getCountExpired() { return countExpired; }

         Pass* getPass(QString id) { return mItemMap.count(id) ? mItemMap[id] : nullptr; }

      signals:
         void countChanged();
         void countExpiredChanged();
         void passUpdatesFetched(QString error);
         void failedPasses(QVariantList passes);

      private:
         void openPasses(bool openExired);
         void fetchPassUpdate(Pass* pass, ResultCallback<Pass*> callback);

         PassResult storePassUpdate(Pass* pass, QByteArray data);

         bool isOpen(const QString& filePath);

         QString getDataPath() const;

         static PassesModel* instance;

         bool storageReady;
         int countExpired;
         Pkpass pkpass;
         PassSorter passSorter;

         std::vector<Pass*> mItems;
         std::map<QString,Pass*> mItemMap;
         QDir passesDir;

         network::Network net;
   };

} // namespace passes

#endif // PASSESMODEL_H
