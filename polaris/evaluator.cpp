#include "evaluator.hpp"
#include "environment.hpp"

#include "cell.hpp"
#include <iostream>

namespace polaris {
evaluator_c::evaluator_c() {
  _callable_symbol_table["quote"] =
      [](cell_t x, std::shared_ptr<environment_c> env) -> cell_t {
    return x.list[1];
  };

  _callable_symbol_table["if"] =
      [this](cell_t x, std::shared_ptr<environment_c> env) -> cell_t {
    return evaluate(evaluate(x.list[1], env).val == "#f"
                        ? (x.list.size() < 4 ? nil : x.list[3])
                        : x.list[2],
                    env);
  };

  _callable_symbol_table["set!"] =
      [this](cell_t x, std::shared_ptr<environment_c> env) -> cell_t {
    return env->find(x.list[1].val)[x.list[1].val] = evaluate(x.list[2], env);
  };

  _callable_symbol_table["define"] =
      [this](cell_t x, std::shared_ptr<environment_c> env) -> cell_t {
    return (*env)[x.list[1].val] = evaluate(x.list[2], env);
  };

  _callable_symbol_table["lambda"] =
      [this](cell_t x, std::shared_ptr<environment_c> env) -> cell_t {
    // (lambda (var*) exp)
    x.type = cell_type_e::LAMBDA;
    // keep a reference to the environment that exists now (when the
    // lambda is being defined) because that's the outer environment
    // we'll need to use when the lambda is executed
    x.env = std::make_shared<environment_c>(env);
    return x;
  };

  _callable_symbol_table["begin"] =
      [this](cell_t x, std::shared_ptr<environment_c> env) -> cell_t {
    // (begin exp*)
    for (size_t i = 1; i < x.list.size() - 1; ++i) {
      evaluate(x.list[i], env);
    }
    return evaluate(x.list[x.list.size() - 1], env);
  };
}

cell_t evaluator_c::evaluate(cell_t x, std::shared_ptr<environment_c> env) {

  // Check for symbol number and string types
  //
  switch (x.type) {
  case cell_type_e::SYMBOL:
    return env->find(x.val)[x.val];
  case cell_type_e::NUMBER:
    [[fallthrough]];
  case cell_type_e::DOUBLE:
    [[fallthrough]];
  case cell_type_e::STRING:
    return x;
  default:
    break;
  }

  // Check to ensure list isn't empty
  //
  if (x.list.empty()) {
    return nil;
  }

  //  If the item is a symbol and its in the symbol table that means
  //  its a callable symbol table that means we need to call it
  //
  if (x.list[0].type == cell_type_e::SYMBOL &&
      _callable_symbol_table.find(x.list[0].val) !=
          _callable_symbol_table.end()) {
    return _callable_symbol_table[x.list[0].val](x, env);
  }

  //  Create a processing cell with evaluated parameters
  //
  cells exps;
  cell_t proc(evaluate(x.list[0], env));
  for (auto exp = x.list.begin() + 1; exp != x.list.end(); ++exp) {
    exps.push_back(evaluate(*exp, env));
  }

  //  Proc type is a lambda, so it needs to be executed.
  //  Upon creation we give it a new environment to thrive in and operate on
  //  with the current environment stated as its outer
  //
  if (proc.type == cell_type_e::LAMBDA) {

    // Evaluate the body (proc.list[2]) of the lambda with the new environemnt
    //
    return evaluate(proc.list[2], std::make_shared<environment_c>(
                                      proc.list[1].list, exps, proc.env));

  } else if (proc.type == cell_type_e::PROC) {

    //  If the item isn't a lambda perhaps its a processing cell so we need to
    //  call it
    //
    return proc.proc(exps);
  }

  //  Sadly, if we get here it is time to kill.. something wild came in and
  //  the user most likely did something silly.
  //
  std::cerr << "Not a function\n";
  std::exit(EXIT_FAILURE);
}
} // namespace polaris