#pragma once

#include <QWidget>
#include <QBoxLayout>
#include <QtWebEngine>
#include <QtWebEngineWidgets>
#include <QSystemTrayIcon>

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
        Normal,
        Notification,
        NetworkError,
    };

    void setNotificationIcon(NotificationIcon icon);

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

    QWebEngineNotification *_notification = nullptr;
};
