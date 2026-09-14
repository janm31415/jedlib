#include "test_edit.h"
#include "test_assert.h"
#include "jedlib/edit.h"

#include <fstream>

JEDLIB_BEGIN

namespace {

env_settings make_edit_settings() {
  env_settings s;
  s.show_all_characters = false;
  s.tab_space = 8;
  s.perform_syntax_highlighting = false;
  return s;
}

std::string command_error(file_buffer fb, const std::string& command, const env_settings& s) {
  try {
    handle_command(fb, command, s);
  }
  catch (std::runtime_error& e) {
    return std::string(e.what());
  }
  return std::string();
}

} // namespace

void parse_test_1() {

  std::string command = "3";
  auto tokens = tokenize(command);
  EXPECT_EQ(1, tokens.size());
  EXPECT_EQ(token::T_NUMBER, tokens.front().type);
  EXPECT_EQ(std::string("3"), tokens.front().value);
  auto expr = parse(tokens);
  EXPECT_EQ(1, expr.size());
  EXPECT_TRUE(std::holds_alternative<AddressRange>(expr.front()));
  EXPECT_TRUE(std::get<AddressRange>(expr.front()).fops.empty());
  EXPECT_TRUE(std::holds_alternative<LineNumber>(std::get<AddressRange>(expr.front()).operands.front().operands.front()));
  EXPECT_EQ(3, std::get<LineNumber>(std::get<AddressRange>(expr.front()).operands.front().operands.front()).value);
}

void parse_test_2() {
  std::string command = ".";
  auto tokens = tokenize(command);
  EXPECT_EQ(1, tokens.size());
  EXPECT_EQ(token::T_DOT, tokens.front().type);
  auto expr = parse(tokens);
  EXPECT_EQ(1, expr.size());
  EXPECT_TRUE(std::holds_alternative<AddressRange>(expr.front()));
  EXPECT_TRUE(std::get<AddressRange>(expr.front()).fops.empty());
  EXPECT_TRUE(std::holds_alternative<Dot>(std::get<AddressRange>(expr.front()).operands.front().operands.front()));
}

void parse_test_3() {
  std::string command = "$";
  auto tokens = tokenize(command);
  EXPECT_EQ(1, tokens.size());
  EXPECT_EQ(token::T_DOLLAR, tokens.front().type);
  auto expr = parse(tokens);
  EXPECT_EQ(1, expr.size());
  EXPECT_TRUE(std::holds_alternative<AddressRange>(expr.front()));
  EXPECT_TRUE(std::get<AddressRange>(expr.front()).fops.empty());
  EXPECT_TRUE(std::holds_alternative<EndOfFile>(std::get<AddressRange>(expr.front()).operands.front().operands.front()));
}

void parse_test_4() {
  std::string command = "$-3";
  auto tokens = tokenize(command);
  EXPECT_EQ(3, tokens.size());
  EXPECT_EQ(token::T_DOLLAR, tokens[0].type);
  EXPECT_EQ(token::T_MINUS, tokens[1].type);
  EXPECT_EQ(token::T_NUMBER, tokens[2].type);
  auto expr = parse(tokens);
  EXPECT_EQ(1, expr.size());
  EXPECT_TRUE(std::holds_alternative<AddressRange>(expr.front()));
  EXPECT_TRUE(std::get<AddressRange>(expr.front()).fops.empty());
  EXPECT_TRUE(std::holds_alternative<EndOfFile>(std::get<AddressRange>(expr.front()).operands.front().operands[0]));
  EXPECT_TRUE(std::holds_alternative<LineNumber>(std::get<AddressRange>(expr.front()).operands.front().operands[1]));
  EXPECT_EQ(1, std::get<AddressRange>(expr.front()).operands.front().fops.size());
  EXPECT_TRUE(std::get<AddressRange>(expr.front()).operands.front().fops[0]==std::string("-"));
}

void parse_test_5() {
  std::string command = "c/Jan/";
  auto tokens = tokenize(command);
  EXPECT_EQ(4, tokens.size());
  EXPECT_EQ(token::T_COMMAND, tokens[0].type);
  EXPECT_EQ(token::T_DELIMITER_SLASH, tokens[1].type);
  EXPECT_EQ(token::T_TEXT, tokens[2].type);
  EXPECT_EQ(token::T_DELIMITER_SLASH, tokens[3].type);
  auto expr = parse(tokens);
  EXPECT_EQ(1, expr.size());
  EXPECT_TRUE(std::holds_alternative<Command>(expr.front()));
  EXPECT_TRUE(std::holds_alternative<Cmd_c>(std::get<Command>(expr.front())));
  EXPECT_TRUE(std::get<Cmd_c>(std::get<Command>(expr.front())).txt.text == std::string("Jan"));
  }
  
