#include "test_buffer.h"
#include "test_assert.h"

#include "jedlib/buffer.h"

#include <iostream>
#include <thread>
#include <stdint.h>

JEDLIB_BEGIN

namespace {

  env_settings make_settings() {
    env_settings s{};
    s.tab_space = 4;
    s.show_all_characters = false;
    s.perform_syntax_highlighting = false;
    return s;
    }

  line make_line(const std::string& s) {
    return to_text(s)[0];
    }

  file_buffer make_buffer() {
    file_buffer buf = make_empty_buffer();
    buf.content = to_text(
      "I'm nobody! Who are you?\n"
      "Are you nobody, too?\n"
      "Then there's a pair of us - don't tell!\n"
      "They'd banish us, you know.\n"
      "\n"
      "How dreary to be somebody!\n"
      "How public, like a frog\n"
      "To tell your name the livelong day\n"
      "To an admiring bog!"
    );
    return buf;
    }

  int64_t line_length(const line& ln) {
    return static_cast<int64_t>(ln.size());
    }

  } // namespace

void test_BufferLineLength() {
  line ln = make_line("abc");
  EXPECT_EQ(3, line_length(ln));
  }

void test_BufferLineLengthUnicoded() {
  line ln = make_line("aÏ€bc");
  EXPECT_EQ(4, line_length(ln));
  }

void test_BufferTestsFailRead() {
  auto fb = read_from_file("peom.txt");
  EXPECT_TRUE(fb.content.empty());
  EXPECT_EQ_STR(fb.name.c_str(), "peom.txt");
  }

void test_BufferTestsReadWrite() {
  file_buffer buf = make_buffer();

  bool success = false;
  file_buffer outbuf = save_to_file(success, buf, "poem.txt");
  EXPECT_TRUE(success);
  EXPECT_EQ(to_string(buf.content), to_string(outbuf.content));

  file_buffer inbuf = read_from_file("poem.txt");
  EXPECT_EQ(to_string(buf.content), to_string(inbuf.content));
  }

void test_BufferInsertCharInsertsAtCursorWithoutMutatingOriginal() {
  env_settings s = make_settings();

  file_buffer buf = make_empty_buffer();
  buf = insert(buf, L"a", s);

  file_buffer with_cursor = buf;
  with_cursor.pos = position(0, 1);

  file_buffer out = insert(with_cursor, L"x", s);

  std::string out_line = to_string(out.content[0]);
  std::string original_line = to_string(with_cursor.content[0]);

  EXPECT_EQ("ax", out_line);
  EXPECT_EQ("a", original_line);
  }

void test_BufferInsertCharInMiddleOfExistingLineFromMakeBuffer() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  file_buffer with_cursor = buf;
  with_cursor.pos = position(1, 8);

  file_buffer out = insert(with_cursor, L"X", s);

  std::string edited_line = to_string(out.content[1]);
  std::string original_line = to_string(with_cursor.content[1]);
  std::string prev_line = to_string(out.content[0]);
  std::string next_line = to_string(out.content[2]);

  EXPECT_EQ(std::string("Are you Xnobody, too?\n"), edited_line);
  EXPECT_EQ(std::string("Are you nobody, too?\n"), original_line);
  EXPECT_EQ(std::string("I'm nobody! Who are you?\n"), prev_line);
  EXPECT_EQ(std::string("Then there's a pair of us - don't tell!\n"), next_line);
  }

