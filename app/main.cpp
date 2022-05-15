#include "polaris/polaris.hpp"

#include <iostream>

void repl(const std::string & prompt, polaris::environment_c * env)
{
  for (;;) {
      std::cout << prompt;
      std::string line; 
      std::getline(std::cin, line);
      std::cout << polaris::to_string(
        polaris::eval(
          polaris::read(line), env)
          ) << std::endl;
  }
}

int main()
{
  polaris::environment_c env;
  polaris::add_globals(env);
  repl("polaris> ", &env);
}