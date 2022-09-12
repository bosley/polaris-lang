#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <polaris/feeder.hpp>
#include <polaris/polaris.hpp>

namespace {
std::vector<std::string> code = {
    "(define name \"Josh\")",
    "(define rank 10)",
    "(define some_func (lambda (x) (print x)))",
};
}

int main(int argc, char **argv) {

   //  Create the evaluator that will execute statements
   //
   polaris::evaluator_c evaluator;

   //  Create the environment that will store data
   //
   auto environment = std::make_shared<polaris::environment_c>();

   //  Create the importer that will search disk for imported files
   //
   polaris::imports_c imports(evaluator, environment, {});

   //  Add standard functionality to the environment
   //
   polaris::add_globals(environment, imports);

   //  Create a line feeder to properly feed data into the system
   //
   polaris::feeder_c feeder(evaluator, environment);

   //  Feed each line in code
   //
   for (auto &line : code) {
      feeder.feed(line);
   }

   //  The 'find' gets the map for the environment, the bracket
   //  call accesses the returned map to get the item
   //
   auto name = environment->find("name")["name"];
   auto rank = environment->find("rank")["rank"];

   //  Output
   //
   std::cout << "Name : " << name.val << ", Rank : " << rank.val << std::endl;

   //  Create a new function and inject it into the environment
   //
   std::string call_to_new_func = "(new_func 1 2 3)";
   environment->get("new_func") =
       polaris::cell_t([](const polaris::cells &c) -> polaris::cell_t {
          std::cout << "My func was called with : " << c.size() << " params"
                    << std::endl;
          return polaris::true_sym;
       });

   //  Call that function
   //
   feeder.feed(call_to_new_func);

   return 0;
}