#pragma once

#include <QString>
#include <QImage>

#include <functional>

// TODO: experimental libnotify implementation, possible memory leaks and crashes
class DesktopNotification
{
public:
    DesktopNotification();
    DesktopNotification(const QString &title, const QString &message, const QImage &image = {}, const std::function<void()> &click_callback = []{});
    ~DesktopNotification();

    void setTitle(const QString &title);
    void setMessage(const QString &message);
    void setImage(const QImage &image);
    void setClickCallback(const std::function<void()> &click_callback);

    static void send(const QString &title, const QString &message, const QImage &image = {}, const std::function<void()> &click_callback = []{});
    void send() const;

private:
    QString _title;
    QString _message;
    QImage _image;

private:
    void update_notification_properties();
    const void *convert_image(const QImage &image);
    void *d_ptr = nullptr;
    unsigned char *d_img = nullptr;

    std::function<void()> clickcb = []{};
};
