#include "backend.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <QUrl>
#include <QGuiApplication>
#include <QClipboard>

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

void Backend::copyToClipboard(const QString &text)
{
    QGuiApplication::clipboard()->setText(text);
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

        processFile(path);
    }
    emit findingsChanged();

    if (!m_hasScanRun) {
        m_hasScanRun = true;
        emit hasScanRunChanged();
    }
}


void Backend::processFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    QStringList lines;
    while (!in.atEnd()) lines << in.readLine();

    const QStringList funcs { "gets", "strcpy", "sprintf" };
    const int contextRadius = 3;

    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines[i];

        for (const QString &funcName : funcs) {
            if (!line.contains(funcName + "(")) continue;

            QVariantList beforeLines;
            QVariantList afterLines;

            const int start = qMax(0, i - contextRadius);
            const int end = qMin(lines.size() - 1, i + contextRadius);

            // Теперь сохраняем и текст, и номер строки
            for (int j = start; j < i; ++j) {
                QVariantMap obj;
                obj["text"] = lines[j];
                obj["ln"] = j + 1;
                beforeLines << obj;
            }
            for (int j = i + 1; j <= end; ++j) {
                QVariantMap obj;
                obj["text"] = lines[j];
                obj["ln"] = j + 1;
                afterLines << obj;
            }

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
}

void Backend::buildSuggestion(const QString &line, const QString &funcName,
                              QString &fixedLine, QString &warningText,
                              QString &recommendationText) const
{
    QString indent;
    for (const QChar &c : line) {
        if (c.isSpace()) indent += c;
        else break;
    }

    // Регулярка выцепляет всё, что между первой ( и последней )
    QRegularExpression re(funcName + "\\s*\\((.*)\\)");
    QRegularExpressionMatch match = re.match(line);
    QString args = match.hasMatch() ? match.captured(1).trimmed() : "";

    if (funcName == "gets") {
        warningText = tr("Функция gets небезопасна: возможно переполнение буфера.");
        recommendationText = tr("Используйте std::string и std::getline.");

        // Убираем _str, оставляем чистое имя
        QString varName = args.isEmpty() ? "str" : args;
        fixedLine = indent + QString("std::string %1; std::getline(std::cin, %1);").arg(varName);

    } else if (funcName == "strcpy") {
        warningText = tr("Функция strcpy небезопасна: нет контроля границ.");
        recommendationText = tr("Используйте std::string или оператор присваивания.");

        // Используем поиск ПЕРВОЙ запятой, чтобы не ломать строки с запятыми внутри
        int firstComma = args.indexOf(',');
        if (firstComma != -1) {
            QString dest = args.left(firstComma).trimmed();
            QString src = args.mid(firstComma + 1).trimmed();
            fixedLine = indent + QString("%1 = %2;").arg(dest, src);
        } else {
            fixedLine = indent + "dest = src;";
        }

    } else if (funcName == "sprintf") {
        warningText = tr("Функция sprintf небезопасна: риск переполнения буфера.");
        recommendationText = tr("Используйте std::format (C++20).");

        int firstComma = args.indexOf(',');
        if (firstComma != -1) {
            QString buffer = args.left(firstComma).trimmed();
            QString rest = args.mid(firstComma + 1).trimmed();
            // rest теперь содержит и строку формата, и все аргументы целиком
            fixedLine = indent + QString("std::string %1_s = std::format(%2);").arg(buffer, rest);
        } else {
            fixedLine = indent + "std::string s = std::format(...);";
        }
    }
}