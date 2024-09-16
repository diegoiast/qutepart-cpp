#include "text_block_user_data.h"

#include "text_type.h"

namespace Qutepart {

namespace {

TextBlockUserData *getData(const QTextBlock &block) {
    QTextBlockUserData *data = block.userData();
    if (data == nullptr) {
        return nullptr;
    }

    return dynamic_cast<TextBlockUserData *>(data);
}

QChar getTextType(const QTextBlock &block, int column) {
    TextBlockUserData *data = getData(block);
    if (data == nullptr) {
        return ' ';
    } else {
        return data->textTypeMap()[column];
    }
}

} // namespace

QString textTypeMap(const QTextBlock &block) {
    TextBlockUserData *data = getData(block);
    if (data == nullptr) {
        return QString().fill(' ', block.text().length());
    }

    return data->textTypeMap();
}

bool isCode(const QTextBlock &block, int column) { return getTextType(block, column) == ' '; }

bool isComment(const QTextBlock &block, int column) {
    QChar type = getTextType(block, column);

    return type == 'c' || type == 'b' || type == 'h';
}

bool isBlockComment(const QTextBlock &block, int column) {
    return getTextType(block, column) == 'b';
}

bool isHereDoc(const QTextBlock &block, int column) { return getTextType(block, column) == 'h'; }

} // namespace Qutepart
