#include <QtTest/QtTest>

#include "base_indenter_test.h"

class Test : public BaseTest {
    Q_OBJECT

  private slots:
    void test() {
        qpart.setIndentAlgorithm(Qutepart::INDENT_ALG_HASKEL);
        qpart.setHighlighter("haskell.xml");
        runDataDrivenTest();
    }

    void test_data() {
        addColumns();

        QTest::newRow("dontIndent1") << "main = do\n"
                                        "    -- This is a comment\n"
                                        "    something\n"
                                        "    something\n"
                                        "\n"
                                        "\"\"]"
                                     << std::make_pair(0, 0)
                                     << "\n"
                                        "foo bar"
                                     << "main = do\n"
                                        "    -- This is a comment\n"
                                        "    something\n"
                                        "    something\n"
                                        "\n"
                                        "    foo bar\n"
                                        "\"\"]\n"
                                        "";

        QTest::newRow("if2") << "foo = if true\n"
                                "\"         then 1\"]"
                             << std::make_pair(0, 0)
                             << "\n"
                                "else 2"
                             << "foo = if true\n"
                                "         then 1\n"
                                "\"         else 2\"]\n"
                                "";

        QTest::newRow("afterComma1") << "primitives = [(\"+\", numericBinop (+)),\n"
                                        "\"\"]"
                                     << std::make_pair(0, 0)
                                     << "\n"
                                        "(\"-\", numericBinop (-)),"
                                     << "primitives = [(\"+\", numericBinop (+)),\n"
                                        "    (\"-\", numericBinop (-)),\n"
                                        "\"\"]\n"
                                        "";

        QTest::newRow("parsec1") << "parseExpr = parseString\n"
                                    "        <|> parseNumber\n"
                                    "\"\"]"
                                 << std::make_pair(0, 0)
                                 << "\n"
                                    "<|> parseAtom"
                                 << "parseExpr = parseString\n"
                                    "        <|> parseNumber\n"
                                    "        <|> parseAtom\n"
                                    "\"\"]\n"
                                    "";

        QTest::newRow("if1") << "foo = if true\n"
                                "\"\"]"
                             << std::make_pair(0, 0)
                             << "\n"
                                "then"
                             << "foo = if true\n"
                                "         then\n"
                                "\"\"]\n"
                                "";
    }
};

QTEST_MAIN(Test)
#include "test_indenter_haskel.moc"
