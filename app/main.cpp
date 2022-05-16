#include "polaris/polaris.hpp"

#include <iostream>

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

int main()
{
  polaris::evaluator_c evaluator;
  auto environment = std::make_shared<polaris::environment_c>();
  polaris::imports_c imports(evaluator, environment);
  polaris::add_globals(environment, imports);
  repl("polaris> ", evaluator, environment);
}