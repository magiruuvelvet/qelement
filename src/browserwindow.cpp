#include "browserwindow.hpp"
#include "paths.hpp"

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
            if (this->trayIcon)
            {
                // indicate that there are notifications
                this->setNotificationIcon(NotificationIcon::Notification);

                // FIXME: generally works, but use libnotify on supported operating systems instead
                this->trayIcon->showMessage(
                    notification->title(),
                    notification->message(),
                    QPixmap::fromImage(notification->icon()), 3000);
            }

            notification->show();
        }
    });

    // add application to user agent
    auto useragent = profile->httpUserAgent();
    useragent.append(QString(" %1/%2").arg(qApp->applicationName(), qApp->applicationVersion()));
    profile->setHttpUserAgent(useragent);

    settings->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    settings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    settings->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    settings->setAttribute(QWebEngineSettings::ShowScrollBars, false);

    connect(page, &QWebEnginePage::fullScreenRequested, this, &BrowserWindow::acceptFullScreen);
    connect(page, &QWebEnginePage::featurePermissionRequested, this, &BrowserWindow::acceptFeaturePermission);

    page->setUrl(QUrl("http://localhost:63535/index.html"));

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
        connect(trayIcon.get(), &QSystemTrayIcon::messageClicked, this, [&]{
            this->show();
            if (this->_notification)
            {
                this->_notification->click();
                this->_notification = nullptr;
            }
        });

        this->trayIcon->setContextMenu(trayMenu.get());
        this->trayIcon->show();
    }

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
}

void BrowserWindow::setNotificationIcon(NotificationIcon icon)
{
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

void BrowserWindow::acceptFullScreen(QWebEngineFullScreenRequest req)
{
    req.accept();
    this->isFullScreen() ? this->showNormal() : this->showFullScreen();
}

void BrowserWindow::acceptFeaturePermission(const QUrl &origin, QWebEnginePage::Feature feature)
{
    page->setFeaturePermission(origin, feature, QWebEnginePage::PermissionGrantedByUser);
}

void BrowserWindow::showEvent(QShowEvent *event)
{
    if (this->trayIcon)
    {
        // remove notification indicator from tray icon
        this->setNotificationIcon(NotificationIcon::Normal);

        this->updateShowHideMenuAction();
    }

    event->accept();
}

void BrowserWindow::closeEvent(QCloseEvent *event)
{
    if (this->trayIcon)
    {
        // if tray icon is present, close the window instead of exiting the application
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
            this->hide();
        }
        else
        {
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
