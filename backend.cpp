#include "backend.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <QUrl>

Backend::Backend(QObject *parent)
    : QObject(parent)
{
}

void Backend::clear()
{
    qDebug() << "[Backend] clear findings";
    m_findings.clear();
    emit findingsChanged();

    if (m_hasScanRun) {
        m_hasScanRun = false;
        emit hasScanRunChanged();
    }
}

QVariantList Backend::findings() const
{
    return m_findings;
}

bool Backend::hasScanRun() const
{
    return m_hasScanRun;
}

void Backend::scanFiles(const QVariantList &paths)
{
    qDebug() << "[Backend] scanFiles called with" << paths.size() << "paths";

    clear();

    for (const QVariant &v : paths) {
        const QString raw = v.toString();
        QString path = raw;
        if (raw.startsWith("file:"))
            path = QUrl(raw).toLocalFile();

        qDebug() << "[Backend] processing path" << path;
        processFile(path);  // не вызываем findingsChanged здесь
    }

    qDebug() << "[Backend] total findings after scan:" << m_findings.size();

    emit findingsChanged();  // один вызов после всех файлов

    if (!m_hasScanRun) {
        m_hasScanRun = true;
        emit hasScanRunChanged();
    }
}


void Backend::processFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[Backend] cannot open file" << path;
        return;
    }

    QTextStream in(&file);
    QStringList lines;
    while (!in.atEnd())
        lines << in.readLine();

    const QStringList funcs { QStringLiteral("gets"), QStringLiteral("strcpy"), QStringLiteral("sprintf") };
    const int contextRadius = 3;

    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines[i];

        for (const QString &funcName : funcs) {
            if (!line.contains(funcName + QLatin1Char('(')))
                continue;

            qDebug() << "[Backend] found" << funcName << "in" << path << "line" << (i + 1);

            // Контекст вокруг найденной строки
            QStringList beforeLinesList;
            QStringList afterLinesList;

            const int start = qMax(0, i - contextRadius);
            const int end = qMin(lines.size() - 1, i + contextRadius);

            for (int j = start; j < i; ++j)
                beforeLinesList << lines[j];
            for (int j = i + 1; j <= end; ++j)
                afterLinesList << lines[j];

            // Конвертируем QStringList в QVariantList для QML
            QVariantList beforeLines;
            QVariantList afterLines;
            for (const QString &str : beforeLinesList)
                beforeLines << str;
            for (const QString &str : afterLinesList)
                afterLines << str;

            QString fixedLine;
            QString warning;
            QString recommendation;
            buildSuggestion(line, funcName, fixedLine, warning, recommendation);

            QVariantMap finding;
            finding["fileName"] = QFileInfo(path).fileName();
            finding["lineNumber"] = i + 1;
            finding["functionName"] = funcName;
            finding["beforeLines"] = beforeLines;
            finding["codeLine"] = line;
            finding["afterLines"] = afterLines;
            finding["fixedLine"] = fixedLine;
            finding["warning"] = warning;
            finding["recommendation"] = recommendation;

            m_findings.append(finding);
        }
    }
    // Не эмитим здесь — эмитим только после обработки всех файлов в scanFiles()
}

void Backend::buildSuggestion(const QString &line,
                              const QString &funcName,
                              QString &fixedLine,
                              QString &warningText,
                              QString &recommendationText) const
{
    Q_UNUSED(line);

    if (funcName == "gets") {
        warningText = tr("Функция gets небезопасна: возможно переполнение буфера (нет контроля длины ввода).");
        recommendationText = tr("Используйте std::string и std::getline(std::cin, ...) вместо сырого буфера.");
        fixedLine = tr("std::string line; std::getline(std::cin, line);");
        return;
    }

    if (funcName == "strcpy") {
        warningText = tr("Функция strcpy небезопасна: не контролируется размер копируемых данных.");
        recommendationText = tr("Используйте std::string и его методы (assign/оператор =) вместо прямого копирования в массив char.");
        fixedLine = tr("std::string dest = src;    // вместо strcpy(dest, src);");
        return;
    }

    if (funcName == "sprintf") {
        warningText = tr("Функция sprintf небезопасна: возможна запись за пределами буфера форматируемой строки.");
        recommendationText = tr("Используйте библиотеку fmt и fmt::format, возвращающую std::string, либо остальные безопасные аналоги.");
        fixedLine = tr("std::string result = fmt::format(\"%s %d\", arg1, arg2);");
        return;
    }

    warningText.clear();
    recommendationText.clear();
    fixedLine.clear();
}
