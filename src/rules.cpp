#include "rules.h"

AbstractRule::AbstractRule(/*ContextPtr parentContext,*/
                           const AbstractRuleParams& params):
    /*parentContext(parentContext),*/
    textType(params.textType),
    attribute(params.attribute),
    context(params.context),
    lookAhead(params.lookAhead),
    firstNonSpace(params.firstNonSpace),
    column(params.column),
    dynamic(params.dynamic)
{}

void AbstractRule::printDescription(QTextStream& out) const {
    out << "\t\tAbstractRule\n";
}


KeywordRule::KeywordRule(const AbstractRuleParams& params,
                         const QString& string):
    AbstractRule(params),
    string(string)
{}

void KeywordRule::printDescription(QTextStream& out) const {
    out << "\t\tKeyword(" << string << ")\n";
}