void test_BufferDeleteCharInMiddleOfExistingLineFromMakeBuffer() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  file_buffer with_cursor = buf;
  with_cursor.pos = position(1, 8);

  file_buffer out = erase(with_cursor, s);

  std::string edited_line = to_string(out.content[1]);
  std::string original_line = to_string(with_cursor.content[1]);
  std::string prev_line = to_string(out.content[0]);
  std::string next_line = to_string(out.content[2]);

  EXPECT_EQ("Are younobody, too?\n", edited_line);
  EXPECT_EQ("Are you nobody, too?\n", original_line);
  EXPECT_EQ("I'm nobody! Who are you?\n", prev_line);
  EXPECT_EQ("Then there's a pair of us - don't tell!\n", next_line);

  out = erase(out, s);
  edited_line = to_string(out.content[1]);
  prev_line = to_string(out.content[0]);
  next_line = to_string(out.content[2]);
  EXPECT_EQ("Are yonobody, too?\n", edited_line);
  EXPECT_EQ("I'm nobody! Who are you?\n", prev_line);
  EXPECT_EQ("Then there's a pair of us - don't tell!\n", next_line);

  out = erase(out, s);
  edited_line = to_string(out.content[1]);
  prev_line = to_string(out.content[0]);
  next_line = to_string(out.content[2]);
  EXPECT_EQ("Are ynobody, too?\n", edited_line);
  EXPECT_EQ("I'm nobody! Who are you?\n", prev_line);
  EXPECT_EQ("Then there's a pair of us - don't tell!\n", next_line);

  out = erase(out, s);
  edited_line = to_string(out.content[1]);
  prev_line = to_string(out.content[0]);
  next_line = to_string(out.content[2]);
  EXPECT_EQ("Are nobody, too?\n", edited_line);
  EXPECT_EQ("I'm nobody! Who are you?\n", prev_line);
  EXPECT_EQ("Then there's a pair of us - don't tell!\n", next_line);

  out = erase(out, s);
  edited_line = to_string(out.content[1]);
  prev_line = to_string(out.content[0]);
  next_line = to_string(out.content[2]);
  EXPECT_EQ("Arenobody, too?\n", edited_line);
  EXPECT_EQ("I'm nobody! Who are you?\n", prev_line);
  EXPECT_EQ("Then there's a pair of us - don't tell!\n", next_line);

  out = erase(out, s);
  edited_line = to_string(out.content[1]);
  prev_line = to_string(out.content[0]);
  next_line = to_string(out.content[2]);
  EXPECT_EQ("Arnobody, too?\n", edited_line);
  EXPECT_EQ("I'm nobody! Who are you?\n", prev_line);
  EXPECT_EQ("Then there's a pair of us - don't tell!\n", next_line);

  out = erase(out, s);
  edited_line = to_string(out.content[1]);
  prev_line = to_string(out.content[0]);
  next_line = to_string(out.content[2]);
  EXPECT_EQ("Anobody, too?\n", edited_line);
  EXPECT_EQ("I'm nobody! Who are you?\n", prev_line);
  EXPECT_EQ("Then there's a pair of us - don't tell!\n", next_line);

  out = erase(out, s);
  edited_line = to_string(out.content[1]);
  prev_line = to_string(out.content[0]);
  next_line = to_string(out.content[2]);
  EXPECT_EQ("nobody, too?\n", edited_line);
  EXPECT_EQ("I'm nobody! Who are you?\n", prev_line);
  EXPECT_EQ("Then there's a pair of us - don't tell!\n", next_line);

  out = erase(out, s);
  edited_line = to_string(out.content[1]);
  prev_line = to_string(out.content[0]);
  next_line = to_string(out.content[2]);
  EXPECT_EQ("Then there's a pair of us - don't tell!\n", edited_line);
  EXPECT_EQ("I'm nobody! Who are you?nobody, too?\n", prev_line);
  EXPECT_EQ("They'd banish us, you know.\n", next_line);

  out = erase(out, s);
  edited_line = to_string(out.content[1]);
  prev_line = to_string(out.content[0]);
  next_line = to_string(out.content[2]);
  EXPECT_EQ("Then there's a pair of us - don't tell!\n", edited_line);
  EXPECT_EQ("I'm nobody! Who are younobody, too?\n", prev_line);
  EXPECT_EQ("They'd banish us, you know.\n", next_line);
  }

