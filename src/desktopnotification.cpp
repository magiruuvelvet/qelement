#include "desktopnotification.hpp"

#include <QApplication>
#include <QColorSpace>

// conflicts with glib
#undef signals

// note: don't use QString for libnotify functions, but std::string::c_str()
//       libnotify does weird things and Valgrind shows errors in QString destructor

#ifdef LIBNOTIFY_ENABLED
#include <libnotify/notify.h>
#include <libnotify/notification.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#ifdef LIBNOTIFY_ENABLED
static std::string d_appname;
static void d_libnotify_init(const QString &appname)
{
    static bool is_init = false;
    if (!is_init)
    {
        d_appname = appname.toStdString();
        notify_init(d_appname.c_str());
        notify_set_app_name(d_appname.c_str());
        is_init = true;
    }
}
#endif

DesktopNotification::DesktopNotification()
{
#ifdef LIBNOTIFY_ENABLED
    d_libnotify_init(qApp->applicationDisplayName());
    this->d_ptr = notify_notification_new("", "", nullptr);
#endif
}

DesktopNotification::DesktopNotification(const QString &title, const QString &message, const QImage &image, const std::function<void()> &click_callback)
    : DesktopNotification()
{
    this->_title = title;
    this->_message = message;
    this->_image = image;
    this->clickcb = click_callback;

    this->update_notification_properties();
}

DesktopNotification::~DesktopNotification()
{
#ifdef LIBNOTIFY_ENABLED
    // crashes with invalid pointer
    //std::free((NotifyNotification*)this->d_ptr);
#endif

    if (this->d_img)
    {
        std::free(this->d_img);
        this->d_img = nullptr;
    }

    this->d_ptr = nullptr;
}

void DesktopNotification::setTitle(const QString &title)
{
    this->_title = title;
    this->update_notification_properties();
}

void DesktopNotification::setMessage(const QString &message)
{
    this->_message = message;
    this->update_notification_properties();
}

void DesktopNotification::setImage(const QImage &image)
{
    this->_image = image;
    this->update_notification_properties();
}

void DesktopNotification::setClickCallback(const std::function<void()> &click_callback)
{
    this->clickcb = click_callback;
    this->update_notification_properties();
}

void DesktopNotification::send(const QString &title, const QString &message, const QImage &image, const std::function<void()> &click_callback)
{
    DesktopNotification notification(title, message, image, click_callback);
    notification.send();
}

void DesktopNotification::send() const
{
#ifdef LIBNOTIFY_ENABLED
    notify_notification_show((NotifyNotification*)this->d_ptr, nullptr);
#endif
}

void DesktopNotification::update_notification_properties()
{
#ifdef LIBNOTIFY_ENABLED
    const std::string title_c = _title.toStdString();
    const std::string message_c = _message.toStdString();
    const void *image_c = this->convert_image(_image);

    notify_notification_update((NotifyNotification*)this->d_ptr, title_c.c_str(), message_c.c_str(), nullptr);
    notify_notification_set_image_from_pixbuf((NotifyNotification*)this->d_ptr, (GdkPixbuf*)image_c);

//    auto action = [](NotifyNotification *n, gchar *action, gpointer data) {
//        typedef void (*callback_ptr)();
//        ((callback_ptr)data)();
//    };

//    notify_notification_clear_actions((NotifyNotification*)this->d_ptr);
//    notify_notification_add_action((NotifyNotification*)this->d_ptr, "DoAction", "default", action, &this->clickcb, nullptr);
#endif
}

const void *DesktopNotification::convert_image(const QImage &image)
{
    if (image.isNull())
    {
        return nullptr;
    }

#ifdef LIBNOTIFY_ENABLED

    // remove old image is present
    if (this->d_img)
    {
        std::free(this->d_img);
        this->d_img = nullptr;
    }

    QImage backup(image);
    backup.convertToColorSpace(QColorSpace::SRgb);
    backup = backup.convertToFormat(QImage::Format_RGBA8888);

    // copy image data
    unsigned char *b = static_cast<unsigned char*>(std::malloc(backup.sizeInBytes()));
    memcpy(b, backup.constBits(), backup.sizeInBytes());
    this->d_img = b;

    int stride = backup.width() * 4;
    return gdk_pixbuf_new_from_data(b, GDK_COLORSPACE_RGB, true, 8, backup.width(), backup.height(), stride, nullptr, nullptr);
#else
    return nullptr;
#endif
}
