#include <QtTest/QtTest>

#include "base_indenter_test.h"

class Test : public BaseTest {
    Q_OBJECT

  private slots:
    void init() override {
        BaseTest::init();

        qpart.setHighlighter("ruby.xml");
        qpart.setIndentAlgorithm(Qutepart::INDENT_ALG_RUBY);
        qpart.setIndentWidth(2);
    }

    void If() { runDataDrivenTest(); }

    void If_data() {
        addColumns();

        QTest::newRow("if10") << "  if foo\n"
                              << std::make_pair(0, 8) << "\nblah"
                              << "  if foo\n"
                                 "    blah\n";

        QTest::newRow("if11") << "  if foo\n"
                                 "    blah\n"
                              << std::make_pair(1, 8) << "\nend"
                              << "  if foo\n"
                                 "    blah\n"
                                 "  end\n";

#if 0 // fails
        QTest::newRow("if20")
            <<  "  var = if foo\n"
            <<  std::make_pair(0, 14)
            <<  "\nblah"
            <<  "  var = if foo\n"
                "    blah\n";

        QTest::newRow("if21")
            <<  "  var = if foo\n"
                "    blah\n"
            <<  std::make_pair(1, 8)
            <<  "\nend"
            <<  "  var = if foo\n"
                "    blah\n"
                "  end\n";
#endif

        QTest::newRow("if22") << "  var = bar if foo\n"
                              << std::make_pair(0, 18) << "\nblah"
                              << "  var = bar if foo\n"
                                 "  blah\n";

        QTest::newRow("if30") << "  if foo; 42 else 37 end\n"
                              << std::make_pair(0, 24) << "\nblah"
                              << "  if foo; 42 else 37 end\n"
                                 "  blah\n";

        QTest::newRow("if31") << "  if foo then 42 else 37 end\n"
                              << std::make_pair(0, 28) << "\nblah"
                              << "  if foo then 42 else 37 end\n"
                                 "  blah\n";
    }

#if 0 // fails
    void Block() {
        runDataDrivenTest();
    }

    void Block_data() {
        addColumns();

        QTest::newRow("block01")
            <<  "10.times {\n"
                "  foo\n"
            <<  std::make_pair(1, 6)
            <<  "\nblah"
            <<  "10.times {\n"
                "  foo\n"
                "  blah\n";

        QTest::newRow("block02")
            <<  "10.times {\n"
                "  if foo\n"
            <<  std::make_pair(1, 8)
            <<  "\nblah"
            <<  "10.times {\n"
                "  if foo\n"
                "    blah\n";
    }
#endif

    void Basic() { runDataDrivenTest(); }

    void Basic_data() {
        addColumns();

        QTest::newRow("basic1") << "# basic1.txt\n"
                                << std::make_pair(0, 12)
                                << "\ndef foo\nif gets\nputs\nelse\nexit\nend\nend"
                                << "# basic1.txt\n"
                                   "def foo\n"
                                   "  if gets\n"
                                   "    puts\n"
                                   "  else\n"
                                   "    exit\n"
                                   "  end\n"
                                   "end\n";

#if 0 // fails
        QTest::newRow("basic2")
            <<  "# basic2.txt\n"
                "\n"
            <<  std::make_pair(1, 0)
            <<  "\nclass MyClass\n\nattr_reader :foo\n\nprivate\n\ndef helper(str)\nputs \"helper(#{str})\"\nend\n\nend"
            <<  "# basic2.txt\n"
                "\n"
                "class MyClass\n"
                "\n"
                "  attr_reader :foo\n"
                "\n"
                "  private\n"
                "\n"
                "  def helper(str)\n"
                "    puts \"helper(#{str})\"\n"
                "  end\n"
                "\n"
                "end\n";
#endif

        QTest::newRow("basic3") << "def foo\n"
                                   "    if check\n"
                                   "       bar\n"
                                << std::make_pair(2, 10) << "\nend\nend"
                                << "def foo\n"
                                   "    if check\n"
                                   "       bar\n"
                                   "    end\n"
                                   "end\n";

        QTest::newRow("basic4") << "def foo\n"
                                   "    array.each do |v|\n"
                                   "       bar\n"
                                << std::make_pair(2, 10) << "\nend\nend"
                                << "def foo\n"
                                   "    array.each do |v|\n"
                                   "       bar\n"
                                   "    end\n"
                                   "end\n";
    }

    void EmptyFile() { runDataDrivenTest(); }

    void EmptyFile_data() {
        addColumns();

        QTest::newRow("empty_file1") << "\n"
                                     << std::make_pair(0, 0) << "\n\n# Comment\ndef foo\nbar\nend"
                                     << "\n"
                                        "\n"
                                        "# Comment\n"
                                        "def foo\n"
                                        "  bar\n"
                                        "end\n";
    }

    void RegExp() { runDataDrivenTest(); }

    void RegExp_data() {
        addColumns();

        QTest::newRow("regexp1") << "  rx =~ /^hello/\n"
                                 << std::make_pair(0, 16) << "\nfoo"
                                 << "  rx =~ /^hello/\n"
                                    "  foo\n";
    }

    void Do() { runDataDrivenTest(); }

    void Do_data() {
        addColumns();

        QTest::newRow("do1") << "# do1.txt\n"
                                "5.times do\n"
                             << std::make_pair(1, 10) << "\nend"
                             << "# do1.txt\n"
                                "5.times do\n"
                                "end\n";

        QTest::newRow("do2") << "# do2.txt\n"
                                "File.open(\"file\") do |f|\n"
                             << std::make_pair(1, 24) << "\nf << foo\nend"
                             << "# do2.txt\n"
                                "File.open(\"file\") do |f|\n"
                                "  f << foo\n"
                                "end\n";

        QTest::newRow("do3") << "# do3.txt\n"
                                "[1,2,3].each_with_index do |obj, i|\n"
                             << std::make_pair(1, 35) << "\nputs \"#{i}: #{obj.inspect}\"\nend"
                             << "# do3.txt\n"
                                "[1,2,3].each_with_index do |obj, i|\n"
                                "  puts \"#{i}: #{obj.inspect}\"\n"
                                "end\n";

        QTest::newRow("do4") << "# do4.txt\n"
                                "File.open(\"#{base}.txt\") do |f|\n"
                             << std::make_pair(1, 31) << "\nf\nend"
                             << "# do4.txt\n"
                                "File.open(\"#{base}.txt\") do |f|\n"
                                "  f\n"
                                "end\n";

        QTest::newRow("do5") << "def foo(f)\n"
                                "  f.each do # loop\n"
                             << std::make_pair(1, 18) << "\nend"
                             << "def foo(f)\n"
                                "  f.each do # loop\n"
                                "  end\n";

        QTest::newRow("do6") << "def foo(f)\n"
                                "  f.each do # loop\n"
                             << std::make_pair(1, 18) << "\nbar\nend"
                             << "def foo(f)\n"
                                "  f.each do # loop\n"
                                "    bar\n"
                                "  end\n";

        QTest::newRow("do7") << "def foo(f)\n"
                                "  f.each do # loop with do\n"
                             << std::make_pair(1, 26) << "\nbar\nend"
                             << "def foo(f)\n"
                                "  f.each do # loop with do\n"
                                "    bar\n"
                                "  end\n";
    }

