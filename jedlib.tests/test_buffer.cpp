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
  std::string s;
  s.push_back('a');
  s.push_back(0xcf);//CF 80 is utf8 for pi
  s.push_back(0x80);
  s.push_back('c');
  line ln = make_line(s);
  EXPECT_EQ(3, line_length(ln));
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

void test_BufferCharacterWidth() {
  env_settings s = make_settings();

  EXPECT_EQ(1, character_width((uint32_t)L'a', 0, s));
  EXPECT_EQ(4, character_width(9, 0, s));
  EXPECT_EQ(3, character_width(9, 1, s));
  EXPECT_EQ(1, character_width(9, 3, s));
  EXPECT_EQ(1, character_width(10, 0, s));
  EXPECT_EQ(1, character_width(13, 0, s));

  s.show_all_characters = true;
  EXPECT_EQ(2, character_width(10, 0, s));
  EXPECT_EQ(2, character_width(13, 0, s));
  }

void test_BufferLineLengthUpToColumn() {
  env_settings s = make_settings();

  line ln = make_line("abc");
  EXPECT_EQ(1, line_length_up_to_column(ln, 0, s));
  EXPECT_EQ(3, line_length_up_to_column(ln, 2, s));
  EXPECT_EQ(3, line_length_up_to_column(ln, 10, s));

  line tabbed = make_line("\tab");
  EXPECT_EQ(4, line_length_up_to_column(tabbed, 0, s));
  EXPECT_EQ(5, line_length_up_to_column(tabbed, 1, s));
  EXPECT_EQ(6, line_length_up_to_column(tabbed, 2, s));
  }

void test_BufferGetColFromLineLength() {
  env_settings s = make_settings();

  line ln = make_line("abc");
  EXPECT_EQ(0, get_col_from_line_length(ln, 0, s));
  EXPECT_EQ(2, get_col_from_line_length(ln, 2, s));

  line tabbed = make_line("\tab");
  EXPECT_EQ(1, get_col_from_line_length(tabbed, 4, s));
  EXPECT_EQ(2, get_col_from_line_length(tabbed, 5, s));
  }

void test_BufferGetXPosition() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(1, 8);
  EXPECT_EQ(8, get_x_position(buf, s));

  file_buffer empty = make_empty_buffer();
  EXPECT_EQ(0, get_x_position(empty, s));
  }

void test_BufferGetActualPosition() {
  file_buffer buf = make_buffer();

  buf.pos = position(1, 8);
  EXPECT_TRUE(position(1, 8) == get_actual_position(buf));

  buf.pos = position(1, 100);
  EXPECT_TRUE(position(1, 20) == get_actual_position(buf));

  buf.pos = position(8, 100);
  EXPECT_TRUE(position(8, 19) == get_actual_position(buf));

  buf.pos = position(100, 0);
  EXPECT_TRUE(position(8, 19) == get_actual_position(buf));

  file_buffer empty = make_empty_buffer();
  empty.pos = position(5, 5);
  EXPECT_TRUE(position(0, 0) == get_actual_position(empty));
  }

void test_BufferGetLastPosition() {
  file_buffer buf = make_buffer();
  EXPECT_TRUE(position(8, 19) == get_last_position(buf));

  EXPECT_TRUE(position(0, 0) == get_last_position(text()));
  EXPECT_TRUE(position(0, 3) == get_last_position(to_text("abc")));
  EXPECT_TRUE(position(0, 3) == get_last_position(to_text("abc\n")));
  }

void test_BufferStartAndClearSelection() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(1, 8);
  EXPECT_TRUE(!has_selection(buf));

  buf = start_selection(buf);
  buf = move_right(buf, s);
  EXPECT_TRUE(has_selection(buf));
  EXPECT_TRUE(has_nontrivial_selection(buf, s));
  EXPECT_TRUE(!has_multiline_selection(buf));
  EXPECT_TRUE(!has_rectangular_selection(buf));

  buf = clear_selection(buf);
  EXPECT_TRUE(!has_selection(buf));
  EXPECT_TRUE(!buf.rectangular_selection);
  }