void parse_test_6() {
  std::string command = "c";
  auto tokens = tokenize(command);
  EXPECT_EQ(1, tokens.size());
  EXPECT_EQ(token::T_COMMAND, tokens[0].type);
  try {
  auto expr = parse(tokens);
  } catch (std::runtime_error& e) {
    EXPECT_EQ(std::string("I expect a token: /"), std::string(e.what()));
  }
}

void handle_command_test_1() {
  env_settings s;
  s.show_all_characters = false;
  s.tab_space = 8;
  file_buffer fb = make_empty_buffer();
  fb = insert(fb, "The quick brown fox jumps over the lazy dog", s);
  EXPECT_TRUE(fb.start_selection == std::nullopt);
  EXPECT_TRUE(fb.pos == get_last_position(fb));
  fb = handle_command(fb, "1", s);
  EXPECT_TRUE(fb.start_selection == position(0, 0));
  EXPECT_TRUE(fb.pos == position(0, fb.content[0].size()-1));
  fb = handle_command(fb, "#1", s);
  EXPECT_TRUE(fb.start_selection == std::nullopt);
  EXPECT_TRUE(fb.pos == position(0, 1));
  fb = handle_command(fb, "#0", s);
  EXPECT_TRUE(fb.start_selection == std::nullopt);
  EXPECT_TRUE(fb.pos == position(0, 0));
  fb = handle_command(fb, ",", s);
  EXPECT_TRUE(fb.start_selection == position(0, 0));
  EXPECT_TRUE(fb.pos == position(0, fb.content[0].size()));
  fb = make_empty_buffer();
  fb = handle_command(fb, "a/ABCDE/", s);
  fb = handle_command(fb, "/B/", s);
  EXPECT_TRUE(fb.pos == position(0,1));
  EXPECT_TRUE(*fb.start_selection == position(0,1));
  fb = handle_command(fb, "#0,#0", s);
  EXPECT_TRUE(fb.start_selection == std::nullopt);
  EXPECT_TRUE(fb.pos == position(0, 0));
  fb = handle_command(fb, "#0,#1", s);
  EXPECT_TRUE(fb.start_selection == position(0, 0));
  EXPECT_TRUE(fb.pos == position(0, 0));
  fb = handle_command(fb, ", c/A/", s);
  fb = handle_command(fb, "#0,#1", s);
  EXPECT_TRUE(fb.start_selection == position(0, 0));
  EXPECT_TRUE(fb.pos == position(0, 1));
}

void handle_command_test_2() {
  env_settings s;
  s.show_all_characters = false;
  s.tab_space = 8;
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/Hello world/", s);
  EXPECT_TRUE(to_string(fb.content)==std::string("Hello world"));
  fb = handle_command(fb, "a/!/", s);
  EXPECT_TRUE(to_string(fb.content)==std::string("Hello world!"));
  fb = handle_command(fb, "a/\\nHere is a newline./", s);
  EXPECT_TRUE(to_string(fb.content)==std::string("Hello world!\nHere is a newline."));
  fb.pos = position(0, 0);
  fb.start_selection = position(0, 10);
  fb = handle_command(fb, "a/INS/", s);
  EXPECT_TRUE(to_string(fb.content)==std::string("Hello worlINSd!\nHere is a newline."));
}

void handle_command_test_3() {
  env_settings s;
  s.show_all_characters = false;
  s.tab_space = 8;
  file_buffer fb = make_empty_buffer();
  fb = insert(fb, "The quick brown fox jumps over the lazy dog", s);
  fb = handle_command(fb, "/q.*k/", s);
  EXPECT_TRUE(fb.start_selection == position(0, 4));
  EXPECT_TRUE(fb.pos == position(0, 8));
  }
  
void handle_command_test_4() {
  env_settings s;
  s.show_all_characters = false;
  s.tab_space = 8;
  file_buffer fb = make_empty_buffer();
  fb = insert(fb, "The quick brown fox jumps over the lazy dog", s);
  fb = handle_command(fb, "c/AAA/", s);
  EXPECT_TRUE(to_string(fb.content)==std::string("The quick brown fox jumps over the lazy dogAAA"));
  }
  