    void NoDo() { runDataDrivenTest(); }

    void NoDo_data() {
        addColumns();

        QTest::newRow("no_do1") << "# no-do1.txt\n"
                                   "if foo\n"
                                   "  # nothing to do\n"
                                << std::make_pair(2, 17) << "\nend"
                                << "# no-do1.txt\n"
                                   "if foo\n"
                                   "  # nothing to do\n"
                                   "end\n";

        QTest::newRow("no_do2") << "# no-do2.txt\n"
                                   "if foo\n"
                                   "  # nothing to do\n"
                                << std::make_pair(2, 17) << "\nf\nend"
                                << "# no-do2.txt\n"
                                   "if foo\n"
                                   "  # nothing to do\n"
                                   "  f\n"
                                   "end\n";

        QTest::newRow("no_do3") << "# no-do3.txt\n"
                                   "if foo\n"
                                   "  puts \"nothing\" # nothing to do\n"
                                << std::make_pair(2, 32) << "\nend"
                                << "# no-do3.txt\n"
                                   "if foo\n"
                                   "  puts \"nothing\" # nothing to do\n"
                                   "end\n";

        QTest::newRow("no_do4") << "# no-do4.txt\n"
                                   "if foo\n"
                                   "  puts \"nothing\" # nothing to do\n"
                                << std::make_pair(2, 32) << "\nf\nend"
                                << "# no-do4.txt\n"
                                   "if foo\n"
                                   "  puts \"nothing\" # nothing to do\n"
                                   "  f\n"
                                   "end\n";
    }

    void SingleLine() { runDataDrivenTest(); }

    void SingleLine_data() {
        addColumns();

        QTest::newRow("singleline01") << "  def foo() 42 end\n"
                                      << std::make_pair(0, 18) << "\nblah"
                                      << "  def foo() 42 end\n"
                                         "  blah\n";

        QTest::newRow("singleline02") << "  def foo; 42 end\n"
                                      << std::make_pair(0, 17) << "\nblah"
                                      << "  def foo; 42 end\n"
                                         "  blah\n";

        QTest::newRow("singleline03") << "  def foo() bar\n"
                                      << std::make_pair(0, 15) << "\nblah"
                                      << "  def foo() bar\n"
                                         "    blah\n";

        QTest::newRow("singleline04") << "  def foo; bar\n"
                                         "    blah\n"
                                      << std::make_pair(1, 8) << "\nend"
                                      << "  def foo; bar\n"
                                         "    blah\n"
                                         "  end\n";
    }

    void Array() { runDataDrivenTest(); }