void test_BufferHasMultilineSelection() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(1, 8);
  buf.xpos = get_x_position(buf, s);
  buf = start_selection(buf);
  buf = move_down(buf, s);
  EXPECT_TRUE(has_multiline_selection(buf));
  }

void test_BufferInSelection() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(1, 8);
  buf = start_selection(buf);
  buf = move_right(buf, s);
  buf = move_right(buf, s);

  EXPECT_TRUE(in_selection(buf, position(1, 8), buf.pos, buf.pos, buf.start_selection, false, s));
  EXPECT_TRUE(in_selection(buf, position(1, 9), buf.pos, buf.pos, buf.start_selection, false, s));
  EXPECT_TRUE(!in_selection(buf, position(1, 7), buf.pos, buf.pos, buf.start_selection, false, s));
  EXPECT_TRUE(!in_selection(buf, position(1, 8), buf.pos, buf.pos, std::nullopt, false, s));
  }

void test_BufferSelectAll() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf = select_all(buf, s);

  EXPECT_TRUE(position(0, 0) == *buf.start_selection);
  EXPECT_TRUE(position(8, 19) == buf.pos);

  text t = get_selection(buf, s);
  EXPECT_EQ(to_string(buf.content), to_string(t));
  }

void test_BufferGetSelectionWithoutSelection() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(1, 8);
  text t = get_selection(buf, s);

  EXPECT_EQ(1, t.size());
  EXPECT_EQ("n", to_string(t[0]));
  }

void test_BufferEraseRight() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(1, 8);

  file_buffer out = erase_right(buf, s);
  EXPECT_EQ("Are you obody, too?\n", to_string(out.content[1]));
  EXPECT_TRUE(position(1, 8) == out.pos);

  // erase at end of line joins the next line
  file_buffer at_end = make_buffer();
  at_end.pos = position(1, 20);
  file_buffer joined = erase_right(at_end, s);
  EXPECT_EQ(8, joined.content.size());
  EXPECT_EQ("Are you nobody, too?Then there's a pair of us - don't tell!\n", to_string(joined.content[1]));
  }

void test_BufferEraseSelection() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(1, 8);
  buf = start_selection(buf);
  buf = move_right(buf, s);
  buf = move_right(buf, s);
  buf = erase(buf, s);

  EXPECT_EQ(9, buf.content.size());
  EXPECT_EQ("Are you ody, too?\n", to_string(buf.content[1]));
  EXPECT_TRUE(!has_selection(buf));
  }

void test_BufferEraseOnEmptyBuffer() {
  env_settings s = make_settings();

  file_buffer buf = make_empty_buffer();
  file_buffer out = erase(buf, s);
  EXPECT_TRUE(out.content.empty());
  }

void test_BufferUndoRedo() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(1, 8);
  std::string original = buffer_to_string(buf);

  file_buffer edited = insert(buf, L"X", s);
  std::string edited_string = buffer_to_string(edited);
  EXPECT_EQ("Are you Xnobody, too?\n", to_string(edited.content[1]));

  file_buffer undone = undo(edited, s);
  EXPECT_EQ(original, buffer_to_string(undone));

  file_buffer redone = redo(undone, s);
  EXPECT_EQ(edited_string, buffer_to_string(redone));
  }

void test_BufferPushUndo() {
  file_buffer buf = make_buffer();
  buf.pos = position(1, 8);

  EXPECT_EQ(0, buf.history.size());
  buf = push_undo(buf);
  EXPECT_EQ(1, buf.history.size());
  EXPECT_EQ(1, buf.undo_redo_index);
  EXPECT_EQ(to_string(buf.content), to_string(buf.history[0].content));
  }