void test_BufferInsertNewLineFromMakeBuffer() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  file_buffer with_cursor = buf;
  with_cursor.pos = position(1, 8);

  file_buffer out = insert(with_cursor, L"\n", s);

  std::string edited_line = to_string(out.content[1]);
  std::string original_line = to_string(with_cursor.content[1]);
  std::string prev_line = to_string(out.content[0]);
  std::string next_line = to_string(out.content[2]);

  EXPECT_EQ(std::string("Are you \n"), edited_line);
  EXPECT_EQ(std::string("Are you nobody, too?\n"), original_line);
  EXPECT_EQ(std::string("I'm nobody! Who are you?\n"), prev_line);
  EXPECT_EQ(std::string("nobody, too?\n"), next_line);
  }

void test_BufferInsertNewLineFromMakeBufferAtEnd() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  file_buffer with_cursor = buf;
  with_cursor.pos = position(1, 80);

  file_buffer out = insert(with_cursor, L"\n", s);

  std::string edited_line = to_string(out.content[1]);
  std::string original_line = to_string(with_cursor.content[1]);
  std::string prev_line = to_string(out.content[0]);
  std::string next_line = to_string(out.content[2]);

  EXPECT_EQ("Are you nobody, too?\n", edited_line);
  EXPECT_EQ("Are you nobody, too?\n", original_line);
  EXPECT_EQ("I'm nobody! Who are you?\n", prev_line);
  EXPECT_EQ("\n", next_line);
  }

void test_BufferInsertNewLineFromMakeBufferAtBegin() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  file_buffer with_cursor = buf;
  with_cursor.pos = position(1, 0);

  file_buffer out = insert(with_cursor, L"\n", s);

  std::string edited_line = to_string(out.content[1]);
  std::string original_line = to_string(with_cursor.content[1]);
  std::string prev_line = to_string(out.content[0]);
  std::string next_line = to_string(out.content[2]);

  EXPECT_EQ("\n", edited_line);
  EXPECT_EQ("Are you nobody, too?\n", original_line);
  EXPECT_EQ("I'm nobody! Who are you?\n", prev_line);
  EXPECT_EQ("Are you nobody, too?\n", next_line);
  }

void test_BufferInsertTextSingleLineInMiddleFromMakeBuffer() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  file_buffer with_cursor = buf;
  with_cursor.pos = position(1, 8);

  text paste = to_text("very ");

  file_buffer out = insert(with_cursor, paste, s);

  std::string edited_line = to_string(out.content[1]);
  std::string original_line = to_string(with_cursor.content[1]);
  std::string prev_line = to_string(out.content[0]);
  std::string next_line = to_string(out.content[2]);

  EXPECT_EQ("Are you very nobody, too?\n", edited_line);
  EXPECT_EQ("Are you nobody, too?\n", original_line);
  EXPECT_EQ("I'm nobody! Who are you?\n", prev_line);
  EXPECT_EQ("Then there's a pair of us - don't tell!\n", next_line);
  EXPECT_TRUE(position(1, 13)==out.pos);
  }

void test_BufferInsertTextMultiLineInMiddleFromMakeBuffer() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  file_buffer with_cursor = buf;
  with_cursor.pos = position(1, 8);

  text paste = to_text("kind \nfriend");

  file_buffer out = insert(with_cursor, paste, s);

  std::string line_1 = to_string(out.content[1]);
  std::string line_2 = to_string(out.content[2]);
  std::string shifted_next_line = to_string(out.content[3]);

  EXPECT_EQ("Are you kind \n", line_1);
  EXPECT_EQ("friendnobody, too?\n", line_2);
  EXPECT_EQ("Then there's a pair of us - don't tell!\n", shifted_next_line);
  EXPECT_TRUE(position(2, 6) == out.pos);
  }

