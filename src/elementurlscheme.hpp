#pragma once

#include <QWebEngineUrlSchemeHandler>

class ElementUrlScheme : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT

public:
    ElementUrlScheme(const QString &root, QObject *parent = nullptr);

    static const QByteArray schemeName()
    {
        return "element";
    }

    void requestStarted(QWebEngineUrlRequestJob *request) override;

private:
    QString root;

    static const QString getFilePath(const QUrl &url);
    static const QByteArray mimeType(const QString &path);
};
