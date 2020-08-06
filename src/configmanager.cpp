#include "configmanager.hpp"

#include <unordered_map>

struct KeyValuePair
{
    const QString key;
    const QVariant value;
};

static const std::unordered_map<ConfigManager::Key, KeyValuePair> definitions = {
    {ConfigManager::Key::Webroot, {"element/webroot", QString("/opt/Element/resources/webapp")}},
};

static inline const decltype(KeyValuePair::key) keyName(const ConfigManager::Key &key)
{
    return definitions.at(key).key;
}
static inline const decltype(KeyValuePair::value) value(const ConfigManager::Key &key)
{
    return definitions.at(key).value;
}

ConfigManager::ConfigManager(const QString &baseLocation, QObject *parent)
    : QObject(parent)
{
    const auto file = QString("%1/%2").arg(baseLocation, "preferences.ini");
    this->settings = std::make_unique<QSettings>(file, QSettings::IniFormat, this);

    // initialize defaults
    this->initialize_key(Key::Webroot);
}

void ConfigManager::initialize_key(const Key &key)
{
    if (!this->settings->contains(keyName(key)))
    {
        this->settings->setValue(keyName(key), value(key));
    }
}

ConfigManager::~ConfigManager()
{
    this->settings->sync();
}

void ConfigManager::setWebroot(const QString &webroot)
{
    this->settings->setValue(keyName(Key::Webroot), webroot);
    emit configUpdated(Key::Webroot);
}

const QString ConfigManager::webroot() const
{
    return this->settings->value(keyName(Key::Webroot), value(Key::Webroot)).toString();
}
