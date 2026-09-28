#include "test_file_utils.h"
#include "test_assert.h"
#include "jedlib/file_utils.h"

JEDLIB_BEGIN

void test_ParsesSimpleRelativePath() {
  ParsedLocation loc;
  EXPECT_TRUE(parse_location("src/main.cpp:12:34:", loc));
  EXPECT_EQ(loc.path, std::filesystem::path("src/main.cpp"));
  EXPECT_EQ(loc.line, 12);
  EXPECT_EQ(loc.column, 34);
  }

void test_ParsesSimpleFilename() {
  ParsedLocation loc;
  EXPECT_TRUE(parse_location("main.cpp:1:2:", loc));
  EXPECT_EQ(loc.path, std::filesystem::path("main.cpp"));
  EXPECT_EQ(loc.line, 1);
  EXPECT_EQ(loc.column, 2);
  }

void test_ParsesPathContainingColon() {
  ParsedLocation loc;
  EXPECT_TRUE(parse_location("dir:with:colon/file.cpp:10:20:", loc));
  EXPECT_EQ(loc.path, std::filesystem::path("dir:with:colon/file.cpp"));
  EXPECT_EQ(loc.line, 10);
  EXPECT_EQ(loc.column, 20);
  }

void test_ParsesWindowsStylePath() {
  ParsedLocation loc;
  EXPECT_TRUE(parse_location(R"(C:\work\file.cpp:45:7:)", loc));
  EXPECT_EQ(loc.path, std::filesystem::path(R"(C:\work\file.cpp)"));
  EXPECT_EQ(loc.line, 45);
  EXPECT_EQ(loc.column, 7);
  }

void test_RejectsEmptyString() {
  ParsedLocation loc;
  EXPECT_TRUE(!parse_location("", loc));
  }

void test_AcceptsMissingTrailingColon() {
  ParsedLocation loc;
  EXPECT_TRUE(parse_location("src/main.cpp:12:34", loc));
  EXPECT_EQ(loc.line, 12);
  EXPECT_EQ(loc.column, 34);
  }

void test_RejectsMissingColumn() {
  ParsedLocation loc;
  EXPECT_TRUE(!parse_location("src/main.cpp:12::", loc));
  }

void test_RejectsMissingLine() {
  ParsedLocation loc;
  EXPECT_TRUE(!parse_location("src/main.cpp::34:", loc));
  }

void test_RejectsMissingPath() {
  ParsedLocation loc;
  EXPECT_TRUE(!parse_location(":12:34:", loc));
  }

void test_RejectsNonNumericLine() {
  ParsedLocation loc;
  EXPECT_TRUE(!parse_location("src/main.cpp:abc:34:", loc));
  }

void test_RejectsNonNumericColumn() {
  ParsedLocation loc;
  EXPECT_TRUE(!parse_location("src/main.cpp:12:xyz:", loc));
  }

void test_RejectsTooFewFields() {
  ParsedLocation loc;
  EXPECT_TRUE(!parse_location("src/main.cpp:12:", loc));
  }

void test_RejectsOnlyNumbers() {
  ParsedLocation loc;
  EXPECT_TRUE(!parse_location("12:34:", loc));
  }

void test_AcceptsZeroLineAndColumnIfImplementationAllowsIt() {
  ParsedLocation loc;
  EXPECT_TRUE(parse_location("src/main.cpp:0:0:", loc));
  EXPECT_EQ(loc.path, std::filesystem::path("src/main.cpp"));
  EXPECT_EQ(loc.line, 0);
  EXPECT_EQ(loc.column, 0);
  }

void test_RejectsNegativeLine() {
  ParsedLocation loc;
  EXPECT_TRUE(!parse_location("src/main.cpp:-1:5:", loc));
  }

void test_RejectsNegativeColumn() {
  ParsedLocation loc;
  EXPECT_TRUE(!parse_location("src/main.cpp:1:-5:", loc));
  }

void test_RejectsExtraTrailingCharacters() {
  ParsedLocation loc;
  EXPECT_TRUE(!parse_location("src/main.cpp:12:34:extra", loc));
  }

JEDLIB_END

void run_file_utils_tests() {
  using namespace JEDLIB;
  test_ParsesSimpleRelativePath();
  test_ParsesSimpleFilename();
  test_ParsesPathContainingColon();
  test_ParsesWindowsStylePath();
  test_RejectsEmptyString();
  test_AcceptsMissingTrailingColon();
  test_RejectsMissingColumn();
  test_RejectsMissingLine();
  test_RejectsMissingPath();
  test_RejectsNonNumericLine();
  test_RejectsNonNumericColumn();
  test_RejectsTooFewFields();
  test_RejectsOnlyNumbers();
  test_AcceptsZeroLineAndColumnIfImplementationAllowsIt();
  test_RejectsNegativeLine();
  test_RejectsNegativeColumn();
  test_RejectsExtraTrailingCharacters();
  }