void handle_command_test_5() {
  env_settings s;
  s.show_all_characters = false;
  s.tab_space = 8;
  file_buffer fb = make_empty_buffer();
  fb = insert(fb, "The quick brown fox jumps over the lazy dog", s);
  fb = handle_command(fb, ",c/AAA/", s);
  EXPECT_TRUE(to_string(fb.content)==std::string("AAA"));
  //printf("%s\n", to_string(fb.content).c_str());
  }
  
void handle_command_test_6() {
  env_settings s;
  s.show_all_characters = false;
  s.tab_space = 8;
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/Peter/", s);
  fb = handle_command(fb, "s/t/st/", s);
  EXPECT_TRUE(to_string(fb.content)==std::string("Pester"));
  //printf("%s\n", to_string(fb.content).c_str());
  fb = handle_command(fb, "u", s);
  EXPECT_TRUE(to_string(fb.content)==std::string("Peter"));
  }
  
void handle_command_test_7() {
  env_settings s;
  s.show_all_characters = false;
  s.tab_space = 8;
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/The quick brown fox jumps over the lazy dog/", s);
  fb = handle_command(fb, "s/ck/cker/", s);
  fb = handle_command(fb, "m #15", s);
  EXPECT_TRUE(to_string(fb.content)==std::string("The qui browckern fox jumps over the lazy dog"));
  //printf("%s\n", to_string(fb.content).c_str());
}

void handle_command_test_8() {
  env_settings s;
  s.show_all_characters = false;
  s.tab_space = 8;
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/The quick brown fox jumps over the lazy dog/", s);
  fb = handle_command(fb, "/b/ c/B/", s);
  EXPECT_TRUE(to_string(fb.content)==std::string("The quick Brown fox jumps over the lazy dog"));
  //printf("%s\n", to_string(fb.content).c_str());
}

void handle_command_test_9() {
  env_settings s;
  s.show_all_characters = false;
  s.tab_space = 8;
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/AAA/", s);
  fb = handle_command(fb, "s/B*/prr/", s);
  EXPECT_TRUE(to_string(fb.content)==std::string("prrAAA"));
  //printf("%s\n", to_string(fb.content).c_str());
}

void handle_command_test_10() {
  env_settings s;
  s.show_all_characters = false;
  s.tab_space = 8;
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/AA/", s);
  fb = handle_command(fb, "#1 i/B/", s);
  EXPECT_TRUE(to_string(fb.content)==std::string("ABA"));
  //printf("%s\n", to_string(fb.content).c_str());
  fb = handle_command(fb, "/C*/ i/C/", s);
  EXPECT_TRUE(to_string(fb.content)==std::string("CABA"));
  //printf("%s\n", to_string(fb.content).c_str());
}

void handle_command_test_11() {
  env_settings s;
  s.show_all_characters = false;
  s.tab_space = 8;
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/ABCDE/", s);
  fb = handle_command(fb, "/B/ m #3", s);
  EXPECT_TRUE(to_string(fb.content)==std::string("ACDBE"));
  //printf("%s\n", to_string(fb.content).c_str());
}

void handle_command_test_12() {
  env_settings s;
  s.show_all_characters = false;
  s.tab_space = 8;
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/The quick brown fox jumps over the lazy dog/", s);
  fb = handle_command(fb, ", c/AAA/", s);
  fb = handle_command(fb, "x/B*/ c/-/", s);
  EXPECT_TRUE(to_string(fb.content)==std::string("-A-A-A-"));
  //printf("%s\n", to_string(fb.content).c_str());
}

void handle_command_test_13() {
  env_settings s;
  s.show_all_characters = false;
  s.tab_space = 8;
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/This text contains a newline\nYou see?\nWhat a text!/", s);
  fb = handle_command(fb, ", x/text/ d", s); // delete all occurences of text
  EXPECT_TRUE(to_string(fb.content)==std::string("This  contains a newline\nYou see?\nWhat a !"));
  //printf("%s\n", to_string(fb.content).c_str());
}

