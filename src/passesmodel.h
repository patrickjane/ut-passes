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

#ifndef PASSESMODEL_H
#define PASSESMODEL_H

#include <QAbstractListModel>
#include <QDir>
#include <QFont>
#include <QObject>

#include "network.h"
#include "pkpass.h"

// **************************************************************************
// class PassesModel
// **************************************************************************

namespace passes {
struct PassSorter {
    bool operator()(PassPtr a, PassPtr b) const
    {
        return a->sortingDate.toSecsSinceEpoch() > b->sortingDate.toSecsSinceEpoch();
    }
};

template <typename T>
using ResultCallback = std::function<void(PassResult)>;

class PassesModel : public QAbstractListModel {
    enum RoleNames { PassRole = Qt::UserRole + 1 };

    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(int countExpired READ getCountExpired NOTIFY countExpiredChanged)
    Q_PROPERTY(QFont defaultFont READ getDefaultFont WRITE setDefaultFont)

public:
    static PassesModel* getInstace()
    {
        return instance;
    }

public:
    explicit PassesModel(QObject* parent = nullptr);

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
    Q_INVOKABLE QString deleteFile(QString filePath);
    Q_INVOKABLE QString deletePass(QString id);

    Q_INVOKABLE QVariant createExportBundle(const QString& bundleId);

    QFont getDefaultFont()
    {
        return QFont();
    }
    void setDefaultFont(QFont to)
    {
        pkpass.setDefaultFont(to);
    }
    int getCountExpired()
    {
        return countExpired;
    }

    PassPtr getPass(QString id)
    {
        return mItemMap.count(id) ? mItemMap[id] : nullptr;
    }

signals:
    void countChanged();
    void countExpiredChanged();
    void passUpdatesFetched(QString error);
    void failedPasses(QVariantList passes);

private:
    void openPasses(bool openExired);
    void readPasses(QVariantList& failed, QMap<QString, PassList>& bundles, bool doShowExpired);
    void addBundlePasses(QMap<QString, PassList>& bundles, bool doShowExpired);

    void fetchPassUpdate(PassPtr pass, ResultCallback<PassPtr> callback);

    PassResult storePassUpdate(PassPtr pass, QByteArray data);

    bool isOpen(const QString& filePath);

    QString getDataPath() const;

    static PassesModel* instance;

    bool storageReady;
    int countExpired;
    Pkpass pkpass;
    PassSorter passSorter;

    PassList mItems;
    PassMap mItemMap;
    QDir passesDir;

    network::Network net;
};

} // namespace passes

#endif // PASSESMODEL_H