    void Array_data() {
        addColumns();

        QTest::newRow("array1") << "  array = [ :a, :b, :c ]\n"
                                << std::make_pair(0, 24) << "\nfoo"
                                << "  array = [ :a, :b, :c ]\n"
                                   "  foo\n";

        QTest::newRow("array2") << "  array = [\n"
                                << std::make_pair(0, 11) << "\n:a"
                                << "  array = [\n"
                                   "    :a\n";

        QTest::newRow("array3") << "  array = [\n"
                                   "    :a,\n"
                                << std::make_pair(1, 7) << "\n:b"
                                << "  array = [\n"
                                   "    :a,\n"
                                   "    :b\n";

        QTest::newRow("array4") << "  array = [\n"
                                   "    :a,\n"
                                   "    :b\n"
                                << std::make_pair(2, 6) << "\n]"
                                << "  array = [\n"
                                   "    :a,\n"
                                   "    :b\n"
                                   "  ]\n";

        QTest::newRow("array5") << "  array = [\n"
                                   "    :a,\n"
                                   "    :b\n"
                                << std::make_pair(2, 6) << "\n(3,2)\n"
                                << "  array = [\n"
                                   "    :a,\n"
                                   "    :b\n"
                                   "  (3,2)\n"
                                   "  \n";

        QTest::newRow("array6") << "  array = [:a,\n"
                                   "           :b,\n"
                                   "           :c]\n"
                                << std::make_pair(2, 14) << "\nfoo"
                                << "  array = [:a,\n"
                                   "           :b,\n"
                                   "           :c]\n"
                                   "  foo\n";

        QTest::newRow("array7") << "  array = [:a,\n"
                                << std::make_pair(0, 14) << "\n:b"
                                << "  array = [:a,\n"
                                   "           :b\n";

        QTest::newRow("array8") << "  array = [:a,\n"
                                   "           :b,\n"
                                << std::make_pair(1, 14) << "\n:c"
                                << "  array = [:a,\n"
                                   "           :b,\n"
                                   "           :c\n";

        QTest::newRow("array9") << "  array = [:a,\n"
                                   "           :b,\n"
                                   "           :c\n"
                                << std::make_pair(2, 13) << "]\nfoo"
                                << "  array = [:a,\n"
                                   "           :b,\n"
                                   "           :c]\n"
                                   "  foo\n";

        QTest::newRow("array10") << "  array = [:a,\n"
                                    "           :b,\n"
                                    "           [:foo,\n"
                                 << std::make_pair(2, 17) << "\n:bar"
                                 << "  array = [:a,\n"
                                    "           :b,\n"
                                    "           [:foo,\n"
                                    "            :bar\n";

        QTest::newRow("array11") << "  array = [:a,\n"
                                    "           :b,\n"
                                    "           [:foo,\n"
                                    "            :bar,\n"
                                 << std::make_pair(3, 17) << "\n:baz"
                                 << "  array = [:a,\n"
                                    "           :b,\n"
                                    "           [:foo,\n"
                                    "            :bar,\n"
                                    "            :baz\n";

        QTest::newRow("array12") << "  array = [:a,\n"
                                    "           :b,\n"
                                    "           [:foo,\n"
                                    "            :bar],\n"
                                 << std::make_pair(3, 18) << "\n:baz"
                                 << "  array = [:a,\n"
                                    "           :b,\n"
                                    "           [:foo,\n"
                                    "            :bar],\n"
                                    "           :baz\n";

        QTest::newRow("array16") << "  array = [ :a,\n"
                                    "            :b,\n"
                                    "            :c ]\n"
                                 << std::make_pair(2, 16) << "\nfoo"
                                 << "  array = [ :a,\n"
                                    "            :b,\n"
                                    "            :c ]\n"
                                    "  foo\n";

        QTest::newRow("array17") << "  array = [ :a,\n"
                                 << std::make_pair(0, 15) << "\n:b"
                                 << "  array = [ :a,\n"
                                    "            :b\n";

        QTest::newRow("array18") << "  array = [ :a,\n"
                                    "            :b,\n"
                                 << std::make_pair(1, 15) << "\n:c"
                                 << "  array = [ :a,\n"
                                    "            :b,\n"
                                    "            :c\n";

        QTest::newRow("array19") << "  array = [ :a,\n"
                                    "            :b,\n"
                                    "            :c\n"
                                 << std::make_pair(2, 14) << " ]\nfoo"
                                 << "  array = [ :a,\n"
                                    "            :b,\n"
                                    "            :c ]\n"
                                    "  foo\n";

        QTest::newRow("array20") << "  array = [ :a,\n"
                                    "            :b,\n"
                                    "            [ :foo,\n"
                                 << std::make_pair(2, 19) << "\n:bar"
                                 << "  array = [ :a,\n"
                                    "            :b,\n"
                                    "            [ :foo,\n"
                                    "              :bar\n";

        QTest::newRow("array21") << "  array = [ :a,\n"
                                    "            :b,\n"
                                    "            [ :foo,\n"
                                    "              :bar,\n"
                                 << std::make_pair(3, 19) << "\n:baz"
                                 << "  array = [ :a,\n"
                                    "            :b,\n"
                                    "            [ :foo,\n"
                                    "              :bar,\n"
                                    "              :baz\n";

        QTest::newRow("array22") << "  array = [ :a,\n"
                                    "            :b,\n"
                                    "            [ :foo,\n"
                                    "              :bar ],\n"
                                 << std::make_pair(3, 21) << "\n:baz"
                                 << "  array = [ :a,\n"
                                    "            :b,\n"
                                    "            [ :foo,\n"
                                    "              :bar ],\n"
                                    "            :baz\n";
    }

    void ArrayComment() { runDataDrivenTest(); }

    void ArrayComment_data() {
        addColumns();

        QTest::newRow("array_comment1") << "  array = [ :a, :b, :c ] # comment\n"
                                           "                         # comment\n"
                                        << std::make_pair(1, 34) << "\nfoo"
                                        << "  array = [ :a, :b, :c ] # comment\n"
                                           "                         # comment\n"
                                           "  foo\n";

        QTest::newRow("array_comment2") << "  array = [ # comment\n"
                                        << std::make_pair(0, 21) << "\n:a"
                                        << "  array = [ # comment\n"
                                           "    :a\n";

        QTest::newRow("array_comment3") << "  array = [ # comment\n"
                                           "    :a,     # comment\n"
                                        << std::make_pair(1, 21) << "\n:b"
                                        << "  array = [ # comment\n"
                                           "    :a,     # comment\n"
                                           "    :b\n";

        QTest::newRow("array_comment4") << "  array = [\n"
                                           "    :a,\n"
                                           "    :b # comment,\n"
                                        << std::make_pair(2, 17) << "\n]"
                                        << "  array = [\n"
                                           "    :a,\n"
                                           "    :b # comment,\n"
                                           "  ]\n";

        QTest::newRow("array_comment5") << "  array = [\n"
                                           "    :a,\n"
                                           "    :b # comment\n"
                                        << std::make_pair(2, 16) << "\n(3,2)\n"
                                        << "  array = [\n"
                                           "    :a,\n"
                                           "    :b # comment\n"
                                           "  (3,2)\n"
                                           "  \n";
    }

    void Hash() { runDataDrivenTest(); }