void handle_command_test_14() {
  env_settings s;
  s.show_all_characters = false;
  s.tab_space = 8;
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/The quick brown fox jumps over the lazy dog/", s);
  std::string error;
  try {
  fb = handle_command(fb, "100", s);
  } catch (std::runtime_error e) {
    error = std::string(e.what());
  }
  EXPECT_TRUE(error == std::string("Invalid address"));
  error = "";
  try {
  fb = handle_command(fb, "1", s);
  } catch (std::runtime_error e) {
    error = std::string(e.what());
  }
  EXPECT_TRUE(error == std::string(""));
  error = "";
  try {
  fb = handle_command(fb, "#500", s);
  } catch (std::runtime_error e) {
    error = std::string(e.what());
  }
  EXPECT_TRUE(error == std::string("Invalid address"));
}

void handle_command_test_15() {
  env_settings s;
  s.show_all_characters = false;
  s.tab_space = 8;
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/The quick brown fox jumps over the lazy dog/", s);
  fb = handle_command(fb, ", c/AAA/", s);
  fb = handle_command(fb, "y/A/ c/-/", s);
  EXPECT_TRUE(to_string(fb.content) == std::string("-A-A-A-"));
  //printf("%s\n", to_string(fb.content).c_str());
  }

void throw_error_test() {
  struct { error_type type; std::string message; } cases[] = {
    { bad_syntax, "Bad syntax" },
    { no_tokens, "I expect more tokens in this command" },
    { command_expected, "I expect a command" },
    { address_expected, "I expect an address" },
    { token_expected, "I expect a token" },
    { invalid_address, "Invalid address" },
    { invalid_regex, "Invalid regular expression" },
    { pipe_error, "Pipe error" },
    { not_implemented, "Not implemented" }
  };
  for (const auto& c : cases) {
    std::string error;
    try {
      throw_error(c.type);
    } catch (std::runtime_error& e) {
      error = std::string(e.what());
    }
    EXPECT_EQ(c.message, error);
  }

  std::string error;
  try {
    throw_error(bad_syntax, "extra info");
  } catch (std::runtime_error& e) {
    error = std::string(e.what());
  }
  EXPECT_EQ(std::string("Bad syntax: extra info"), error);
}

void tokenize_test_simple_tokens() {
  auto tokens = tokenize(",.+-$#");
  EXPECT_EQ(6, tokens.size());
  EXPECT_EQ(token::T_COMMA, tokens[0].type);
  EXPECT_EQ(token::T_DOT, tokens[1].type);
  EXPECT_EQ(token::T_PLUS, tokens[2].type);
  EXPECT_EQ(token::T_MINUS, tokens[3].type);
  EXPECT_EQ(token::T_DOLLAR, tokens[4].type);
  EXPECT_EQ(token::T_HASHTAG, tokens[5].type);
}

void tokenize_test_empty() {
  auto tokens = tokenize("");
  EXPECT_EQ(0, tokens.size());

  auto only_spaces = tokenize("   ");
  EXPECT_EQ(0, only_spaces.size());
}

void tokenize_test_filename() {
  auto tokens = tokenize("w myfile.txt");
  EXPECT_EQ(2, tokens.size());
  EXPECT_EQ(token::T_COMMAND, tokens[0].type);
  EXPECT_EQ(std::string("w"), tokens[0].value);
  EXPECT_EQ(token::T_FILENAME, tokens[1].type);
  EXPECT_EQ(std::string("myfile.txt"), tokens[1].value);

  auto quoted = tokenize("e \"my file.txt\"");
  EXPECT_EQ(2, quoted.size());
  EXPECT_EQ(token::T_FILENAME, quoted[1].type);
  EXPECT_EQ(std::string("my file.txt"), quoted[1].value);
}

void tokenize_test_substitute() {
  auto tokens = tokenize("s/a/b/");
  EXPECT_EQ(6, tokens.size());
  EXPECT_EQ(token::T_COMMAND, tokens[0].type);
  EXPECT_EQ(token::T_DELIMITER_SLASH, tokens[1].type);
  EXPECT_EQ(token::T_TEXT, tokens[2].type);
  EXPECT_EQ(std::string("a"), tokens[2].value);
  EXPECT_EQ(token::T_DELIMITER_SLASH, tokens[3].type);
  EXPECT_EQ(token::T_TEXT, tokens[4].type);
  EXPECT_EQ(std::string("b"), tokens[4].value);
  EXPECT_EQ(token::T_DELIMITER_SLASH, tokens[5].type);
}

