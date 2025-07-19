#include "webenginepage.hpp"

#include <QDesktopServices>

WebEnginePage::WebEnginePage(QWebEngineProfile *profile, QObject *parent)
    : QWebEnginePage(profile, parent)
{
    this->profile = profile;
}

bool WebEnginePage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool)
{
    qDebug() << "acceptNavigationRequest:" << url;

    if (type == QWebEnginePage::NavigationTypeLinkClicked)
    {
        // open url in system web browser if target is outside of the application
        if (url.host() != "localhost")
        {
            QDesktopServices::openUrl(url);
            return false;
        }
        else
        {
            return true;
        }
    }

    return true;
}

WebEnginePage *WebEnginePage::createWindow(WebWindowType type)
{
    // open url in system web browser if target is outside of the application
    auto self = new WebEnginePage(this->profile);
    connect(self, &QWebEnginePage::urlChanged, self, &WebEnginePage::openExternalUrl);
    return self;
}

void WebEnginePage::javaScriptConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID)
{
    // do nothing, remove console spam
}

void WebEnginePage::openExternalUrl(const QUrl &url)
{
    qDebug() << "openExternalUrl:" << url;
    QDesktopServices::openUrl(url);

    // FIXME: suspicious memory leak
    qobject_cast<WebEnginePage*>(this->sender())->deleteLater();
    delete qobject_cast<WebEnginePage*>(this->sender());
}