void test_BufferMoveLeftRight() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(0, 23);

  buf = move_right(buf, s);
  EXPECT_TRUE(position(0, 24) == buf.pos);
  buf = move_right(buf, s);
  EXPECT_TRUE(position(1, 0) == buf.pos);
  buf = move_left(buf, s);
  EXPECT_TRUE(position(0, 25) == buf.pos);

  file_buffer begin = make_buffer();
  begin.pos = position(0, 0);
  begin = move_left(begin, s);
  EXPECT_TRUE(position(0, 0) == begin.pos);

  file_buffer last = make_buffer();
  last.pos = position(8, 18);
  last = move_right(last, s);
  EXPECT_TRUE(position(8, 19) == last.pos);
  last = move_right(last, s);
  EXPECT_TRUE(position(8, 19) == last.pos);

  file_buffer empty = make_empty_buffer();
  EXPECT_TRUE(position(0, 0) == move_left(empty, s).pos);
  EXPECT_TRUE(position(0, 0) == move_right(empty, s).pos);
  }

void test_BufferMoveUpDown() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(1, 8);
  buf.xpos = get_x_position(buf, s);

  buf = move_down(buf, s);
  EXPECT_TRUE(position(2, 8) == buf.pos);
  buf = move_up(buf, s);
  EXPECT_TRUE(position(1, 8) == buf.pos);

  file_buffer top = make_buffer();
  top.pos = position(0, 3);
  top.xpos = get_x_position(top, s);
  top = move_up(top, s);
  EXPECT_TRUE(position(0, 3) == top.pos);

  file_buffer bottom = make_buffer();
  bottom.pos = position(8, 3);
  bottom.xpos = get_x_position(bottom, s);
  bottom = move_down(bottom, s);
  EXPECT_TRUE(position(8, 3) == bottom.pos);
  }

void test_BufferMovePageUpDown() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(1, 3);
  buf.xpos = get_x_position(buf, s);

  buf = move_page_down(buf, 3, s);
  EXPECT_EQ(4, buf.pos.row);

  buf = move_page_up(buf, 2, s);
  EXPECT_EQ(2, buf.pos.row);

  buf = move_page_up(buf, 100, s);
  EXPECT_EQ(0, buf.pos.row);

  buf = move_page_down(buf, 100, s);
  EXPECT_EQ(8, buf.pos.row);
  }

void test_BufferMoveHomeEnd() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(1, 8);
  buf = move_end(buf, s);
  EXPECT_TRUE(position(1, 20) == buf.pos);

  buf = move_home(buf, s);
  EXPECT_TRUE(position(1, 0) == buf.pos);
  EXPECT_EQ(0, buf.xpos);

  file_buffer last = make_buffer();
  last.pos = position(8, 0);
  last = move_end(last, s);
  EXPECT_TRUE(position(8, 19) == last.pos);

  file_buffer indented = make_empty_buffer();
  indented.content = to_text("    indented\n");
  indented.pos = position(0, 8);
  indented = move_home(indented, s);
  EXPECT_TRUE(position(0, 4) == indented.pos);
  indented = move_home(indented, s);
  EXPECT_TRUE(position(0, 0) == indented.pos);
  }

void test_BufferUpdatePosition() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf = update_position(buf, position(2, 5), s);
  EXPECT_TRUE(position(2, 5) == buf.pos);
  EXPECT_EQ(5, buf.xpos);

  buf = update_position(buf, position(100, 2), s);
  EXPECT_EQ(8, buf.pos.row);
  }

void test_BufferToStringConversions() {
  file_buffer buf = make_buffer();

  EXPECT_EQ(to_string(buf.content), buffer_to_string(buf));
  EXPECT_EQ("I'm", to_string(buf.content, position(0, 0), position(0, 3)));
  EXPECT_TRUE(to_wstring(to_text("abc")) == std::wstring(L"abc"));
  EXPECT_TRUE(to_wstring(buf.content, position(0, 0), position(0, 3)) == std::wstring(L"I'm"));
  EXPECT_EQ("abc", to_string(make_line("abc")));
  }

void test_BufferGetNextAndPreviousPosition() {
  file_buffer buf = make_buffer();

  EXPECT_TRUE(position(0, 1) == get_next_position(buf, position(0, 0)));
  EXPECT_TRUE(position(1, 0) == get_next_position(buf, position(0, 24)));
  EXPECT_TRUE(position(0, 24) == get_previous_position(buf, position(1, 0)));
  EXPECT_TRUE(position(0, 0) == get_previous_position(buf, position(0, 1)));
  EXPECT_TRUE(get_last_position(buf) == get_next_position(buf, get_last_position(buf)));
  }

