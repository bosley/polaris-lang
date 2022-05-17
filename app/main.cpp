#include "polaris/polaris.hpp"
#include "polaris/version.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

//! \brief A simple feeder object that makes sure that 
//!        messy user input is massaged into something
//!        nice and formal that can be accepted into 
//!        the system
class feeder_c {
public:
  //! \brief Create the feeder
  //! \param evaluator The evaluator to use
  //! \param env The environment to use in execution
  feeder_c(polaris::evaluator_c &evaluator, 
    std::shared_ptr<polaris::environment_c> env) 
    : _eval(evaluator), _env(env) {}

  //! \brief Feed the line into the system. 
  //! \returns true iff a statement was submitted
  bool feed(std::string &line, bool print_result=false) {

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
    for(auto &c : line) {
      if (c == '(') { _tracker++; }
      else if (c == ')') { _tracker--;}
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

private:
  polaris::evaluator_c &_eval;
  std::shared_ptr<polaris::environment_c> _env;
  uint64_t _tracker{0};
  std::string _statement;
};

void help() {
  std::cout << "\nHelp : " << std::endl
            << "-i | --include  < ':' delim list >    Add include directories\n"
            << "-h | --help                           Show help\n"
            << "-v | --version                        Show version\n"
            << "\nTo enter REPL do not include a file\n"
            << std::endl;
  std::exit(EXIT_SUCCESS);
}

void version() {
  std::cout << "polaris version " LIBPOLARIS_VERSION << std::endl;
  std::exit(EXIT_SUCCESS);
}

void repl(const std::string &prompt, polaris::evaluator_c &evaluator,
          std::shared_ptr<polaris::environment_c> env) {

  bool show_prompt{true};
  feeder_c feeder(evaluator, env);
  while(1) {
    if (show_prompt) {
      std::cout << "polaris> ";
    }
    std::string line;
    std::getline(std::cin, line);
    show_prompt = feeder.feed(line);
  }
}

void execute(const std::string &file, polaris::evaluator_c &evaluator,
             std::shared_ptr<polaris::environment_c> env) {

  std::filesystem::path p(file);
  if (!std::filesystem::is_regular_file(p)) {
    std::cerr << "Item: " << file << " does not exist" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  std::fstream fs;
  fs.open(file, std::fstream::in);

  if (!fs.is_open()) {
    std::cerr << "Unable to open file: " << file << std::endl;
    std::exit(EXIT_FAILURE);
  }

  std::string line;
  feeder_c feeder(evaluator, env);
  while (std::getline(fs, line)) {
    feeder.feed(line);
  }
}

int main(int argc, char **argv) {
  polaris::evaluator_c evaluator;
  auto environment = std::make_shared<polaris::environment_c>();

  std::string file;
  std::vector<std::string> include_dirs;

  auto arguments = std::vector<std::string>(argv + 1, argv + argc);
  for (size_t i = 0; i < arguments.size(); i++) {
    if (arguments[i] == "-i" || arguments[i] == "--include") {
      if (i + 1 >= arguments.size()) {
        std::cerr << "Expected value to be passed in with -i --include"
                  << std::endl;
        std::exit(EXIT_FAILURE);
      }
      i++;
      std::string item;
      std::istringstream ss(arguments[i]);
      while (std::getline(ss, item, ':')) {
        include_dirs.emplace_back(item);
      }
      continue;
    }

    if (arguments[i] == "-h" || arguments[i] == "--help") {
      help();
    }

    if (arguments[i] == "-v" || arguments[i] == "--version") {
      version();
    }

    // If it isn't an option it is a file
    if (file.empty()) {
      file = arguments[i];
    }
  }

  polaris::imports_c imports(evaluator, environment, include_dirs);
  polaris::add_globals(environment, imports);

  if (file.empty()) {
    repl("polaris> ", evaluator, environment);
  } else {
    execute(file, evaluator, environment);
  }
  return 0;
}