void tokenize_test_escaped_slash() {
  auto tokens = tokenize("a/a\\/b/");
  EXPECT_EQ(4, tokens.size());
  EXPECT_EQ(token::T_TEXT, tokens[2].type);
  EXPECT_EQ(std::string("a/b"), tokens[2].value);
}

void tokenize_test_unterminated_text() {
  auto tokens = tokenize("a/no closing slash");
  EXPECT_EQ(3, tokens.size());
  EXPECT_EQ(token::T_COMMAND, tokens[0].type);
  EXPECT_EQ(token::T_DELIMITER_SLASH, tokens[1].type);
  EXPECT_EQ(token::T_TEXT, tokens[2].type);
  EXPECT_EQ(std::string("no closing slash"), tokens[2].value);
}

void tokenize_test_external_command() {
  auto tokens = tokenize("|sort -u");
  EXPECT_EQ(2, tokens.size());
  EXPECT_EQ(token::T_COMMAND, tokens[0].type);
  EXPECT_EQ(std::string("|"), tokens[0].value);
  EXPECT_EQ(token::T_EXTERNAL_COMMAND, tokens[1].type);
  EXPECT_EQ(std::string("sort -u"), tokens[1].value);
}

void parse_test_character_number() {
  auto expr = parse(tokenize("#12"));
  EXPECT_EQ(1, expr.size());
  EXPECT_TRUE(std::holds_alternative<AddressRange>(expr.front()));
  const auto& sa = std::get<AddressRange>(expr.front()).operands.front().operands.front();
  EXPECT_TRUE(std::holds_alternative<CharacterNumber>(sa));
  EXPECT_EQ(12, std::get<CharacterNumber>(sa).value);
}

void parse_test_regexp_address() {
  auto expr = parse(tokenize("/abc/"));
  EXPECT_EQ(1, expr.size());
  const auto& sa = std::get<AddressRange>(expr.front()).operands.front().operands.front();
  EXPECT_TRUE(std::holds_alternative<RegExp>(sa));
  EXPECT_EQ(std::string("abc"), std::get<RegExp>(sa).regexp);
}

void parse_test_address_range_comma() {
  auto expr = parse(tokenize("1,3"));
  EXPECT_EQ(1, expr.size());
  const auto& ar = std::get<AddressRange>(expr.front());
  EXPECT_EQ(2, ar.operands.size());
  EXPECT_EQ(1, ar.fops.size());
  EXPECT_EQ(std::string(","), ar.fops[0]);
  EXPECT_EQ(1, std::get<LineNumber>(ar.operands[0].operands.front()).value);
  EXPECT_EQ(3, std::get<LineNumber>(ar.operands[1].operands.front()).value);
}

void parse_test_address_plus() {
  auto expr = parse(tokenize(".+2"));
  const auto& at = std::get<AddressRange>(expr.front()).operands.front();
  EXPECT_EQ(2, at.operands.size());
  EXPECT_EQ(std::string("+"), at.fops[0]);
  EXPECT_TRUE(std::holds_alternative<Dot>(at.operands[0]));
  EXPECT_EQ(2, std::get<LineNumber>(at.operands[1]).value);
}