    void Hash_data() {
        addColumns();

        QTest::newRow("hash1") << "  hash = { :a => 1, :b => 2, :c => 3 }\n"
                               << std::make_pair(0, 38) << "\nfoo"
                               << "  hash = { :a => 1, :b => 2, :c => 3 }\n"
                                  "  foo\n";

        QTest::newRow("hash2") << "  hash = {\n"
                               << std::make_pair(0, 10) << "\n:a => 1"
                               << "  hash = {\n"
                                  "    :a => 1\n";

        QTest::newRow("hash3") << "  hash = {\n"
                                  "    :a => 1,\n"
                               << std::make_pair(1, 12) << "\n:b => 2"
                               << "  hash = {\n"
                                  "    :a => 1,\n"
                                  "    :b => 2\n";

        QTest::newRow("hash4") << "  hash = {\n"
                                  "    :a => 1,\n"
                                  "    :b => 2\n"
                               << std::make_pair(2, 11) << "\n}"
                               << "  hash = {\n"
                                  "    :a => 1,\n"
                                  "    :b => 2\n"
                                  "  }\n";

        QTest::newRow("hash5") << "  hash = {\n"
                                  "    :a => 1,\n"
                                  "    :b => 2\n"
                               << std::make_pair(2, 11) << "\n(3,2)\n"
                               << "  hash = {\n"
                                  "    :a => 1,\n"
                                  "    :b => 2\n"
                                  "  (3,2)\n"
                                  "  \n";

        QTest::newRow("hash6") << "  hash = {:a => 1,\n"
                                  "          :b => 2,\n"
                                  "          :c => 3}\n"
                               << std::make_pair(2, 18) << "\nfoo"
                               << "  hash = {:a => 1,\n"
                                  "          :b => 2,\n"
                                  "          :c => 3}\n"
                                  "  foo\n";

        QTest::newRow("hash7") << "  hash = {:a => 1,\n"
                               << std::make_pair(0, 18) << "\n:b => 2"
                               << "  hash = {:a => 1,\n"
                                  "          :b => 2\n";

        QTest::newRow("hash8") << "  hash = {:a => 1,\n"
                                  "          :b => 2,\n"
                               << std::make_pair(1, 18) << "\n:c => 3"
                               << "  hash = {:a => 1,\n"
                                  "          :b => 2,\n"
                                  "          :c => 3\n";

        QTest::newRow("hash9") << "  hash = {:a => 1,\n"
                                  "          :b => 2,\n"
                                  "          :c => 3\n"
                               << std::make_pair(2, 17) << "}\nfoo"
                               << "  hash = {:a => 1,\n"
                                  "          :b => 2,\n"
                                  "          :c => 3}\n"
                                  "  foo\n";

        QTest::newRow("hash10") << "  hash = {:a => 1,\n"
                                   "          :b => 2,\n"
                                   "          :c => {:foo => /^f/,\n"
                                << std::make_pair(2, 30) << "\n:bar => /^b/"
                                << "  hash = {:a => 1,\n"
                                   "          :b => 2,\n"
                                   "          :c => {:foo => /^f/,\n"
                                   "                 :bar => /^b/\n";

        QTest::newRow("hash11") << "  hash = {:a => 1,\n"
                                   "          :b => 2,\n"
                                   "          :c => {:foo => /^f/,\n"
                                   "                 :bar => /^b/},\n"
                                << std::make_pair(3, 31) << "\n:d => 4"
                                << "  hash = {:a => 1,\n"
                                   "          :b => 2,\n"
                                   "          :c => {:foo => /^f/,\n"
                                   "                 :bar => /^b/},\n"
                                   "          :d => 4\n";

        QTest::newRow("hash12") << "  hash = {:a => 1,\n"
                                   "          :b => 2,\n"
                                   "          :c => {:foo => /^f/,\n"
                                   "                 :bar => /^b/,\n"
                                << std::make_pair(3, 30) << "\n:baz => /^b/"
                                << "  hash = {:a => 1,\n"
                                   "          :b => 2,\n"
                                   "          :c => {:foo => /^f/,\n"
                                   "                 :bar => /^b/,\n"
                                   "                 :baz => /^b/\n";

        QTest::newRow("hash16") << "  hash = { :a => 1,\n"
                                   "           :b => 2,\n"
                                   "           :c => 3 }\n"
                                << std::make_pair(2, 20) << "\nfoo"
                                << "  hash = { :a => 1,\n"
                                   "           :b => 2,\n"
                                   "           :c => 3 }\n"
                                   "  foo\n";

        QTest::newRow("hash17") << "  hash = { :a => 1,\n"
                                << std::make_pair(0, 19) << "\n:b => 2"
                                << "  hash = { :a => 1,\n"
                                   "           :b => 2\n";

        QTest::newRow("hash18") << "  hash = { :a => 1,\n"
                                   "           :b => 2,\n"
                                << std::make_pair(1, 19) << "\n:c => 3"
                                << "  hash = { :a => 1,\n"
                                   "           :b => 2,\n"
                                   "           :c => 3\n";

        QTest::newRow("hash19") << "  hash = { :a => 1,\n"
                                   "           :b => 2,\n"
                                   "           :c => 3\n"
                                << std::make_pair(2, 18) << " }\nfoo"
                                << "  hash = { :a => 1,\n"
                                   "           :b => 2,\n"
                                   "           :c => 3 }\n"
                                   "  foo\n";

        QTest::newRow("hash20") << "  hash = { :a => 1,\n"
                                   "           :b => 2,\n"
                                   "           :c => { :foo => /^f/,\n"
                                << std::make_pair(2, 32) << "\n:bar => /^b/"
                                << "  hash = { :a => 1,\n"
                                   "           :b => 2,\n"
                                   "           :c => { :foo => /^f/,\n"
                                   "                   :bar => /^b/\n";

        QTest::newRow("hash21") << "  hash = { :a => 1,\n"
                                   "           :b => 2,\n"
                                   "           :c => { :foo => /^f/,\n"
                                   "                   :bar => /^b/ },\n"
                                << std::make_pair(3, 34) << "\n:d => 4"
                                << "  hash = { :a => 1,\n"
                                   "           :b => 2,\n"
                                   "           :c => { :foo => /^f/,\n"
                                   "                   :bar => /^b/ },\n"
                                   "           :d => 4\n";

        QTest::newRow("hash22") << "  hash = { :a => 1,\n"
                                   "           :b => 2,\n"
                                   "           :c => { :foo => /^f/,\n"
                                   "                   :bar => /^b/,\n"
                                << std::make_pair(3, 32) << "\n:baz => /^b/"
                                << "  hash = { :a => 1,\n"
                                   "           :b => 2,\n"
                                   "           :c => { :foo => /^f/,\n"
                                   "                   :bar => /^b/,\n"
                                   "                   :baz => /^b/\n";
    }

    void Ops() { runDataDrivenTest(); }