void test_BufferValidPosition() {
  file_buffer buf = make_buffer();

  EXPECT_TRUE(valid_position(buf, position(0, 0)));
  EXPECT_TRUE(valid_position(buf, position(8, 18)));
  EXPECT_TRUE(!valid_position(buf, position(-1, 0)));
  EXPECT_TRUE(!valid_position(buf, position(0, -1)));
  EXPECT_TRUE(!valid_position(buf, position(100, 0)));
  EXPECT_TRUE(!valid_position(buf, position(0, 25)));
  }

void test_BufferFindNextOccurence() {
  file_buffer buf = make_buffer();

  EXPECT_TRUE(position(0, 4) == find_next_occurence(buf.content, position(0, 0), L'n'));
  EXPECT_TRUE(position(0, 4) == find_next_occurence(buf, position(0, 0), L'n'));
  EXPECT_TRUE(position(-1, -1) == find_next_occurence(buf.content, position(0, 0), L'@'));

  EXPECT_TRUE(position(0, 4) == find_next_occurence(buf.content, position(0, 0), std::wstring(L"nobody")));
  EXPECT_TRUE(position(-1, -1) == find_next_occurence(buf.content, position(0, 0), std::wstring(L"somewhere")));
  EXPECT_TRUE(position(0, 4) == find_next_occurence_reverse(buf.content, position(1, 0), std::wstring(L"nobody")));
  }

void test_BufferFindText() {
  file_buffer buf = make_buffer();
  buf.pos = position(0, 0);

  file_buffer found = find_text(buf, std::string("nobody"));
  EXPECT_TRUE(position(0, 4) == *found.start_selection);
  EXPECT_TRUE(position(0, 9) == found.pos);

  file_buffer found_w = find_text(buf, std::wstring(L"nobody"));
  EXPECT_TRUE(position(0, 4) == *found_w.start_selection);

  file_buffer not_found = find_text(buf, std::string("nothing to find here"));
  EXPECT_TRUE(get_last_position(buf) == not_found.pos);
  EXPECT_TRUE(!not_found.start_selection.has_value());

  file_buffer unchanged = find_text(buf, text());
  EXPECT_TRUE(position(0, 0) == unchanged.pos);
  }

void test_BufferFindTextCaseInsensitive() {
  file_buffer buf = make_buffer();
  buf.pos = position(0, 0);

  file_buffer found = find_text_case_insensitive(buf, std::string("NOBODY"));
  EXPECT_TRUE(position(0, 4) == *found.start_selection);
  EXPECT_TRUE(position(0, 9) == found.pos);

  file_buffer found_w = find_text_case_insensitive(buf, std::wstring(L"NoBoDy"));
  EXPECT_TRUE(position(0, 4) == *found_w.start_selection);

  file_buffer not_found = find_text_case_insensitive(buf, std::string("NOTHING TO FIND"));
  EXPECT_TRUE(!not_found.start_selection.has_value());
  }

void test_BufferReadNextWord() {
  line ln = make_line("hello world");
  EXPECT_TRUE(read_next_word(ln.begin(), ln.end()) == std::wstring(L"hello"));

  line separators = make_line("(abc)");
  EXPECT_TRUE(read_next_word(separators.begin(), separators.end()) == std::wstring(L""));
  }

void test_BufferFindCorrespondingToken() {
  file_buffer buf = make_empty_buffer();
  buf.content = to_text("(a(b)c)");

  EXPECT_TRUE(position(0, 6) == find_corresponding_token(buf, position(0, 0), 0, 0));
  EXPECT_TRUE(position(0, 0) == find_corresponding_token(buf, position(0, 6), 0, 0));
  EXPECT_TRUE(position(0, 4) == find_corresponding_token(buf, position(0, 2), 0, 0));
  EXPECT_TRUE(position(-1, -1) == find_corresponding_token(buf, position(0, 1), 0, 0));
  EXPECT_TRUE(position(-1, -1) == find_corresponding_token(buf, position(100, 0), 0, 0));
  }

