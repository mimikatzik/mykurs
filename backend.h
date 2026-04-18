#pragma once
#include <QObject>
#include <QStringList>
#include <QVariantList>

class Backend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList findings READ findings NOTIFY findingsChanged)
    Q_PROPERTY(bool hasScanRun READ hasScanRun NOTIFY hasScanRunChanged)

public:
    explicit Backend(QObject *parent = nullptr);

    Q_INVOKABLE void scanFiles(const QVariantList &paths);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void copyToClipboard(const QString &text); // Добавили метод
    Q_INVOKABLE void applyAllFixes();

    QVariantList findings() const;
    bool hasScanRun() const;

signals:
    void findingsChanged();
    void hasScanRunChanged();

private:
    QVariantList m_findings;
    bool m_hasScanRun { false };

    void processFile(const QString &path);
    void buildSuggestion(const QString &line,
                         const QString &funcName,
                         QString &fixedLine,
                         QString &warningText,
                         QString &recommendationText) const;
};
