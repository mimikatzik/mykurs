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

            // Используем регулярку для точного поиска имени функции
            QRegularExpression re("\\b" + funcName + "\\s*\\(");
            if (!line.contains(re)) continue;

            QVariantList beforeLines;
            QVariantList afterLines;

            const int start = qMax(0, i - contextRadius);
            const int end = qMin(lines.size() - 1, i + contextRadius);

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
            finding["fullPath"] = path;
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

    // Регулярка для захвата содержимого скобок
    QRegularExpression re(funcName + "\\s*\\((.*)\\)");
    QRegularExpressionMatch match = re.match(line);
    QString args = match.hasMatch() ? match.captured(1).trimmed() : "";

    // ПРОВЕРКА: Если это объявление функции (есть типы данных), пропускаем
    if (args.contains(QRegularExpression("\\b(char|int|void|unsigned|const)\\b"))) {
        fixedLine = "";
        return;
    }

    if (funcName == "gets") {
        warningText = tr("Функция gets() небезопасна: невозможно контролировать размер буфера.");
        recommendationText = tr("Рекомендуется использовать std::getline для безопасного чтения строк.");

        if (args.isEmpty() || args.contains('*')) {
            fixedLine = "";
        } else {
            fixedLine = indent + QString("std::getline(std::cin, %1);").arg(args);
        }

    } else if (funcName == "strcpy") {
        warningText = tr("Функция strcpy() небезопасна: не проверяет границы приемника.");
        recommendationText = tr("Рекомендуется использовать std::string или метод .assign().");

        int firstComma = args.indexOf(',');
        if (firstComma != -1) {
            QString dest = args.left(firstComma).trimmed();
            QString src = args.mid(firstComma + 1).trimmed();

            if (!dest.contains('*') && !dest.contains(' ')) {
                fixedLine = indent + QString("%1 = %2;").arg(dest, src);
            } else {
                fixedLine = "";
            }
        } else {
            fixedLine = "";
        }

    } else if (funcName == "sprintf") {
        warningText = tr("Функция sprintf() небезопасна: риск переполнения буфера.");
        recommendationText = tr("Рекомендуется использовать fmt::format для безопасного форматирования.");

        int firstComma = args.indexOf(',');
        if (firstComma != -1) {
            QString buffer = args.left(firstComma).trimmed();
            QString rest = args.mid(firstComma + 1).trimmed();

            if (!buffer.contains('*') && !buffer.contains(' ')) {
                fixedLine = indent + QString("%1 = fmt::format(%2);").arg(buffer, rest);
            } else {
                fixedLine = "";
            }
        } else {
            fixedLine = "";
        }
    }
}

void Backend::applyAllFixes()
{
    if (m_findings.isEmpty()) return;

    // 1. Собираем все правки и группируем по файлам
    QMap<QString, QList<QPair<int, QString>>> changesByFile;

    for (const QVariant &v : m_findings) {
        QVariantMap finding = v.toMap();
        QString fullPath = finding["fullPath"].toString();
        int lineIdx = finding["lineNumber"].toInt() - 1;
        QString fixed = finding["fixedLine"].toString();

        if (!fixed.isEmpty()) {
            changesByFile[fullPath].append({lineIdx, fixed});
        }
    }

    // 2. Применяем правки для каждого файла
    for (auto it = changesByFile.begin(); it != changesByFile.end(); ++it) {
        QString path = it.key();
        QFile file(path);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

        QStringList lines;
        QTextStream in(&file);
        while (!in.atEnd()) lines << in.readLine();
        file.close();

        // Сортируем правки по номеру строки В ОБРАТНОМ ПОРЯДКЕ (от большего к меньшему)
        // Это критично, чтобы не поплыли индексы, если мы будем добавлять/удалять строки
        auto &fileChanges = it.value();
        std::sort(fileChanges.begin(), fileChanges.end(), [](const QPair<int, QString> &a, const QPair<int, QString> &b) {
            return a.first > b.first;
        });

        for (const auto &change : fileChanges) {
            if (change.first >= 0 && change.first < lines.size()) {
                lines[change.first] = change.second;
            }
        }

        // Записываем обновленный контент обратно
        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&file);
            for (int i = 0; i < lines.size(); ++i) {
                out << lines[i] << (i == lines.size() - 1 ? "" : "\n");
            }
            file.close();
        }
    }

    // 3. Очищаем результаты и уведомляем интерфейс
    qDebug() << "Все исправления применены успешно!";
    m_findings.clear();
    emit findingsChanged();

    // Сбрасываем флаг сканирования, чтобы UI предложил просканировать заново
    m_hasScanRun = false;
    emit hasScanRunChanged();
}