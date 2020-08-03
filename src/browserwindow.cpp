#include "browserwindow.hpp"
#include "paths.hpp"
#include "desktopnotification.hpp"

#include <QShortcut>
#include <QShowEvent>
#include <QCloseEvent>

BrowserWindow::BrowserWindow(QWidget *parent)
    : QWidget(parent)
{
    this->setMinimumSize(250, 250);

    // retain size when hidden
    auto size_policy = this->sizePolicy();
    size_policy.setRetainSizeWhenHidden(true);
    this->setSizePolicy(size_policy);

    // setup layout
    this->_layout = std::make_unique<QHBoxLayout>();
    this->_layout->setContentsMargins(0, 0, 0, 0);
    this->_layout->setSpacing(0);
    this->setLayout(this->_layout.get());

    // setup web view
    this->_webview = std::make_unique<WebEngineView>();
    this->_layout->addWidget(this->_webview.get());

    // retain webview size when hidden to avoid rendering bugs when shown again by clicking on the tray icon
    auto webview_size_policy = this->_webview->sizePolicy();
    webview_size_policy.setRetainSizeWhenHidden(true);
    this->_webview->setSizePolicy(webview_size_policy);

    webview = this->_webview.get();
    page = webview->page();
    settings = webview->settings();
    profile = page->profile();

    webview->setContextMenuPolicy(Qt::NoContextMenu);

    profile->setCachePath(paths::webengine_profile_path());
    profile->setPersistentStoragePath(QString("%1/%2").arg(paths::webengine_profile_path(), "Storage"));
    profile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);
    profile->setNotificationPresenter([&](std::unique_ptr<QWebEngineNotification> notification){
        qDebug() << "notification received:" << notification->title() << notification->message();
        this->_notification = notification.get();

        // only trigger notifications when application is not visible
        if (!this->isVisible())
        {
            this->_hasNotification = true;

            if (this->trayIcon)
            {
                // indicate that there are notifications
                this->setNotificationIcon(NotificationIcon::Notification);

                // send notification using Qt when libnotify is not enabled
#ifndef LIBNOTIFY_ENABLED
                this->trayIcon->showMessage(
                    notification->title(),
                    notification->message(),
                    QPixmap::fromImage(notification->icon()), 3000);
#endif
            }

            // send notification using libnotify when enabled
#ifdef LIBNOTIFY_ENABLED
            DesktopNotification::send(
                notification->title(),
                notification->message(),
                notification->icon());
#endif

            notification->show();
        }
    });
    this->initializeScripts();

    // add application to user agent
    auto useragent = profile->httpUserAgent();
    useragent.append(QString(" %1/%2").arg(qApp->applicationName(), qApp->applicationVersion()));
    profile->setHttpUserAgent(useragent);

    settings->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    settings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    settings->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
    settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    settings->setAttribute(QWebEngineSettings::ShowScrollBars, false);

    connect(page, &QWebEnginePage::fullScreenRequested, this, &BrowserWindow::acceptFullScreen);
    connect(page, &QWebEnginePage::featurePermissionRequested, this, &BrowserWindow::acceptFeaturePermission);
    connect(page, &QWebEnginePage::loadFinished, this, &BrowserWindow::setupNetworkMonitor);

    page->setUrl(QUrl("element://localhost/"));

    // create system tray icon with notification support
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        this->trayIcon = std::make_unique<QSystemTrayIcon>();
        this->setNotificationIcon(NotificationIcon::Normal);

        this->trayMenu = std::make_unique<QMenu>();
        trayMenu->addAction({}, this, [&]{
            this->trayTriggerCallback(QSystemTrayIcon::Trigger);
        });
        trayMenu->addSeparator();
        trayMenu->addAction(tr("Quit %1").arg(qApp->applicationDisplayName()), this, []{
            qApp->quit();
        });

        connect(trayIcon.get(), &QSystemTrayIcon::activated, this, &BrowserWindow::trayTriggerCallback);
        connect(trayIcon.get(), &QSystemTrayIcon::messageClicked, this, &BrowserWindow::notificationMessageClicked);

        this->trayIcon->setContextMenu(trayMenu.get());
        this->trayIcon->show();
    }

    // setup network monitor
    this->networkMonitor = std::make_unique<QNetworkAccessManager>(this);
    connect(this->networkMonitor.get(), &QNetworkAccessManager::finished, this, &BrowserWindow::updateNetworkState);
    this->networkMonitorTimer = std::make_unique<QTimer>(this);
    connect(this->networkMonitorTimer.get(), &QTimer::timeout, this, [&]{
        if (!this->homeserver.isEmpty())
        {
            this->networkMonitor->get(QNetworkRequest(QUrl(this->homeserver)));
        }
        this->networkMonitorTimer->setInterval(60000);
        this->networkMonitorTimer->start();
    });
    this->networkMonitorTimer->setInterval(60000);
    this->networkMonitorTimer->start();

    // setup shortcuts
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this),
            &QShortcut::activated, this, []{
                qApp->quit();
            });
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_R), this),
            &QShortcut::activated, this, [&]{
                webview->triggerPageAction(QWebEnginePage::ReloadAndBypassCache);
            });
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_I), this),
            &QShortcut::activated, this, [&]{
                //page->setInspectedPage(page);
            });
}

BrowserWindow::~BrowserWindow()
{
    this->networkMonitorTimer->stop();
}

