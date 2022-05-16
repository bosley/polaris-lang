#include "polaris/polaris.hpp"
#include "polaris/version.hpp"

#include <iostream>
#include <sstream>
#include <vector>

void help()
{
  std::cout << "\nHelp : " << std::endl
            << "-i | --include  < ':' delim list >    Add include directories\n"
            << "-h | --help                           Show help\n"
            << "-v | --version                        Show version\n"
            << "\nTo enter REPL do not include a file\n"
            << std::endl;
  std::exit(EXIT_SUCCESS);
}

void version()
{
  std::cout << "polaris version " LIBPOLARIS_VERSION << std::endl;
  std::exit(EXIT_SUCCESS);
}

void repl(const std::string & prompt, polaris::evaluator_c &evaluator, std::shared_ptr<polaris::environment_c> env)
{
  for (;;) {
      std::cout << prompt;
      std::string line; 
      std::getline(std::cin, line);
      std::cout << polaris::to_string(
        evaluator.evaluate(
          polaris::read(line), env)
          ) << std::endl;
  }
}

void execute(const std::string &file, polaris::evaluator_c &evaluator, std::shared_ptr<polaris::environment_c> env)
{
  std::cout << "Execute : " << file << std::endl;
}

int main(int argc, char ** argv)
{
  polaris::evaluator_c evaluator;
  auto environment = std::make_shared<polaris::environment_c>();

  std::string file;
  std::vector<std::string> include_dirs;

  auto arguments = std::vector<std::string>(argv + 1, argv + argc);
  for (size_t i = 0; i < arguments.size(); i++) {
    if (arguments[i] == "-i" || arguments[i] == "--include") {
      if(i + 1 >= arguments.size()) {
        std::cerr << "Expected value to be passed in with -i --include" << std::endl;
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

  if(file.empty()) {
    repl("polaris> ", evaluator, environment);
  } else {
    execute(file, evaluator, environment);
  }
  return 0;
}