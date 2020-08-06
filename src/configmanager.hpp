#pragma once

#include <QObject>
#include <QSettings>

#include <memory>

class ConfigManager : public QObject
{
    Q_OBJECT

public:
    explicit ConfigManager(const QString &baseLocation, QObject *parent = nullptr);
    ~ConfigManager();

    enum class Key
    {
        Webroot,
        SysTrayIconEnabled,
    };

    void setWebroot(const QString &webroot);
    const QString webroot() const;

    void setSysTrayIconEnabled(bool enabled);
    bool sysTrayIconEnabled() const;

signals:
    void configUpdated(const Key &key);

private:
    std::unique_ptr<QSettings> settings;

    void initialize_key(const Key &key);
};
