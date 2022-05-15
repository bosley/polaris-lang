#include "polaris/polaris.hpp"

#include <iostream>

void repl(const std::string & prompt, std::shared_ptr<polaris::environment_c> env)
{
  polaris::evaluator_c evaluator;
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
  auto env = std::make_shared<polaris::environment_c>();
  
  polaris::add_globals(env);
  repl("polaris> ", env);
}