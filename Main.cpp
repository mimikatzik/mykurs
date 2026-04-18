#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "backend.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    Backend backend;
    engine.rootContext()->setContextProperty("backend", &backend);

    // Загружаем QML-тип Main из модуля "kurs"
    engine.loadFromModule("kurs", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
