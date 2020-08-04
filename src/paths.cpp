#include "paths.hpp"

#include <QStandardPaths>
#include <QDir>

std::unique_ptr<Paths> Paths::_defaultInstance;

Paths::Paths(const QString &prefix)
{
    if (prefix.isEmpty())
    {
        this->_baseLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }
    else
    {
        this->_baseLocation = prefix;
    }

    QDir root = QDir(this->_baseLocation);
    this->_valid = root.mkpath(".");
}

Paths *Paths::defaultInstance()
{
    if (!_defaultInstance)
    {
        _defaultInstance = std::make_unique<Paths>();
    }

    return _defaultInstance.get();
}

const QString &Paths::baseLocation(bool real) const
{
    if (real)
    {
        return this->_valid ? this->_baseLocation : this->_empty;
    }
    else
    {
        return this->_baseLocation;
    }
}

const QString Paths::webEngineProfilePath(const QString &profile, bool real) const
{
    if (!this->_valid)
    {
        return {};
    }

    const QString _profile = profile.isEmpty() ? "-default" : "-" + profile;

    const auto location = QString("%1/%2").arg(this->_baseLocation,
#ifdef DEBUG_BUILD
    "Profile-Debug" + _profile
#else
    "Profile" + _profile
#endif
    );

    const bool success = QDir(location).mkpath(".");

    if (real)
    {
        return success ? location : "";
    }
    else
    {
        return location;
    }
}
