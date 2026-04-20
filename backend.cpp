#include "backend.h" // заголовочный файл
#include <QFile> // класс для работы с файлами
#include <QFileInfo> // класс для получения метаданных о файле
#include <QTextStream> // поток для чтения и записи текстовых данных
#include <QRegularExpression> // класс для работы с регулярными выражениями
#include <QDebug> // инструмент для отладки
#include <QUrl> // класс для представления URL
#include <QGuiApplication> // основной класс приложения без виджетов
#include <QClipboard> // класс для работы с буфером обмена

//базовая инициализация объекта Backend, встроенного в систему сигналов и слотов
Backend::Backend(QObject *parent)
    : QObject(parent) // указатель на базовый класс QObject, от которого наследуются все классы
{
}

//сброс состояния анализа
void Backend::clear()
{
    qDebug() << "[Backend] clear findings"; // отладочный вызов
    m_findings.clear(); // очистка контейнера m_findings
    emit findingsChanged(); // уведомление компонентов Qt об изменении данных

    // проверка на то, выполнялся ли ранее анализ
    if (m_hasScanRun) {
        m_hasScanRun = false; // изменение состояния
        emit hasScanRunChanged();
    }
}

// геттер для возврата m_findings
QVariantList Backend::findings() const
{
    return m_findings;
}

// геттер для проверки запуска анализа ранее
bool Backend::hasScanRun() const
{
    return m_hasScanRun;
}

// запись строки в буфер обмена
void Backend::copyToClipboard(const QString &text)
{
    QGuiApplication::clipboard()->setText(text);
}

// обработка входных данных (списка файлов)
void Backend::scanFiles(const QVariantList &paths)
{
    qDebug() << "[Backend] scanFiles called with" << paths.size() << "paths";

    clear(); // очистка предыдущих результатов

    // проход по списку путей к файлам
    for (const QVariant &v : paths) {
        const QString raw = v.toString(); // преобразование файла в строку
        QString path = raw; // изменяемая копия строки
        if (raw.startsWith("file:")) // если строка является URL-ом
            path = QUrl(raw).toLocalFile(); // преобразование в путь

        processFile(path); // переход к обработке файла
    }
    emit findingsChanged();

    // проверка на запуск файла ранее для предотвращения лишних обновлений интерфейса
    if (!m_hasScanRun) {
        m_hasScanRun = true;
        emit hasScanRunChanged();
    }
}

// анализ файла
void Backend::processFile(const QString &path)
{
    QFile file(path); // создание объекта файла по пути path
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return; // проверка на возможность открытия файла

    QTextStream in(&file); // создание текстового потока
    QStringList lines; // список строк
    while (!in.atEnd()) lines << in.readLine(); // чтение файла доконца и запись в список строк

    const QStringList funcs { "gets", "strcpy", "sprintf" }; // список небезопасных функций
    const int contextRadius = 3; // радиус контекста (строки до и после)

    // основной цикл анализа
    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines[i]; // получение текущей строки по ссылке

        // перебор небезопасных функций
        for (const QString &funcName : funcs) {

            QRegularExpression re("\\b" + funcName + "\\s*\\("); // регулярное выражение для точного поиска имени функции
            if (!line.contains(re)) continue; // переход к следующей функции при несовпадении

            QVariantList beforeLines; // контекст до
            QVariantList afterLines; // контекст после

            const int start = qMax(0, i - contextRadius); // начальный индекс контектста
            const int end = qMin(lines.size() - 1, i + contextRadius); // конечный индек

            // проход по строкам перед текущей
            for (int j = start; j < i; ++j) {
                QVariantMap obj; // ассоциативный контейнер (ключ-значение)
                obj["text"] = lines[j]; // текст строки
                obj["ln"] = j + 1; // номер строки
                beforeLines << obj; // запись в контейнер строк до
            }
            // проход по строкам после текущей
            for (int j = i + 1; j <= end; ++j) {
                QVariantMap obj;
                obj["text"] = lines[j];
                obj["ln"] = j + 1;
                afterLines << obj;
            }

            QString fixedLine; // исправленная строка
            QString warning; // предупреждение
            QString recommendation; // рекомендация
            buildSuggestion(line, funcName, fixedLine, warning, recommendation); // вызов функции для интерпретации

            QVariantMap finding; // единица результата анализа
            finding["fullPath"] = path; // полный путь к файлу
            finding["fileName"] = QFileInfo(path).fileName(); // только имя файла
            finding["lineNumber"] = i + 1; // номер строки
            finding["functionName"] = funcName; // имя найденной функции
            finding["beforeLines"] = beforeLines; // контекст до
            finding["codeLine"] = line; // строка с уязвимостью
            finding["afterLines"] = afterLines; // контекст после
            finding["fixedLine"] = fixedLine; // исправленная строка
            finding["warning"] = warning; // описание проблемы
            finding["recommendation"] = recommendation; // рекомендации по исправлению

            m_findings.append(finding); // добавление результата в общий список
        }
    }
}

