#pragma once

#include <QString>

namespace paths
{

// location where the profile is stored
// may return an empty string
const QString &webengine_profile_path(bool real = true);

}
