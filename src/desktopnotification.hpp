#pragma once

#include <QString>
#include <QImage>

// TODO: experimental libnotify implementation, possible memory leaks and crashes
class DesktopNotification
{
public:
    DesktopNotification();
    DesktopNotification(const QString &title, const QString &message, const QImage &image = {});
    ~DesktopNotification();

    // works
    static void send(const QString &title, const QString &message, const QImage &image = {});

    // doesn't work for some reason :/
    void send() const;

private:
    QString _title;
    QString _message;
    QImage _image;

    void *d_ptr = nullptr;
};
