#pragma once

#include <QTextBlockUserData>

#include "context_stack.h"

namespace Qutepart {

class TextBlockUserData : public QTextBlockUserData {
  public:
    TextBlockUserData(const QString &textTypeMap, const ContextStack &contexts);
    QString textTypeMap;
    ContextStack contexts;
    QString lineEnding;
};

} // namespace Qutepart
