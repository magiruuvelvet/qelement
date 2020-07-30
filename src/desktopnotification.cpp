#include "desktopnotification.hpp"

#include <QApplication>

// conflicts with glib
#undef signals

#ifdef LIBNOTIFY_ENABLED
#include <libnotify/notify.h>
#include <libnotify/notification.h>
#endif

#ifdef LIBNOTIFY_ENABLED
static void d_libnotify_init(const QString &appname)
{
    static bool is_init = false;
    if (!is_init)
    {
        const auto utf8name = appname.toUtf8().constData();
        notify_init(utf8name);
        notify_set_app_name(utf8name);
        is_init = true;
    }
}
#endif

DesktopNotification::DesktopNotification()
{
#ifdef LIBNOTIFY_ENABLED
    d_libnotify_init(qApp->applicationDisplayName());
    this->d_ptr = notify_notification_new("", "", "");
#endif
}

DesktopNotification::DesktopNotification(const QString &title, const QString &message, const QImage &image)
    : _title(title),
      _message(message),
      _image(image)
{
#ifdef LIBNOTIFY_ENABLED
    d_libnotify_init(qApp->applicationDisplayName());
    this->d_ptr = notify_notification_new(_title.toUtf8().constData(), _message.toUtf8().constEnd(), "");
#endif
}

DesktopNotification::~DesktopNotification()
{
#ifdef LIBNOTIFY_ENABLED
    // note: calling free() causes double free or corruption error
    this->d_ptr = nullptr;
#endif
}

void DesktopNotification::send(const QString &title, const QString &message, const QImage &image)
{
#ifdef LIBNOTIFY_ENABLED
    d_libnotify_init(qApp->applicationDisplayName());

    auto notification = notify_notification_new(title.toUtf8().constData(), message.toUtf8().constData(), "");
    notify_notification_show(notification, nullptr);
#endif
}

void DesktopNotification::send() const
{
#ifdef LIBNOTIFY_ENABLED
    notify_notification_show((NotifyNotification*)d_ptr, nullptr);
#endif
}
