#pragma once

#include <QWidget>
#include <QBoxLayout>
#include <QtWebEngine>
#include <QtWebEngineWidgets>
#include <QSystemTrayIcon>
#include <QNetworkAccessManager>

#include "webengineview.hpp"

#include <memory>

class BrowserWindow : public QWidget
{
    Q_OBJECT

public:
    explicit BrowserWindow(QWidget *parent = nullptr);
    ~BrowserWindow();

    enum class NotificationIcon
    {
        NoIcon          = -1,
        Normal          = 0,
        Notification    = 1,
        NetworkError    = 2,
    };

    void setNotificationIcon(NotificationIcon icon);
    NotificationIcon notificationIcon() const;
    bool hasNotification() const;

private:
    void acceptFullScreen(QWebEngineFullScreenRequest);
    void acceptFeaturePermission(const QUrl &origin, QWebEnginePage::Feature feature);
    void notificationMessageClicked();

protected:
    void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    std::unique_ptr<QHBoxLayout> _layout;
    std::unique_ptr<WebEngineView> _webview;
    QByteArray _geometry;

    std::unique_ptr<QSystemTrayIcon> trayIcon;
    std::unique_ptr<QMenu> trayMenu;

    // convenience pointers for web view
    WebEngineView *webview;
    WebEnginePage *page;
    QWebEngineSettings *settings;
    QWebEngineProfile *profile;

    void trayTriggerCallback(QSystemTrayIcon::ActivationReason reason);
    void updateShowHideMenuAction();
    void initializeScripts();
    void setupNetworkMonitor(bool ok);
    void updateNetworkState(QNetworkReply *reply);

    NotificationIcon _notificationIcon = NotificationIcon::NoIcon;
    bool _hasNotification = false;
    QWebEngineNotification *_notification = nullptr;

    QString homeserver;
    std::unique_ptr<QNetworkAccessManager> networkMonitor;
    std::unique_ptr<QTimer> networkMonitorTimer;
};