void test_BufferIndentation() {
  file_buffer buf = make_empty_buffer();
  buf.content = to_text("    indented\nno indentation\n");

  EXPECT_TRUE(position(0, 4) == get_indentation_at_row(buf, 0));
  EXPECT_TRUE(position(1, 0) == get_indentation_at_row(buf, 1));

  file_buffer only_spaces = make_empty_buffer();
  only_spaces.content = to_text("    \nx");
  EXPECT_EQ("    ", get_row_indentation_pattern(only_spaces, position(0, 4)));
  EXPECT_EQ("", get_row_indentation_pattern(buf, position(0, 4)));
  EXPECT_EQ("", get_row_indentation_pattern(buf, position(100, 0)));
  }

void test_BufferLexerStatus() {
  env_settings s = make_settings();
  s.perform_syntax_highlighting = true;

  file_buffer buf = make_empty_buffer();
  buf.syntax.should_highlight = true;
  buf.syntax.single_line = "//";
  buf.syntax.multiline_begin = "/*";
  buf.syntax.multiline_end = "*/";
  buf.content = to_text(
    "int a; // comment\n"
    "/*\n"
    "blah\n"
    "*/\n"
    "done"
  );

  buf = init_lexer_status(buf, s);
  EXPECT_EQ(5, buf.lex.size());
  EXPECT_EQ(lexer_normal, buf.lex[0]);
  EXPECT_EQ(lexer_normal, buf.lex[1]);
  EXPECT_EQ(lexer_inside_multiline_comment, buf.lex[2]);
  EXPECT_EQ(lexer_inside_multiline_comment, buf.lex[3]);
  EXPECT_EQ(lexer_normal, buf.lex[4]);

  EXPECT_EQ(lexer_normal, get_end_of_line_lexer_status(buf, 0));
  EXPECT_EQ(lexer_inside_multiline_comment, get_end_of_line_lexer_status(buf, 1));

  buf = update_lexer_status(buf, 0, s);
  EXPECT_EQ(lexer_inside_multiline_comment, buf.lex[2]);
  buf = update_lexer_status(buf, 0, 4, s);
  EXPECT_EQ(lexer_normal, buf.lex[4]);
  }

void test_BufferGetTextType() {
  env_settings s = make_settings();

  file_buffer buf = make_empty_buffer();
  buf.syntax.should_highlight = true;
  buf.syntax.single_line = "//";
  buf.syntax.multiline_begin = "/*";
  buf.syntax.multiline_end = "*/";
  buf.content = to_text(
    "int a; // comment\n"
    "/*\n"
    "blah\n"
    "*/\n"
    "done"
  );

  auto tt_off = get_text_type(buf, 0, s);
  EXPECT_EQ(1, tt_off.size());
  EXPECT_EQ(0, tt_off[0].first);
  EXPECT_EQ(tt_normal, tt_off[0].second);

  s.perform_syntax_highlighting = true;
  buf = init_lexer_status(buf, s);

  auto tt_line = get_text_type(buf, 0, s);
  EXPECT_EQ(2, tt_line.size());
  EXPECT_EQ(7, tt_line[0].first);
  EXPECT_EQ(tt_comment, tt_line[0].second);
  EXPECT_EQ(0, tt_line[1].first);
  EXPECT_EQ(tt_normal, tt_line[1].second);

  auto tt_multi = get_text_type(buf, 2, s);
  EXPECT_EQ(1, tt_multi.size());
  EXPECT_EQ(tt_comment, tt_multi[0].second);
  }

