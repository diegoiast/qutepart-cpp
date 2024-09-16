#pragma once

#include <QSharedPointer>
#include <QXmlStreamReader>

#include "language.h"

namespace Qutepart {

QSharedPointer<Language> loadLanguage(const QString &xmlFileName);

ContextPtr loadExternalContext(const QString &contextName);

} // namespace Qutepart