    void Ops_data() {
        addColumns();

        QTest::newRow("ops1") << "t = foo() +\n"
                              << std::make_pair(0, 11) << "\nbar()"
                              << "t = foo() +\n"
                                 "    bar()\n";

        QTest::newRow("ops2") << "t = foo() + # Comment\n"
                              << std::make_pair(0, 21) << "\nbar()"
                              << "t = foo() + # Comment\n"
                                 "    bar()\n";

        QTest::newRow("ops3") << "t = foo() -\n"
                              << std::make_pair(0, 11) << "\nbar()"
                              << "t = foo() -\n"
                                 "    bar()\n";

        QTest::newRow("ops4") << "t = foo() - # Comment\n"
                              << std::make_pair(0, 21) << "\nbar()"
                              << "t = foo() - # Comment\n"
                                 "    bar()\n";

        QTest::newRow("ops5") << "t = foo() *\n"
                              << std::make_pair(0, 11) << "\nbar()"
                              << "t = foo() *\n"
                                 "    bar()\n";

        QTest::newRow("ops6") << "t = foo() * # Comment\n"
                              << std::make_pair(0, 21) << "\nbar()"
                              << "t = foo() * # Comment\n"
                                 "    bar()\n";

        QTest::newRow("ops7") << "t = foo() /\n"
                              << std::make_pair(0, 11) << "\nbar()"
                              << "t = foo() /\n"
                                 "    bar()\n";

        QTest::newRow("ops8") << "t = foo() / # Comment\n"
                              << std::make_pair(0, 21) << "\nbar()"
                              << "t = foo() / # Comment\n"
                                 "    bar()\n";

        QTest::newRow("ops11") << "t = foo() +\n"
                                  "    bar()\n"
                               << std::make_pair(1, 9) << "\nfoobar"
                               << "t = foo() +\n"
                                  "    bar()\n"
                                  "foobar\n";

        QTest::newRow("ops12") << "t = foo() + # Comment\n"
                                  "    bar()\n"
                               << std::make_pair(1, 9) << "\nfoobar"
                               << "t = foo() + # Comment\n"
                                  "    bar()\n"
                                  "foobar\n";

        QTest::newRow("ops13") << "t = foo() -\n"
                                  "    bar()\n"
                               << std::make_pair(1, 9) << "\nfoobar"
                               << "t = foo() -\n"
                                  "    bar()\n"
                                  "foobar\n";

        QTest::newRow("ops14") << "t = foo() - # Comment\n"
                                  "    bar()\n"
                               << std::make_pair(1, 9) << "\nfoobar"
                               << "t = foo() - # Comment\n"
                                  "    bar()\n"
                                  "foobar\n";

        QTest::newRow("ops15") << "t = foo() *\n"
                                  "    bar()\n"
                               << std::make_pair(1, 9) << "\nfoobar"
                               << "t = foo() *\n"
                                  "    bar()\n"
                                  "foobar\n";

        QTest::newRow("ops16") << "t = foo() * # Comment\n"
                                  "    bar()\n"
                               << std::make_pair(1, 9) << "\nfoobar"
                               << "t = foo() * # Comment\n"
                                  "    bar()\n"
                                  "foobar\n";

        QTest::newRow("ops17") << "t = foo() /\n"
                                  "    bar()\n"
                               << std::make_pair(1, 9) << "\nfoobar"
                               << "t = foo() /\n"
                                  "    bar()\n"
                                  "foobar\n";

        QTest::newRow("ops18") << "t = foo() / # Comment\n"
                                  "    bar()\n"
                               << std::make_pair(1, 9) << "\nfoobar"
                               << "t = foo() / # Comment\n"
                                  "    bar()\n"
                                  "foobar\n";

        QTest::newRow("ops21") << "t = foo() +\n"
                                  "    bar() # Comment\n"
                               << std::make_pair(1, 19) << "\nfoobar"
                               << "t = foo() +\n"
                                  "    bar() # Comment\n"
                                  "foobar\n";

        QTest::newRow("ops22") << "t = foo() + # Comment\n"
                                  "    bar() # Comment\n"
                               << std::make_pair(1, 19) << "\nfoobar"
                               << "t = foo() + # Comment\n"
                                  "    bar() # Comment\n"
                                  "foobar\n";

        QTest::newRow("ops23") << "t = foo() -\n"
                                  "    bar() # Comment\n"
                               << std::make_pair(1, 19) << "\nfoobar"
                               << "t = foo() -\n"
                                  "    bar() # Comment\n"
                                  "foobar\n";

        QTest::newRow("ops24") << "t = foo() - # Comment\n"
                                  "    bar() # Comment\n"
                               << std::make_pair(1, 19) << "\nfoobar"
                               << "t = foo() - # Comment\n"
                                  "    bar() # Comment\n"
                                  "foobar\n";

        QTest::newRow("ops25") << "t = foo() *\n"
                                  "    bar() # Comment\n"
                               << std::make_pair(1, 19) << "\nfoobar"
                               << "t = foo() *\n"
                                  "    bar() # Comment\n"
                                  "foobar\n";

        QTest::newRow("ops26") << "t = foo() * # Comment\n"
                                  "    bar() # Comment\n"
                               << std::make_pair(1, 19) << "\nfoobar"
                               << "t = foo() * # Comment\n"
                                  "    bar() # Comment\n"
                                  "foobar\n";

        QTest::newRow("ops27") << "t = foo() /\n"
                                  "    bar() # Comment\n"
                               << std::make_pair(1, 19) << "\nfoobar"
                               << "t = foo() /\n"
                                  "    bar() # Comment\n"
                                  "foobar\n";

        QTest::newRow("ops28") << "t = foo() / # Comment\n"
                                  "    bar() # Comment\n"
                               << std::make_pair(1, 19) << "\nfoobar"
                               << "t = foo() / # Comment\n"
                                  "    bar() # Comment\n"
                                  "foobar\n";
    }

    void WordList() { runDataDrivenTest(); }

    void WordList_data() {
        addColumns();

        QTest::newRow("wordlist01") << "  for elem in %w[ foo, bar,\n"
                                    << std::make_pair(0, 27) << "\nfoobar"
                                    << "  for elem in %w[ foo, bar,\n"
                                       "                  foobar\n";

        QTest::newRow("wordlist02") << "  for elem in %w[ foo, bar,\n"
                                       "                  foobar ]\n"
                                    << std::make_pair(1, 26) << "\nblah"
                                    << "  for elem in %w[ foo, bar,\n"
                                       "                  foobar ]\n"
                                       "    blah\n";

#if 0 // fails
        QTest::newRow("wordlist11")
            <<  "  for elem in %w< foo, bar,\n"
            <<  std::make_pair(0, 27)
            <<  "\nfoobar"
            <<  "  for elem in %w< foo, bar,\n"
                "                  foobar\n";

        QTest::newRow("wordlist12")
            <<  "  for elem in %w< foo, bar,\n"
                "                  foobar >\n"
            <<  std::make_pair(1, 26)
            <<  "\nblah"
            <<  "  for elem in %w< foo, bar,\n"
                "                  foobar >\n"
                "    blah\n";

        QTest::newRow("wordlist21")
            <<  "  for elem in %w| foo, bar,\n"
            <<  std::make_pair(0, 27)
            <<  "\nfoobar"
            <<  "  for elem in %w| foo, bar,\n"
                "                  foobar\n";

        QTest::newRow("wordlist22")
            <<  "  for elem in %w| foo, bar,\n"
                "                  foobar |\n"
            <<  std::make_pair(1, 26)
            <<  "\nblah"
            <<  "  for elem in %w| foo, bar,\n"
                "                  foobar |\n"
                "    blah\n";
#endif
    }

