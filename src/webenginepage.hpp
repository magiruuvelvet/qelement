#pragma once

#include <QtWebEngineCore>
#include <QtWebEngineWidgets>

class WebEnginePage : public QWebEnginePage
{
    Q_OBJECT

public:
    explicit WebEnginePage(QWebEngineProfile *profile, QObject *parent = nullptr);

protected:
    bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool);
    WebEnginePage *createWindow(WebWindowType type);

    void javaScriptConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID);

private:
    void openExternalUrl(const QUrl &url);

    QWebEngineProfile *profile;
};
