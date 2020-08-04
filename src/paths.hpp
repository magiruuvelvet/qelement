#pragma once

#include <memory>

#include <QString>

class Paths
{
public:
    /**
     * Constructs a paths object with the given prefix.
     * Leave empty to use the default prefix which is
     * QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).
     */
    Paths(const QString &prefix = {});

    /**
     * Returns a constructed default instance for standard usage.
     */
    static Paths *defaultInstance();

    /**
     * Prefix as given in the constructor.
     */
    const QString &baseLocation(bool real = true) const;

    /**
     * QtWebEngine profile path for persistent storage.
     */
    const QString webEngineProfilePath(const QString &profile = {}, bool real = true) const;

private:
    static std::unique_ptr<Paths> _defaultInstance;
    QString _empty;
    QString _baseLocation;
    bool _valid = false;
};