void test_BufferInsertTextAppendsWhenCursorIsAtBufferEnd() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  file_buffer with_cursor = buf;
  with_cursor.pos = position(with_cursor.content.size(), 0);
  int64_t end_row = with_cursor.content.size();

  text paste = to_text("PS:\nnobody is perfect");

  file_buffer out = insert(with_cursor, paste, s);

  EXPECT_EQ(with_cursor.content.size() + 1, out.content.size());
  EXPECT_EQ("To an admiring bog!PS:\n", to_string(out.content[out.content.size() - 2]));
  EXPECT_EQ("nobody is perfect", to_string(out.content[out.content.size() - 1]));
  EXPECT_TRUE(position(end_row, 17) == out.pos);
  }

void test_BufferSelection1() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(1, 8);
  buf.xpos = get_x_position(buf, s);
  buf = start_selection(buf);
  buf = move_down(buf, s);
  text t = get_selection(buf, s);

  EXPECT_EQ(2, t.size());
  EXPECT_EQ("nobody, too?\n", to_string(t[0]));
  EXPECT_EQ("Then ther", to_string(t[1]));
  }

void test_BufferSelection2() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(1, 8);
  buf = start_selection(buf);
  buf = move_right(buf, s);
  text t = get_selection(buf, s);

  EXPECT_EQ(1, t.size());
  EXPECT_EQ("no", to_string(t[0]));
  }

void test_BufferSelection3() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(1, 8);
  buf = start_selection(buf);
  buf = move_end(buf, s);
  text t = get_selection(buf, s);

  EXPECT_EQ(1, t.size());
  EXPECT_EQ("nobody, too?", to_string(t[0]));
  }

void test_BufferCut1() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(1, 8);
  buf.xpos = get_x_position(buf, s);
  buf = start_selection(buf);
  buf = move_down(buf, s);
  buf = insert(buf, L"\n", s); // assumes insert replaces selection

  EXPECT_EQ(9, buf.content.size());
  EXPECT_EQ("Are you \n", to_string(buf.content[1]));
  EXPECT_EQ("e's a pair of us - don't tell!\n", to_string(buf.content[2]));
  EXPECT_EQ("They'd banish us, you know.\n", to_string(buf.content[3]));
  }

void test_BufferCut2() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(1, 8);
  buf.xpos = get_x_position(buf, s);
  buf = start_selection(buf);
  buf = move_down(buf, s);
  buf = insert(buf, L"", s); // assumes insert replaces selection

  EXPECT_EQ(8, buf.content.size());
  EXPECT_EQ("Are you e's a pair of us - don't tell!\n", to_string(buf.content[1])); 
  EXPECT_EQ("They'd banish us, you know.\n", to_string(buf.content[2]));
  }

void test_BufferCut3() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(1, 8);
  buf = start_selection(buf);
  buf = move_right(buf, s);
  buf = move_right(buf, s);
  buf = insert(buf, L"", s); // assumes insert replaces selection

  EXPECT_EQ(9, buf.content.size());
  EXPECT_EQ("Are you ody, too?\n", to_string(buf.content[1]));
  EXPECT_EQ("Then there's a pair of us - don't tell!\n", to_string(buf.content[2]));
  }
  
JEDLIB_END

void run_buffer_tests() {
  using namespace JEDLIB;
  test_BufferLineLength();
  test_BufferLineLengthUnicoded();
  test_BufferTestsFailRead();
  test_BufferTestsReadWrite();
  test_BufferInsertCharInsertsAtCursorWithoutMutatingOriginal();
  test_BufferInsertCharInMiddleOfExistingLineFromMakeBuffer();
  test_BufferDeleteCharInMiddleOfExistingLineFromMakeBuffer();
  test_BufferInsertNewLineFromMakeBuffer();
  test_BufferInsertNewLineFromMakeBufferAtEnd();
  test_BufferInsertNewLineFromMakeBufferAtBegin();
  test_BufferInsertTextSingleLineInMiddleFromMakeBuffer();
  test_BufferInsertTextMultiLineInMiddleFromMakeBuffer();
  test_BufferInsertTextAppendsWhenCursorIsAtBufferEnd();
  test_BufferSelection1();
  test_BufferSelection2();
  test_BufferSelection3();
  test_BufferCut1();
  test_BufferCut2();
  test_BufferCut3();
  }