void BrowserWindow::setNotificationIcon(NotificationIcon icon)
{
    // don't call QSystemTrayIcon::setIcon() when the icon didn't change
    if (this->_notificationIcon == icon)
    {
        return;
    }

    this->_notificationIcon = icon;

    if (this->trayIcon)
    {
        switch (icon)
        {
            case NotificationIcon::Normal:
                this->trayIcon->setIcon(qApp->windowIcon());
                break;

            case NotificationIcon::Notification:
                this->trayIcon->setIcon(QIcon(":/element-notification.png"));
                break;

            case NotificationIcon::NetworkError:
                this->trayIcon->setIcon(QIcon(":/element-networkerror.png"));
                break;
        }
    }
}

BrowserWindow::NotificationIcon BrowserWindow::notificationIcon() const
{
    return this->_notificationIcon;
}

bool BrowserWindow::hasNotification() const
{
    return this->_hasNotification;
}

void BrowserWindow::acceptFullScreen(QWebEngineFullScreenRequest req)
{
    req.accept();
    this->isFullScreen() ? this->showNormal() : this->showFullScreen();
}

void BrowserWindow::acceptFeaturePermission(const QUrl &origin, QWebEnginePage::Feature feature)
{
    page->setFeaturePermission(origin, feature, QWebEnginePage::PermissionGrantedByUser);
}

void BrowserWindow::notificationMessageClicked()
{
    this->show();
    this->activateWindow();

    if (this->_notification)
    {
        this->_notification->click();
        this->_notification = nullptr;
    }
}

void BrowserWindow::showEvent(QShowEvent *event)
{
    this->_hasNotification = false;

    if (this->trayIcon)
    {
        // remove notification indicator from tray icon
        this->setNotificationIcon(NotificationIcon::Normal);

        this->updateShowHideMenuAction();
    }

    this->restoreGeometry(this->_geometry);
    event->accept();
}

void BrowserWindow::closeEvent(QCloseEvent *event)
{
    if (this->trayIcon)
    {
        // if tray icon is present, close the window instead of exiting the application
        this->_geometry = this->saveGeometry();
        this->hide();
        this->updateShowHideMenuAction();
    }
    else
    {
        // exit the application when no tray icon is present
        event->accept();
    }
}

void BrowserWindow::trayTriggerCallback(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
    {
        if (this->isVisible())
        {
            this->_geometry = this->saveGeometry();
            this->hide();
        }
        else
        {
            this->restoreGeometry(this->_geometry);
            this->show();
            this->activateWindow();
        }

        this->updateShowHideMenuAction();
    }
}

void BrowserWindow::updateShowHideMenuAction()
{
    if (this->trayIcon)
    {
        const auto action = trayMenu->actions().at(0);

        if (this->isVisible())
        {
            action->setText(tr("Hide"));
        }
        else
        {
            action->setText(tr("Show"));
        }
    }
}

void BrowserWindow::initializeScripts()
{
    auto scripts = profile->scripts();

    QWebEngineScript homeserverUrlGetter;
    homeserverUrlGetter.setName("homeserver-url-getter");
    homeserverUrlGetter.setWorldId(QWebEngineScript::MainWorld);
    homeserverUrlGetter.setInjectionPoint(QWebEngineScript::Deferred);
    homeserverUrlGetter.setSourceCode(R"(window.mx_hs_url = localStorage.getItem("mx_hs_url");)");

    // notifications are reset to disabled on every app restart and reload for some unknown reason
    QWebEngineScript notificationFixer;
    notificationFixer.setName("notification-fixer");
    notificationFixer.setWorldId(QWebEngineScript::MainWorld);
    notificationFixer.setInjectionPoint(QWebEngineScript::Deferred);
    notificationFixer.setSourceCode(R"(
        console.info("fixing notifications...");
        let notification_fixer = () => {
            try { mxSettingsStore.setValue("notificationsEnabled", null, "device", true); } catch (e){}
            try { mxSettingsStore.setValue("notificationBodyEnabled", null, "device", true); } catch (e){}
            try { mxSettingsStore.setValue("audioNotificationsEnabled", null, "device", true); } catch (e){}
        };
        // run snippet 3 times to make absolutely sure, sometimes the setting doesn't apply
        setTimeout(notification_fixer, 2000);
        setTimeout(notification_fixer, 3000);
        setTimeout(notification_fixer, 4000);
    )");


    if (!scripts->contains(homeserverUrlGetter))
    {
        qDebug() << "install homeserver url getter script...";
        scripts->insert(homeserverUrlGetter);
    }

    if (!scripts->contains(notificationFixer))
    {
        qDebug() << "install notification fixer script...";
        scripts->insert(notificationFixer);
    }
}

void BrowserWindow::setupNetworkMonitor(bool ok)
{
    if (ok)
    {
        QTimer::singleShot(10000, this, [&]{
            page->runJavaScript("window.mx_hs_url;", [&](const QVariant &result) {
                if (result.canConvert(QVariant::String))
                {
                    this->homeserver = result.toString();
                    qDebug() << "homeserver url:" << this->homeserver;
                    if (!this->homeserver.isEmpty())
                    {
                        this->networkMonitor->get(QNetworkRequest(QUrl(this->homeserver)));
                    }
                }
                else
                {
                    this->homeserver.clear();
                }
            });
        });
    }
    else
    {
        this->homeserver.clear();
    }
}

void BrowserWindow::updateNetworkState(QNetworkReply *reply)
{
    qDebug() << "network monitor:" << reply->url() << reply->error();

    if (reply->error() == QNetworkReply::NoError)
    {
        if (this->_hasNotification)
        {
            this->setNotificationIcon(NotificationIcon::Notification);
        }
        else
        {
            this->setNotificationIcon(NotificationIcon::Normal);
        }
    }
    else
    {
        this->setNotificationIcon(NotificationIcon::NetworkError);
    }
}
