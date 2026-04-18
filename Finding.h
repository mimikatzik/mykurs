#pragma once
#include <QString>
#include <QStringList>

struct Finding {
    QString fileName;
    int lineNumber;
    QString html;
    QString warningText;
};
