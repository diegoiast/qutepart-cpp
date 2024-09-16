#pragma once

#include <QTextBlockUserData>

#include "context_stack.h"

namespace Qutepart {

class TextBlockUserData : public QTextBlockUserData {
  public:
    TextBlockUserData(const QString &textTypeMap, const ContextStack &contexts);
    const QString &textTypeMap() const { return _textTypeMap; };
    const ContextStack &contexts() const { return _contexts; };

  private:
    QString _textTypeMap;
    ContextStack _contexts;
};

} // namespace Qutepart