    void Multiline() { runDataDrivenTest(); }

    void Multiline_data() {
        addColumns();

        QTest::newRow("multiline1") << "# multiline1.txt\n"
                                       "if (foo == \"bar\" and\n"
                                       "    bar == \"foo\")\n"
                                    << std::make_pair(2, 17) << "\nend"
                                    << "# multiline1.txt\n"
                                       "if (foo == \"bar\" and\n"
                                       "    bar == \"foo\")\n"
                                       "end\n";

        QTest::newRow("multiline2") << "# multiline2.txt\n"
                                       "if (foo == \"bar\" and\n"
                                       "    bar == \"foo\")\n"
                                    << std::make_pair(2, 17) << "\nputs\nend"
                                    << "# multiline2.txt\n"
                                       "if (foo == \"bar\" and\n"
                                       "    bar == \"foo\")\n"
                                       "  puts\n"
                                       "end\n";

        QTest::newRow("multiline3") << "# multiline3.txt\n"
                                       "s = \"hello\" +\n"
                                    << std::make_pair(1, 13) << "\n\"world\"\nbar"
                                    << "# multiline3.txt\n"
                                       "s = \"hello\" +\n"
                                       "    \"world\"\n"
                                       "bar\n";

        QTest::newRow("multiline4") << "# multiline4.txt\n"
                                       "s = \"hello\" +\n"
                                       "    # Comment\n"
                                    << std::make_pair(2, 13) << "\n\"world\"\nbar"
                                    << "# multiline4.txt\n"
                                       "s = \"hello\" +\n"
                                       "    # Comment\n"
                                       "    \"world\"\n"
                                       "bar\n";

        QTest::newRow("multiline5") << "# multiline5.txt\n"
                                       "s = \"hello\" +\n"
                                       "# Misplaced comment\n"
                                    << std::make_pair(2, 19) << "\n\"world\"\nbar"
                                    << "# multiline5.txt\n"
                                       "s = \"hello\" +\n"
                                       "# Misplaced comment\n"
                                       "    \"world\"\n"
                                       "bar\n";

#if 0 // fails
        QTest::newRow("multiline6")
            <<  "# multiline6.txt\n"
                "foo \"hello\" \\\n"
            <<  std::make_pair(1, 13)
            <<  "\n\nbar"
            <<  "# multiline6.txt\n"
                "foo \"hello\" \\\n"
                "\n"
                "bar\n";
#endif

        QTest::newRow("multiline7") << "foo \"hello\",\n"
                                    << std::make_pair(0, 12) << "\n\"world\""
                                    << "foo \"hello\",\n"
                                       "    \"world\"\n";

        QTest::newRow("multiline8") << "def foo(array)\n"
                                       "  array.each_with_index \\\n"
                                       "      do\n"
                                    << std::make_pair(2, 8) << "\nbar\nend"
                                    << "def foo(array)\n"
                                       "  array.each_with_index \\\n"
                                       "      do\n"
                                       "    bar\n"
                                       "  end\n";

        QTest::newRow("multiline9") << "if test \\\n"
                                       "# if ends here\n"
                                       "  foo do\n"
                                    << std::make_pair(2, 8) << "\nbar"
                                    << "if test \\\n"
                                       "# if ends here\n"
                                       "  foo do\n"
                                       "    bar\n";

        QTest::newRow("multiline10") << "if test1 &&\n"
                                        "# still part of condition\n"
                                        "   test2\n"
                                     << std::make_pair(2, 8) << "\nfoo"
                                     << "if test1 &&\n"
                                        "# still part of condition\n"
                                        "   test2\n"
                                        "  foo\n";
    }

    void Plist() { runDataDrivenTest(); }

