#pragma once

#include <QSharedPointer>
#include <QXmlStreamReader>

#include "language.h"

namespace Qutepart {

class Theme;

QSharedPointer<Language> loadLanguage(const QString &xmlFileName, const Theme *theme);

ContextPtr loadExternalContext(const QString &contextName, const Theme *theme);

} // namespace Qutepart
