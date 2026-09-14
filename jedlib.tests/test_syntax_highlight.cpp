#include "test_syntax_highlight.h"
#include "test_assert.h"
#include "jedlib/syntax_highlight.h"

JEDLIB_BEGIN

void test_syntax_highlight_cpp() {
  syntax_highlighter sh;
  EXPECT_TRUE(sh.extension_or_filename_has_syntax_highlighter("cpp"));
  EXPECT_TRUE(sh.extension_or_filename_has_syntax_highlighter("c"));
  EXPECT_TRUE(sh.extension_or_filename_has_syntax_highlighter("cxx"));
  EXPECT_TRUE(sh.extension_or_filename_has_keywords("cpp"));
  EXPECT_TRUE(sh.extension_or_filename_has_keywords("c"));
  EXPECT_TRUE(sh.extension_or_filename_has_keywords("cxx"));
  auto cd = sh.get_syntax_highlighter("cpp");
  EXPECT_EQ(std::string("/*"), cd.multiline_begin);
  EXPECT_EQ(std::string("*/"), cd.multiline_end);
  EXPECT_EQ(std::string("//"), cd.single_line);
  EXPECT_EQ(std::string("R\"("), cd.multistring_begin);
  EXPECT_EQ(std::string(")\""), cd.multistring_end);
  EXPECT_TRUE(cd.uses_quotes_for_chars);
  }

JEDLIB_END

void run_syntax_highlight_tests() {
  using namespace JEDLIB;
  test_syntax_highlight_cpp();
  }