    void Plist_data() {
        addColumns();

        QTest::newRow("plist1") << "  foobar(foo,\n"
                                << std::make_pair(0, 13) << "\nbar"
                                << "  foobar(foo,\n"
                                   "         bar\n";

        QTest::newRow("plist2") << "  foobar(foo, foo,\n"
                                << std::make_pair(0, 18) << "\nbar"
                                << "  foobar(foo, foo,\n"
                                   "         bar\n";

        QTest::newRow("plist3") << "  foobar(foo,\n"
                                   "         bar)\n"
                                << std::make_pair(1, 13) << "\nblah"
                                << "  foobar(foo,\n"
                                   "         bar)\n"
                                   "  blah\n";

        QTest::newRow("plist4") << "  foobar(foo,\n"
                                   "         bar,\n"
                                << std::make_pair(1, 13) << "\nbaz"
                                << "  foobar(foo,\n"
                                   "         bar,\n"
                                   "         baz\n";

        QTest::newRow("plist5") << "  foobar(foo,\n"
                                   "         bar, bar,\n"
                                << std::make_pair(1, 18) << "\nbaz"
                                << "  foobar(foo,\n"
                                   "         bar, bar,\n"
                                   "         baz\n";

        QTest::newRow("plist6") << "  foobar(foo,\n"
                                   "         bar,\n"
                                   "         baz)\n"
                                << std::make_pair(2, 13) << "\nblah"
                                << "  foobar(foo,\n"
                                   "         bar,\n"
                                   "         baz)\n"
                                   "  blah\n";

        QTest::newRow("plist7") << "  foobar(foo(bar,\n"
                                << std::make_pair(0, 17) << "\nbaz"
                                << "  foobar(foo(bar,\n"
                                   "             baz\n";

        QTest::newRow("plist8") << "  foobar(foo(bar,\n"
                                   "             baz),\n"
                                << std::make_pair(1, 18) << "\nblah"
                                << "  foobar(foo(bar,\n"
                                   "             baz),\n"
                                   "         blah\n";

        QTest::newRow("plist9") << "  foobar(foo(bar,\n"
                                   "             baz),\n"
                                   "         foobaz(),\n"
                                << std::make_pair(2, 18) << "\nblah"
                                << "  foobar(foo(bar,\n"
                                   "             baz),\n"
                                   "         foobaz(),\n"
                                   "         blah\n";

        QTest::newRow("plist10") << "  foobar(foo(bar,\n"
                                    "             baz),\n"
                                    "         blah)\n"
                                 << std::make_pair(2, 14) << "\nfoobaz"
                                 << "  foobar(foo(bar,\n"
                                    "             baz),\n"
                                    "         blah)\n"
                                    "  foobaz\n";

        QTest::newRow("plist11") << "  foobar( foo,\n"
                                 << std::make_pair(0, 14) << "\nbar"
                                 << "  foobar( foo,\n"
                                    "          bar\n";

        QTest::newRow("plist12") << "  foobar( foo, foo,\n"
                                 << std::make_pair(0, 19) << "\nbar"
                                 << "  foobar( foo, foo,\n"
                                    "          bar\n";

        QTest::newRow("plist13") << "  foobar( foo,\n"
                                    "          bar )\n"
                                 << std::make_pair(1, 15) << "\nblah"
                                 << "  foobar( foo,\n"
                                    "          bar )\n"
                                    "  blah\n";

        QTest::newRow("plist14") << "  foobar ( foo,\n"
                                    "           bar,\n"
                                 << std::make_pair(1, 15) << "\nbaz"
                                 << "  foobar ( foo,\n"
                                    "           bar,\n"
                                    "           baz\n";

        QTest::newRow("plist15") << "  foobar ( foo,\n"
                                    "           bar, bar,\n"
                                 << std::make_pair(1, 20) << "\nbaz"
                                 << "  foobar ( foo,\n"
                                    "           bar, bar,\n"
                                    "           baz\n";

        QTest::newRow("plist16") << "  foobar ( foo,\n"
                                    "           bar,\n"
                                    "           baz )\n"
                                 << std::make_pair(2, 16) << "\nblah"
                                 << "  foobar ( foo,\n"
                                    "           bar,\n"
                                    "           baz )\n"
                                    "  blah\n";

        QTest::newRow("plist17") << "  foobar ( foo ( bar,\n"
                                 << std::make_pair(0, 21) << "\nbaz"
                                 << "  foobar ( foo ( bar,\n"
                                    "                 baz\n";

        QTest::newRow("plist18") << "  foobar ( foo ( bar,\n"
                                    "                 baz ),\n"
                                 << std::make_pair(1, 23) << "\nblah"
                                 << "  foobar ( foo ( bar,\n"
                                    "                 baz ),\n"
                                    "           blah\n";

        QTest::newRow("plist19") << "  foobar ( foo ( bar,\n"
                                    "                 baz ),\n"
                                    "           foobaz(),\n"
                                 << std::make_pair(2, 20) << "\nblah"
                                 << "  foobar ( foo ( bar,\n"
                                    "                 baz ),\n"
                                    "           foobaz(),\n"
                                    "           blah\n";

        QTest::newRow("plist20") << "  foobar ( foo ( bar,\n"
                                    "                 baz ),\n"
                                    "           blah )\n"
                                 << std::make_pair(2, 17) << "\nfoobaz"
                                 << "  foobar ( foo ( bar,\n"
                                    "                 baz ),\n"
                                    "           blah )\n"
                                    "  foobaz\n";
    }

    void PlistComment() { runDataDrivenTest(); }

    void PlistComment_data() {
        addColumns();

        QTest::newRow("plist_comment1") << "  foobar(foo, # comment\n"
                                        << std::make_pair(0, 23) << "\nbar"
                                        << "  foobar(foo, # comment\n"
                                           "         bar\n";

        QTest::newRow("plist_comment2") << "  foobar(foo, foo, # comment\n"
                                        << std::make_pair(0, 28) << "\nbar"
                                        << "  foobar(foo, foo, # comment\n"
                                           "         bar\n";

        QTest::newRow("plist_comment3") << "  foobar(foo,\n"
                                           "         # comment\n"
                                           "         bar) # comment\n"
                                        << std::make_pair(2, 23) << "\nblah"
                                        << "  foobar(foo,\n"
                                           "         # comment\n"
                                           "         bar) # comment\n"
                                           "  blah\n";

        QTest::newRow("plist_comment4") << "  foobar(foo,\n"
                                           "         bar, # comment\n"
                                           "         # comment\n"
                                        << std::make_pair(2, 18) << "\nbaz"
                                        << "  foobar(foo,\n"
                                           "         bar, # comment\n"
                                           "         # comment\n"
                                           "         baz\n";

        QTest::newRow("plist_comment5") << "  foobar(foo,\n"
                                           "         bar, bar, # comment\n"
                                           "                   # comment\n"
                                        << std::make_pair(2, 28) << "\nbaz"
                                        << "  foobar(foo,\n"
                                           "         bar, bar, # comment\n"
                                           "                   # comment\n"
                                           "         baz\n";

        QTest::newRow("plist_comment6") << "  foobar(foo,\n"
                                           "         bar,\n"
                                           "         baz) # comment,\n"
                                        << std::make_pair(2, 24) << "\nblah"
                                        << "  foobar(foo,\n"
                                           "         bar,\n"
                                           "         baz) # comment,\n"
                                           "  blah\n";

        QTest::newRow("plist_comment7") << "  foobar(foo(bar, # comment\n"
                                           "                  # comment\n"
                                        << std::make_pair(1, 27) << "\nbaz"
                                        << "  foobar(foo(bar, # comment\n"
                                           "                  # comment\n"
                                           "             baz\n";

        QTest::newRow("plist_comment8") << "  foobar(foo(bar,\n"
                                           "  # comment\n"
                                           "             baz), # comment\n"
                                        << std::make_pair(2, 28) << "\nblah"
                                        << "  foobar(foo(bar,\n"
                                           "  # comment\n"
                                           "             baz), # comment\n"
                                           "         blah\n";

        QTest::newRow("plist_comment9") << "  foobar(foo(bar,\n"
                                           "             baz), # comment\n"
                                           "         foobaz(), # comment\n"
                                           "                   # comment\n"
                                        << std::make_pair(3, 28) << "\nblah"
                                        << "  foobar(foo(bar,\n"
                                           "             baz), # comment\n"
                                           "         foobaz(), # comment\n"
                                           "                   # comment\n"
                                           "         blah\n";

        QTest::newRow("plist_comment10") << "  foobar(foo(bar,\n"
                                            "             baz),\n"
                                            "       # comment\n"
                                            "         blah)\n"
                                            "       # comment\n"
                                         << std::make_pair(4, 16) << "\nfoobaz"
                                         << "  foobar(foo(bar,\n"
                                            "             baz),\n"
                                            "       # comment\n"
                                            "         blah)\n"
                                            "       # comment\n"
                                            "  foobaz\n";
    }

