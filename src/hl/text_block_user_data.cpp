#include "text_block_user_data.h"

namespace Qutepart {

TextBlockUserData::TextBlockUserData(const QString &textTypeMap, const ContextStack &contexts)
    : _textTypeMap(textTypeMap), _contexts(contexts) {}

} // namespace Qutepart
