#include "feeder.hpp"
#include "polaris.hpp"

#include <iostream>

namespace polaris {

feeder_c::feeder_c(polaris::evaluator_c &evaluator,
                   std::shared_ptr<polaris::environment_c> env)
    : _eval(evaluator), _env(env) {}

bool feeder_c::feed(std::string &line, bool print_result) {

  // Remove comments
  std::size_t comment_loc = line.find_first_of(";");
  if (comment_loc != std::string::npos) {
    line = line.substr(0, comment_loc);
  }

  // If the line is empty pretend that we sent something
  // so if REPL is active then the primpt will show again
  if (line.empty()) {
    return true;
  }

  // Walk the line and see if the given parens
  // indicate that we have a full
  for (auto &c : line) {
    if (c == '(') {
      _tracker++;
    } else if (c == ')') {
      _tracker--;
    }
    _statement += c;
  }

  // If we have a statement and all parens are closed then
  // we can submit the statement
  if (_tracker == 0 && !_statement.empty()) {

    auto result = _eval.evaluate(polaris::read(_statement), _env);

    // If they requested that we print the result,
    // then print the result
    if (print_result) {
      std::cout << polaris::to_string(result) << std::endl;
    }
    _statement.clear();
    return true;

  } else {
    _statement += ' ';
  }
  return false;
}

} // namespace polaris