    void Comment() { runDataDrivenTest(); }

    void Comment_data() {
        addColumns();

        QTest::newRow("comment1") << "# comment1.txt\n"
                                     "if foo\n"
                                     "  # Comment\n"
                                  << std::make_pair(2, 11) << "\nend"
                                  << "# comment1.txt\n"
                                     "if foo\n"
                                     "  # Comment\n"
                                     "end\n";

        QTest::newRow("comment2") << "# comment2.txt\n"
                                     "if foo\n"
                                     "  # Comment\n"
                                  << std::make_pair(2, 11) << "\nbar\nend"
                                  << "# comment2.txt\n"
                                     "if foo\n"
                                     "  # Comment\n"
                                     "  bar\n"
                                     "end\n";

        QTest::newRow("comment3") << "# comment3.txt\n"
                                     "if foo\n"
                                     "  # Comment\n"
                                  << std::make_pair(2, 11) << "\nbar\n# Another comment\nend"
                                  << "# comment3.txt\n"
                                     "if foo\n"
                                     "  # Comment\n"
                                     "  bar\n"
                                     "  # Another comment\n"
                                     "end\n";

        QTest::newRow("comment4") << "# comment4.txt\n"
                                     "if foo\n"
                                     "  # Comment\n"
                                  << std::make_pair(2, 11) << "\nbar # Another comment\nend"
                                  << "# comment4.txt\n"
                                     "if foo\n"
                                     "  # Comment\n"
                                     "  bar # Another comment\n"
                                     "end\n";

        QTest::newRow("comment5") << "# comment5.txt\n"
                                     "if foo\n"
                                     "     # Misplaced comment\n"
                                  << std::make_pair(2, 24) << "\nbar\nend"
                                  << "# comment5.txt\n"
                                     "if foo\n"
                                     "     # Misplaced comment\n"
                                     "  bar\n"
                                     "end\n";

        QTest::newRow("comment6") << "# comment6.txt\n"
                                     "if foo\n"
                                     "# Misplaced comment\n"
                                  << std::make_pair(2, 19) << "\nbar\nend"
                                  << "# comment6.txt\n"
                                     "if foo\n"
                                     "# Misplaced comment\n"
                                     "  bar\n"
                                     "end\n";
    }

    void Heredoc() { runDataDrivenTest(); }

    void Heredoc_data() {
        addColumns();

        QTest::newRow("heredoc1") << "doc = <<EOF\n"
                                  << std::make_pair(0, 11) << "\nkoko"
                                  << "doc = <<EOF\n"
                                     "koko\n";

        QTest::newRow("heredoc2") << "doc = <<EOF\n"
                                  << std::make_pair(0, 11) << "\n\nkoko"
                                  << "doc = <<EOF\n"
                                     "\n"
                                     "koko\n";

        QTest::newRow("heredoc3") << "doc = <<EOF\n"
                                     "if foo\n"
                                  << std::make_pair(1, 6) << "\nkoko"
                                  << "doc = <<EOF\n"
                                     "if foo\n"
                                     "koko\n";

        QTest::newRow("heredoc4") << "doc = <<EOF\n"
                                     "if foo\n"
                                  << std::make_pair(1, 6) << "\n\nkoko"
                                  << "doc = <<EOF\n"
                                     "if foo\n"
                                     "\n"
                                     "koko\n";

        QTest::newRow("heredoc5") << "if foo\n"
                                     "doc = <<EOF\n"
                                  << std::make_pair(1, 11) << "\nkoko"
                                  << "if foo\n"
                                     "doc = <<EOF\n"
                                     "koko\n";

        QTest::newRow("heredoc6") << "if foo\n"
                                     "doc = <<EOF\n"
                                  << std::make_pair(1, 11) << "\n\nkoko"
                                  << "if foo\n"
                                     "doc = <<EOF\n"
                                     "\n"
                                     "koko\n";
    }

    void BlockComment() { runDataDrivenTest(); }

    void BlockComment_data() {
        addColumns();

        QTest::newRow("block_comment1") << "=begin\n"
                                        << std::make_pair(0, 6) << "\nkoko"
                                        << "=begin\n"
                                           "koko\n";

        QTest::newRow("block_comment2") << "=begin\n"
                                        << std::make_pair(0, 6) << "\n\nkoko"
                                        << "=begin\n"
                                           "\n"
                                           "koko\n";

        QTest::newRow("block_comment3") << "=begin\n"
                                           "if foo\n"
                                        << std::make_pair(1, 6) << "\nkoko"
                                        << "=begin\n"
                                           "if foo\n"
                                           "koko\n";

        QTest::newRow("block_comment4") << "=begin\n"
                                           "if foo\n"
                                        << std::make_pair(1, 6) << "\n\nkoko"
                                        << "=begin\n"
                                           "if foo\n"
                                           "\n"
                                           "koko\n";

        QTest::newRow("block_comment5") << "if foo\n"
                                           "=begin\n"
                                        << std::make_pair(1, 6) << "\nkoko"
                                        << "if foo\n"
                                           "=begin\n"
                                           "koko\n";

        QTest::newRow("block_comment6") << "if foo\n"
                                           "=begin\n"
                                        << std::make_pair(1, 6) << "\n\nkoko"
                                        << "if foo\n"
                                           "=begin\n"
                                           "\n"
                                           "koko\n";

        QTest::newRow("block_comment7") << "if foo\n"
                                           "=begin\n"
                                           "=end\n"
                                        << std::make_pair(2, 4) << "\nkoko"
                                        << "if foo\n"
                                           "=begin\n"
                                           "=end\n"
                                           "  koko\n";

#if 0 // TODO fails
        QTest::newRow("block_comment8")
            <<  "if foo\n"
                "=begin\n"
                "=end\n"
            <<  std::make_pair(2, 4)
            <<  "\n\nkoko"
            <<  "if foo\n"
                "=begin\n"
                "=end\n"
                "\n"
                "  koko\n";
#endif
    }
};

QTEST_MAIN(Test)
#include "test_indenter_ruby.moc"
