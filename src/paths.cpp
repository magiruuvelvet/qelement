#include "paths.hpp"

#include <QStandardPaths>
#include <QDir>

namespace paths
{

const QString &webengine_profile_path(const QString &profile, bool real)
{
    // first-time initialization
    static QString empty;
    static const auto base_location = QString("%1/%2").arg(
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation),
#ifdef DEBUG_BUILD
        "Profile-Debug-" + profile);
#else
        "Profile-" + profile);
#endif
    static auto init = ([&]{
        QDir root = QDir(base_location);
        return root.mkpath(".");
    })();

    if (real)
    {
        if (!init) return empty;
        return base_location;
    }
    else
    {
        return base_location;
    }
}

}