void parse_test_all_commands() {
  {
    auto expr = parse(tokenize("a/txt/"));
    EXPECT_TRUE(std::holds_alternative<Cmd_a>(std::get<Command>(expr.front())));
    EXPECT_EQ(std::string("txt"), std::get<Cmd_a>(std::get<Command>(expr.front())).txt.text);
  }
  {
    auto expr = parse(tokenize("i/txt/"));
    EXPECT_TRUE(std::holds_alternative<Cmd_i>(std::get<Command>(expr.front())));
    EXPECT_EQ(std::string("txt"), std::get<Cmd_i>(std::get<Command>(expr.front())).txt.text);
  }
  {
    auto expr = parse(tokenize("d"));
    EXPECT_TRUE(std::holds_alternative<Cmd_d>(std::get<Command>(expr.front())));
  }
  {
    auto expr = parse(tokenize("e file.txt"));
    EXPECT_TRUE(std::holds_alternative<Cmd_e>(std::get<Command>(expr.front())));
    EXPECT_EQ(std::string("file.txt"), std::get<Cmd_e>(std::get<Command>(expr.front())).filename);
  }
  {
    auto expr = parse(tokenize("r file.txt"));
    EXPECT_TRUE(std::holds_alternative<Cmd_r>(std::get<Command>(expr.front())));
    EXPECT_EQ(std::string("file.txt"), std::get<Cmd_r>(std::get<Command>(expr.front())).filename);
  }
  {
    auto expr = parse(tokenize("w file.txt"));
    EXPECT_TRUE(std::holds_alternative<Cmd_w>(std::get<Command>(expr.front())));
    EXPECT_EQ(std::string("file.txt"), std::get<Cmd_w>(std::get<Command>(expr.front())).filename);
  }
  {
    auto expr = parse(tokenize("s/a/b/"));
    EXPECT_TRUE(std::holds_alternative<Cmd_s>(std::get<Command>(expr.front())));
    const auto& cmd = std::get<Cmd_s>(std::get<Command>(expr.front()));
    EXPECT_EQ(std::string("a"), cmd.regexp.regexp);
    EXPECT_EQ(std::string("b"), cmd.txt.text);
  }
  {
    auto expr = parse(tokenize("u"));
    EXPECT_TRUE(std::holds_alternative<Cmd_u>(std::get<Command>(expr.front())));
    EXPECT_EQ(1, std::get<Cmd_u>(std::get<Command>(expr.front())).value);
  }
  {
    auto expr = parse(tokenize("u3"));
    EXPECT_EQ(3, std::get<Cmd_u>(std::get<Command>(expr.front())).value);
  }
  {
    auto expr = parse(tokenize("m #3"));
    EXPECT_TRUE(std::holds_alternative<Cmd_m>(std::get<Command>(expr.front())));
    const auto& cmd = std::get<Cmd_m>(std::get<Command>(expr.front()));
    EXPECT_EQ(3, std::get<CharacterNumber>(cmd.addr.operands.front().operands.front()).value);
  }
  {
    auto expr = parse(tokenize("t #3"));
    EXPECT_TRUE(std::holds_alternative<Cmd_t>(std::get<Command>(expr.front())));
    const auto& cmd = std::get<Cmd_t>(std::get<Command>(expr.front()));
    EXPECT_EQ(3, std::get<CharacterNumber>(cmd.addr.operands.front().operands.front()).value);
  }
  {
    auto expr = parse(tokenize("x/ab/ d"));
    EXPECT_TRUE(std::holds_alternative<Cmd_x>(std::get<Command>(expr.front())));
    const auto& cmd = std::get<Cmd_x>(std::get<Command>(expr.front()));
    EXPECT_EQ(std::string("ab"), cmd.regexp.regexp);
    EXPECT_EQ(1, cmd.cmd.size());
    EXPECT_TRUE(std::holds_alternative<Cmd_d>(cmd.cmd.front()));
  }
  {
    auto expr = parse(tokenize("y/ab/ d"));
    EXPECT_TRUE(std::holds_alternative<Cmd_y>(std::get<Command>(expr.front())));
    EXPECT_EQ(std::string("ab"), std::get<Cmd_y>(std::get<Command>(expr.front())).regexp.regexp);
  }
  {
    auto expr = parse(tokenize("g/ab/ d"));
    EXPECT_TRUE(std::holds_alternative<Cmd_g>(std::get<Command>(expr.front())));
    EXPECT_EQ(std::string("ab"), std::get<Cmd_g>(std::get<Command>(expr.front())).regexp.regexp);
  }
  {
    auto expr = parse(tokenize("v/ab/ d"));
    EXPECT_TRUE(std::holds_alternative<Cmd_v>(std::get<Command>(expr.front())));
    EXPECT_EQ(std::string("ab"), std::get<Cmd_v>(std::get<Command>(expr.front())).regexp.regexp);
  }
}

void parse_test_address_and_command() {
  auto expr = parse(tokenize("1 d"));
  EXPECT_EQ(2, expr.size());
  EXPECT_TRUE(std::holds_alternative<AddressRange>(expr[0]));
  EXPECT_TRUE(std::holds_alternative<Command>(expr[1]));
  EXPECT_TRUE(std::holds_alternative<Cmd_d>(std::get<Command>(expr[1])));
}

void parse_test_unknown_command() {
  std::string error;
  try {
    parse(tokenize("q"));
  } catch (std::runtime_error& e) {
    error = std::string(e.what());
  }
  EXPECT_EQ(std::string("I expect a command"), error);
}