// интерпретация детектора
void Backend::buildSuggestion(const QString &line, const QString &funcName,
                              QString &fixedLine, QString &warningText,
                              QString &recommendationText) const
{
    QString indent; // отступ
    // вычисление отступа
    for (const QChar &c : line) {
        if (c.isSpace()) indent += c; // если пробел, то добавить
        else break;
    }

    QRegularExpression re(funcName + "\\s*\\((.*)\\)"); // регулярное выражение для захвата содержимого скобок
    QRegularExpressionMatch match = re.match(line); // сопоставление шаблона со строкой
    QString args = match.hasMatch() ? match.captured(1).trimmed() : ""; // захват содержимого скобок и удаление пробелов по краям

    // если это объявление функции (есть типы данных), пропускаем
    if (args.contains(QRegularExpression("\\b(char|int|void|unsigned|const)\\b"))) {
        fixedLine = "";
        return;
    }

    // проверка на gets()
    if (funcName == "gets") {
        warningText = tr("Функция gets() небезопасна: невозможно контролировать размер буфера.");
        recommendationText = tr("Рекомендуется использовать std::getline для безопасного чтения строк.");

        // защитная эвристика: если аргументы не распарсились или содержат указатель, то отказ от исправления
        if (args.isEmpty() || args.contains('*')) {
            fixedLine = "";
        } else {
            fixedLine = indent + QString("std::getline(std::cin, %1);").arg(args); // формирование строки с подставлением аргументов
        }

    // аналогично для strcpy()
    } else if (funcName == "strcpy") {
        warningText = tr("Функция strcpy() небезопасна: не проверяет границы приемника.");
        recommendationText = tr("Рекомендуется использовать std::string или метод .assign().");

        int firstComma = args.indexOf(','); // разделение по запятой
        if (firstComma != -1) { // если запятая найдена
            QString dest = args.left(firstComma).trimmed(); // левая часть
            QString src = args.mid(firstComma + 1).trimmed(); // правая часть

            if (!dest.contains('*') && !dest.contains(' ')) {
                fixedLine = indent + QString("%1 = %2;").arg(dest, src);
            } else {
                fixedLine = "";
            }
        } else {
            fixedLine = "";
        }

    // аналогично для sprintf()
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

// автоисправление переданного файла
void Backend::applyAllFixes()
{
    if (m_findings.isEmpty()) return; // проверка списка результатов на пустоту

    QMap<QString, QList<QPair<int, QString>>> changesByFile; // агрегация всех правок, где ключ - это путь к файлу, а значение - список изменений

    // проход по найденным уязвимостям
    for (const QVariant &v : m_findings) {
        QVariantMap finding = v.toMap(); // преобразование в словарь ключ-значение
        QString fullPath = finding["fullPath"].toString(); // извлечение пути к файлу
        int lineIdx = finding["lineNumber"].toInt() - 1; // получение номера строки и перевод его в индекс
        QString fixed = finding["fixedLine"].toString(); // предложенная замена

        // фильтр только тех случаев, где есть что применять
        if (!fixed.isEmpty()) {
            changesByFile[fullPath].append({lineIdx, fixed}); // аккумуляция изменений для одного файла в одном месте
        }
    }

    // применение правки для каждого файла
    for (auto it = changesByFile.begin(); it != changesByFile.end(); ++it) {
        QString path = it.key(); // извлечение пути к текущему файлу
        QFile file(path); // объект файла

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue; // проверка на возможность открыть файл

        QStringList lines; // список строк
        QTextStream in(&file); // создание потока для чтения
        while (!in.atEnd()) lines << in.readLine(); // чтение доконца
        file.close(); // закрытие файла

        // сортировка правок по номеру строки в обратом порядке
        auto &fileChanges = it.value(); // ссылка на список изменений для текущего файла
        std::sort(fileChanges.begin(), fileChanges.end(), [](const QPair<int, QString> &a, const QPair<int, QString> &b) {
            return a.first > b.first; // сортировка
        });

        // применение изменений
        for (const auto &change : fileChanges) {
            // гарантия что индекс не переходит границы
            if (change.first >= 0 && change.first < lines.size()) {
                lines[change.first] = change.second;
            }
        }

        // запись обновленного контента обратно
        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&file); // создание потока для записи
            for (int i = 0; i < lines.size(); ++i) {
                out << lines[i] << (i == lines.size() - 1 ? "" : "\n"); // запись строк
            }
            file.close(); // закрытие файла
        }
    }

    // очистка результатов и уведомление интерфейса
    qDebug() << "Все исправления применены успешно!";
    m_findings.clear();
    emit findingsChanged();

    // сброс флага сканирования, чтобы UI предложил просканировать заново
    m_hasScanRun = false;
    emit hasScanRunChanged();
}