void test_BufferRectangularSelectionInsert() {
  env_settings s = make_settings();

  file_buffer buf = make_empty_buffer();
  buf.content = to_text("abcd\nefgh\nijkl");
  buf.start_selection = position(0, 2);
  buf.pos = position(2, 2);
  buf.rectangular_selection = true;

  EXPECT_TRUE(has_rectangular_selection(buf));
  EXPECT_TRUE(has_trivial_rectangular_selection(buf, s));
  EXPECT_TRUE(!has_nontrivial_selection(buf, s));

  file_buffer out = insert(buf, L"X", s);
  EXPECT_EQ("abXcd\n", to_string(out.content[0]));
  EXPECT_EQ("efXgh\n", to_string(out.content[1]));
  EXPECT_EQ("ijXkl", to_string(out.content[2]));
  EXPECT_TRUE(out.rectangular_selection);
  }

void test_BufferRectangularSelectionGetSelection() {
  env_settings s = make_settings();

  file_buffer buf = make_empty_buffer();
  buf.content = to_text("abcd\nefgh\nijkl");
  buf.start_selection = position(0, 1);
  buf.pos = position(2, 2);
  buf.rectangular_selection = true;

  EXPECT_TRUE(has_nontrivial_selection(buf, s));

  text t = get_selection(buf, s);
  EXPECT_EQ(3, t.size());
  EXPECT_EQ("bc\n", to_string(t[0]));
  EXPECT_EQ("fg\n", to_string(t[1]));
  EXPECT_EQ("jk", to_string(t[2]));
  }

void test_BufferRectangularSelectionErase() {
  env_settings s = make_settings();

  file_buffer buf = make_empty_buffer();
  buf.content = to_text("abcd\nefgh\nijkl");
  buf.start_selection = position(0, 1);
  buf.pos = position(2, 3);
  buf.rectangular_selection = true;

  file_buffer out = erase(buf, s);
  EXPECT_EQ("a\n", to_string(out.content[0]));
  EXPECT_EQ("e\n", to_string(out.content[1]));
  EXPECT_EQ("i", to_string(out.content[2]));
  }

void test_BufferGetRectangularSelection() {
  env_settings s = make_settings();

  file_buffer buf = make_empty_buffer();
  buf.content = to_text("abcd\nefgh\nijkl");

  int64_t minrow, maxrow, minx, maxx;
  get_rectangular_selection(minrow, maxrow, minx, maxx, buf, position(2, 3), position(0, 1), s);
  EXPECT_EQ(0, minrow);
  EXPECT_EQ(2, maxrow);
  EXPECT_EQ(1, minx);
  EXPECT_EQ(3, maxx);
  }

void test_BufferInsertString() {
  env_settings s = make_settings();

  file_buffer buf = make_buffer();
  buf.pos = position(1, 8);

  file_buffer out = insert(buf, std::string("very "), s);
  EXPECT_EQ("Are you very nobody, too?\n", to_string(out.content[1]));
  EXPECT_TRUE(position(1, 13) == out.pos);
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
  test_BufferCharacterWidth();
  test_BufferLineLengthUpToColumn();
  test_BufferGetColFromLineLength();
  test_BufferGetXPosition();
  test_BufferGetActualPosition();
  test_BufferGetLastPosition();
  test_BufferStartAndClearSelection();
  test_BufferHasMultilineSelection();
  test_BufferInSelection();
  test_BufferSelectAll();
  test_BufferGetSelectionWithoutSelection();
  test_BufferEraseRight();
  test_BufferEraseSelection();
  test_BufferEraseOnEmptyBuffer();
  test_BufferUndoRedo();
  test_BufferPushUndo();
  test_BufferMoveLeftRight();
  test_BufferMoveUpDown();
  test_BufferMovePageUpDown();
  test_BufferMoveHomeEnd();
  test_BufferUpdatePosition();
  test_BufferToStringConversions();
  test_BufferGetNextAndPreviousPosition();
  test_BufferValidPosition();
  test_BufferFindNextOccurence();
  test_BufferFindText();
  test_BufferFindTextCaseInsensitive();
  test_BufferReadNextWord();
  test_BufferFindCorrespondingToken();
  test_BufferIndentation();
  test_BufferLexerStatus();
  test_BufferGetTextType();
  test_BufferRectangularSelectionInsert();
  test_BufferRectangularSelectionGetSelection();
  test_BufferRectangularSelectionErase();
  test_BufferGetRectangularSelection();
  test_BufferInsertString();
  }