void parse_test_no_tokens() {
  auto expr = parse(std::vector<token>());
  EXPECT_EQ(0, expr.size());
}

void handle_command_cmd_d_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/ABCDE/", s);
  fb = handle_command(fb, "#1,#3 d", s);
  EXPECT_EQ(std::string("ADE"), to_string(fb.content));
}

void handle_command_cmd_i_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/ABC/", s);
  fb = handle_command(fb, "#0 i/X/", s);
  EXPECT_EQ(std::string("XABC"), to_string(fb.content));
}

void handle_command_cmd_t_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/ABCDE/", s);
  fb = handle_command(fb, "#0,#2 t #5", s);
  EXPECT_EQ(std::string("ABCDEAB"), to_string(fb.content));
}

void handle_command_cmd_u_multiple_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/A/", s);
  fb = handle_command(fb, "a/B/", s);
  fb = handle_command(fb, "a/C/", s);
  EXPECT_EQ(std::string("ABC"), to_string(fb.content));
  fb = handle_command(fb, "u2", s);
  EXPECT_EQ(std::string("A"), to_string(fb.content));
}

void handle_command_cmd_g_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/hello world/", s);
  fb = handle_command(fb, ", g/world/ c/MATCH/", s);
  EXPECT_EQ(std::string("MATCH"), to_string(fb.content));

  file_buffer fb2 = make_empty_buffer();
  fb2 = handle_command(fb2, "a/hello world/", s);
  fb2 = handle_command(fb2, ", g/zzz/ c/MATCH/", s);
  EXPECT_EQ(std::string("hello world"), to_string(fb2.content));
}

void handle_command_cmd_v_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/hello world/", s);
  fb = handle_command(fb, ", v/zzz/ c/NOMATCH/", s);
  EXPECT_EQ(std::string("NOMATCH"), to_string(fb.content));

  file_buffer fb2 = make_empty_buffer();
  fb2 = handle_command(fb2, "a/hello world/", s);
  fb2 = handle_command(fb2, ", v/world/ c/NOMATCH/", s);
  EXPECT_EQ(std::string("hello world"), to_string(fb2.content));
}

void handle_command_cmd_w_and_e_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/Written content/", s);
  fb = handle_command(fb, "w edit_test_file.txt", s);

  file_buffer other = make_empty_buffer();
  other = handle_command(other, "a/Something else/", s);
  other = handle_command(other, "e edit_test_file.txt", s);
  EXPECT_EQ(std::string("Written content"), to_string(other.content));
}

void handle_command_cmd_r_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/Written content/", s);
  fb = handle_command(fb, "w edit_test_file.txt", s);

  file_buffer other = make_empty_buffer();
  other = handle_command(other, "a/AB/", s);
  other = handle_command(other, "#1 r edit_test_file.txt", s);
  EXPECT_EQ(std::string("AWritten contentB"), to_string(other.content));
}

void handle_command_address_dot_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/ABCDE/", s);
  fb = handle_command(fb, "#1,#3", s);
  fb = handle_command(fb, ".", s);
  EXPECT_TRUE(fb.start_selection == position(0, 1));
  EXPECT_TRUE(fb.pos == position(0, 2));
}

void handle_command_address_dollar_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/line1\\nline2\\nline3/", s);
  fb = handle_command(fb, "$", s);
  EXPECT_TRUE(fb.pos == get_last_position(fb));
}

void handle_command_address_arithmetic_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/line1\\nline2\\nline3/", s);
  // '+' is relative to the end of the left hand address, so 1+2 is the second line.
  fb = handle_command(fb, "1+2", s);
  EXPECT_EQ(1, fb.pos.row);
}

void handle_command_empty_buffer_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, ",", s);
  EXPECT_TRUE(fb.content.empty());
  fb = handle_command(fb, "/abc/", s);
  EXPECT_TRUE(fb.content.empty());
}

void regex_invalid_expression_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/The quick brown fox/", s);

  // Unbalanced parentheses, brackets and braces must not crash but report an error.
  const char* invalid_regexes[] = { "(", ")", "[", "[a-", "a{2", "*", "+", "?", "(?", "a{1000000000}" };
  for (const auto* re : invalid_regexes) {
    std::string command = std::string("/") + re + std::string("/");
    std::string error = command_error(fb, command, s);
    // Either the regex is rejected with a controlled error, or it is a valid
    // ECMAScript expression. In both cases we may not crash.
    if (!error.empty())
      EXPECT_TRUE(error.find("Invalid regular expression") == 0);
  }
}

