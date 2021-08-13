#include <QGuiApplication>
#include <QCoreApplication>
#include <QUrl>
#include <QString>
#include <QQuickView>

#include "src/passesmodel.h"
#include "src/passimageprovider.h"

int main(int argc, char *argv[])
{
    QGuiApplication *app = new QGuiApplication(argc, (char**)argv);
    app->setApplicationName("passes.s710");

    qmlRegisterType<passes::PassesModel>("PassesModel", 1, 0, "PassesModel");

    QQuickView *view = new QQuickView();
    QQmlEngine *engine = view->engine();
    engine->addImageProvider(QLatin1String("passes"), new passes::PassImageProvider());

    view->setSource(QUrl("qrc:/Main.qml"));
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->show();

    return app->exec();
}
