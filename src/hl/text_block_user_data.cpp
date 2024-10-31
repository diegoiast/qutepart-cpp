#include "text_block_user_data.h"

namespace Qutepart {

TextBlockUserData::TextBlockUserData(const QString &textTypeMap, const ContextStack &contexts)
    : textTypeMap(textTypeMap), contexts(contexts) {}

} // namespace Qutepart