void regex_invalid_expression_in_commands_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/The quick brown fox/", s);

  const char* commands[] = { ", x/(/ d", ", y/(/ d", ", g/(/ d", ", v/(/ d", ", s/(/X/" };
  for (const auto* c : commands) {
    std::string error = command_error(fb, c, s);
    EXPECT_TRUE(error.empty() || error.find("Invalid regular expression") == 0);
  }
}

void regex_empty_expression_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/ABC/", s);
  // An empty regex matches an empty string everywhere. This may not hang or crash.
  fb = handle_command(fb, "//", s);
  EXPECT_TRUE(fb.start_selection == std::nullopt);
  EXPECT_TRUE(fb.pos == position(0, 0));
}

void regex_search_out_of_range_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/ABC\\nDEF/", s);
  // A starting position beyond the buffer must be clamped instead of reading
  // out of bounds.
  fb.pos = position(100, 100);
  fb.start_selection = std::nullopt;
  fb = handle_command(fb, "/DEF/", s);
  EXPECT_TRUE(fb.start_selection == position(1, 0));
  EXPECT_TRUE(fb.pos == position(1, 2));
}

void regex_reverse_search_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/aXbXcX/", s);
  fb = handle_command(fb, "$-/X/", s);
  EXPECT_EQ(0, fb.pos.row);
}

void regex_no_match_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/ABC/", s);
  fb = handle_command(fb, "/ZZZ/", s);
  // No match: dot is moved to the end of the buffer, no crash.
  EXPECT_TRUE(fb.pos.row >= 0);
  EXPECT_EQ(std::string("ABC"), to_string(fb.content));
}

void regex_multiline_dot_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/aa\\nbb\\ncc/", s);
  fb = handle_command(fb, ", x/b/ c/B/", s);
  EXPECT_EQ(std::string("aa\nBB\ncc"), to_string(fb.content));
}

void regex_anchors_test() {
  env_settings s = make_edit_settings();
  file_buffer fb = make_empty_buffer();
  fb = handle_command(fb, "a/abc/", s);
  // Anchors used to be able to generate zero length matches in a loop.
  fb = handle_command(fb, ", x/^/ c/-/", s);
  EXPECT_TRUE(to_string(fb.content).find('-') != std::string::npos);
}

JEDLIB_END

void run_edit_tests() {
  using namespace JEDLIB;
  parse_test_1();
  parse_test_2();
  parse_test_3();
  parse_test_4();
  parse_test_5();
  parse_test_6();
  handle_command_test_1();
  handle_command_test_2();
  handle_command_test_3();
  handle_command_test_4();
  handle_command_test_5();
  handle_command_test_6();
  handle_command_test_7();
  handle_command_test_8();
  handle_command_test_9();
  handle_command_test_10();
  handle_command_test_11();
  handle_command_test_12();
  handle_command_test_13();
  handle_command_test_14();
  handle_command_test_15();
  throw_error_test();
  tokenize_test_simple_tokens();
  tokenize_test_empty();
  tokenize_test_filename();
  tokenize_test_substitute();
  tokenize_test_escaped_slash();
  tokenize_test_unterminated_text();
  tokenize_test_external_command();
  parse_test_character_number();
  parse_test_regexp_address();
  parse_test_address_range_comma();
  parse_test_address_plus();
  parse_test_all_commands();
  parse_test_address_and_command();
  parse_test_unknown_command();
  parse_test_no_tokens();
  handle_command_cmd_d_test();
  handle_command_cmd_i_test();
  handle_command_cmd_t_test();
  handle_command_cmd_u_multiple_test();
  handle_command_cmd_g_test();
  handle_command_cmd_v_test();
  handle_command_cmd_w_and_e_test();
  handle_command_cmd_r_test();
  handle_command_address_dot_test();
  handle_command_address_dollar_test();
  handle_command_address_arithmetic_test();
  handle_command_empty_buffer_test();
  regex_invalid_expression_test();
  regex_invalid_expression_in_commands_test();
  regex_empty_expression_test();
  regex_search_out_of_range_test();
  regex_reverse_search_test();
  regex_no_match_test();
  regex_multiline_dot_test();
  regex_anchors_test();
}
