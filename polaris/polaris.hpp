#ifndef POLARIS_LANG_HPP
#define POLARIS_LANG_HPP

#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace polaris {
namespace {
bool is_digit(const char c) {
  return std::isdigit(static_cast<unsigned char>(c)) != 0;
}

} // namespace

//! \brief General cell types
enum class cell_type_e { SYMBOL, NUMBER, LIST, PROC, LAMBDA, STRING };

//! \brief Execution environment
class environment_c;

//! \brief A given cell
struct cell_t {
  //! Shorthand for function calls
  using proc_fn = std::function<cell_t(const std::vector<cell_t> &)>;

  //! Shorthand for an unordered map of cells
  using map = std::unordered_map<std::string, cell_t>;
  
  //! The cells type
  cell_type_e type{cell_type_e::SYMBOL};

  //! Cell value
  std::string val;

  //! Cell list
  std::vector<cell_t> list;
  
  //! Function call (lambda)
  proc_fn proc;

  //! Cell operating environment
  std::shared_ptr<environment_c> env;

  //! \brief Construct a cell with only a given type
  //! \param type The type to give the cell
  cell_t(cell_type_e type = cell_type_e::SYMBOL) : type(type) {}

  //! \brief Construct the cell with a type and value
  //! \param type The type to give the cell
  //! \param val The value to give the cell
  cell_t(cell_type_e type, const std::string &val) : type(type), val(val) {}

  //! \brief Construct a cell that executes a function 
  //! \param proc The function to process
  cell_t(proc_fn proc) : type(cell_type_e::PROC), proc(proc) {}
};

using cells = std::vector<cell_t>;  //! Shorthand for vector of cells
const cell_t false_sym(cell_type_e::SYMBOL, "#f");  //! Cell for "FALSE"
const cell_t true_sym(cell_type_e::SYMBOL, "#t"); //! Cell for "TRUE"
const cell_t nil(cell_type_e::SYMBOL, "nil"); //! Cell for "NIL"

//! \brief Execution environment
class environment_c {
public:
  //! \brief Construct a base environment
  environment_c() {}
  
  //! \brief Construct an environment with an "outer" parent environemnt
  //!        that it can use to locate symbols
  environment_c(std::shared_ptr<environment_c> outer) : _outer(outer) {}

  //! \brief Construct an environment wiht specific data baked into it 
  //!        from the getgo
  //! \param params Names of incoming data 
  //! \param args Value of incoming datas
  environment_c(const cells &params, const cells &args,
                std::shared_ptr<environment_c> outer)
      : _outer(outer) {
    auto arg = args.begin();
    for (auto param = params.begin(); param != params.end(); ++param) {
      _env[param->val] = *arg++;
    }
  }

  //! \brief Find an environment variable given the name
  //!        If the item can not be found in the current environment
  //!        Then the outer environments will be checked - If the item
  //!        does not exist std::exit will be called
  cell_t::map &find(const std::string &var) {
    if (_env.find(var) != _env.end()) {
      return _env;
    }
    if (_outer) {
      return _outer->find(var);
    }
    std::cerr << "Unbound symbol : [" << var << "]" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  //! \brief Operator [] overload for accessing environment variables
  //! \param var The variable to retrieve
  cell_t &operator[](const std::string &var) { return _env[var]; }

  //! \brief Get accessor for grabbing things through the shared pointer
  //! \param var The variable to retrieve
  cell_t &get(const std::string &value) { return _env[value]; }

private:
  cell_t::map _env;
  std::shared_ptr<environment_c> _outer;
};

//! \brief Evaluator
class evaluator_c {
public:
  //! \brief Construct the base evaluator with the standard
  //!         callable symbols baked into it
  evaluator_c() {
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

  //! \brief Evaluate a cell given and environment
  //! \param x The cell to evaluate
  //! \param env The environment to use in the evaluation
  cell_t evaluate(cell_t x, std::shared_ptr<environment_c> env) {

    // Check for symbol number and string types
    //
    switch (x.type) {
    case cell_type_e::SYMBOL:
      return env->find(x.val)[x.val];
    case cell_type_e::NUMBER:
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
      return evaluate(proc.list[2],
          std::make_shared<environment_c>(proc.list[1].list, exps, proc.env));

    } else if (proc.type == cell_type_e::PROC) {

      //  If the item isn't a lambda perhaps its a processing cell so we need to call it
      //
      return proc.proc(exps);
    }

    //  Sadly, if we get here it is time to kill.. something wild came in and the
    //  user most likely did something silly. 
    //
    std::cerr << "Not a function\n";
    std::exit(EXIT_FAILURE);
  }

private:
  std::unordered_map<std::string, std::function<cell_t(
                                      cell_t, std::shared_ptr<environment_c>)>>
      _callable_symbol_table;
};

//! \brief Add the basic sumbols to a given environment
//! \param env The environment to load the symbols into
static void add_globals(std::shared_ptr<environment_c> env) {
  env->get("nil") = nil;
  env->get("#f") = false_sym;
  env->get("#t") = true_sym;

  env->get("print") = cell_t([](const cells &c) -> cell_t {
    std::string result;
    for (auto i = c.begin(); i != c.end(); ++i) {
      result += i->val;
      result += ", ";
    }
    if (result.size() > 0) {
      result = result.substr(0, result.size() - 2);
    }
    return cell_t(cell_type_e::STRING, result);
  });

  env->get("append") = cell_t([](const cells &c) -> cell_t {
    cell_t result(cell_type_e::LIST);
    result.list = c[0].list;
    for (auto i = c[1].list.begin(); i != c[1].list.end(); ++i) {
      result.list.push_back(*i);
    }
    return result;
  });

  env->get("car") =
      cell_t([](const cells &c) -> cell_t { return c[0].list[0]; });

  env->get("cdr") = cell_t([](const cells &c) -> cell_t {
    if (c[0].list.size() < 2) {
      return nil;
    }
    cell_t result(c[0]);
    result.list.erase(result.list.begin());
    return result;
  });

  env->get("cons") = cell_t([](const cells &c) -> cell_t {
    cell_t result(cell_type_e::LIST);
    result.list.push_back(c[0]);
    for (auto i = c[1].list.begin(); i != c[1].list.end(); ++i) {
      result.list.push_back(*i);
    }
    return result;
  });

  env->get("length") = cell_t([](const cells &c) -> cell_t {
    return cell_t(cell_type_e::NUMBER, std::to_string(c[0].list.size()));
  });

  env->get("list") = cell_t([](const cells &c) -> cell_t {
    cell_t result(cell_type_e::LIST);
    result.list = c;
    return result;
  });

  env->get("null?") = cell_t([](const cells &c) -> cell_t {
    return c[0].list.empty() ? true_sym : false_sym;
  });

  env->get("+") = cell_t([](const cells &c) -> cell_t {
    long n(atol(c[0].val.c_str()));
    for (auto i = c.begin() + 1; i != c.end(); ++i) {
      n += atol(i->val.c_str());
    }
    return cell_t(cell_type_e::NUMBER, std::to_string(n));
  });

  env->get("-") = cell_t([](const cells &c) -> cell_t {
    long n(atol(c[0].val.c_str()));
    for (auto i = c.begin() + 1; i != c.end(); ++i) {
      n -= atol(i->val.c_str());
    }
    return cell_t(cell_type_e::NUMBER, std::to_string(n));
  });

  env->get("*") = cell_t([](const cells &c) -> cell_t {
    long n(1);
    for (auto i = c.begin(); i != c.end(); ++i) {
      n *= atol(i->val.c_str());
    }
    return cell_t(cell_type_e::NUMBER, std::to_string(n));
  });

  env->get("/") = cell_t([](const cells &c) -> cell_t {
    long n(atol(c[0].val.c_str()));
    for (auto i = c.begin() + 1; i != c.end(); ++i) {
      n /= atol(i->val.c_str());
    }
    return cell_t(cell_type_e::NUMBER, std::to_string(n));
  });

  env->get(">") = cell_t([](const cells &c) -> cell_t {
    long n(atol(c[0].val.c_str()));
    for (auto i = c.begin() + 1; i != c.end(); ++i) {
      if (n <= atol(i->val.c_str())) {
        return false_sym;
      }
    }
    return true_sym;
  });

  env->get("<") = cell_t([](const cells &c) -> cell_t {
    long n(atol(c[0].val.c_str()));
    for (auto i = c.begin() + 1; i != c.end(); ++i) {
      if (n >= atol(i->val.c_str())) {
        return false_sym;
      }
    }
    return true_sym;
  });

  env->get("<=") = cell_t([](const cells &c) -> cell_t {
    long n(atol(c[0].val.c_str()));
    for (auto i = c.begin() + 1; i != c.end(); ++i) {
      if (n > atol(i->val.c_str())) {
        return false_sym;
      }
    }
    return true_sym;
  });
}

//  These functions don't need to be exposed
//
namespace {

// Convert a string into a list of string "tokens" to be processed
std::list<std::string> tokenize(const std::string &str) {
  std::list<std::string> tokens;
  for (auto i = 0; i < str.size(); i++) {
    if (std::isspace(str[i])) {
      continue;
    }
    if (str[i] == '(' || str[i] == ')') {
      tokens.push_back(str[i] == '(' ? "(" : ")");
    } else {
      bool in_str{false};
      std::string token;
      while (i < str.size()) {
        if (str[i] == '"') {
          if (i > 0 && str[i - 1] != '\\') {
            in_str = !in_str;
          }
        }
        if (in_str) {
          token += str[i];
        } else if (!std::isspace(str[i]) && str[i] != '(' && str[i] != ')') {
          token += str[i];
        } else {
          --i;
          break;
        }
        ++i;
      }
      if (!token.empty()) {
        tokens.push_back(token);
      }
    }
  }
  return tokens;
}

// Take a token and convert it into a cell
cell_t atom(const std::string &token) {
  if (is_digit(token[0]) || (token[0] == '-' && is_digit(token[1]))) {
    return cell_t(cell_type_e::NUMBER, token);
  }

  if (token.starts_with('"') && token.ends_with('"')) {
    auto trimmed = token.substr(1, token.size() - 2);
    return cell_t(cell_type_e::STRING, trimmed);
  }

  return cell_t(cell_type_e::SYMBOL, token);
}

// Take a list of tokens and convert them into  proper cell
cell_t read_from(std::list<std::string> &tokens) {
  const std::string token(tokens.front());
  tokens.pop_front();
  if (token == "(") {
    cell_t c(cell_type_e::LIST);
    while (tokens.front() != ")") {
      c.list.push_back(read_from(tokens));
    }
    tokens.pop_front();
    return c;
  } else {
    return atom(token);
  }
}
} // End anonymous namespace

//! \brief Take a given string and process it down 
//!        down to the cell_t level
//! \param s The string to process in
static cell_t read(const std::string &s) {
  std::list<std::string> tokens(tokenize(s));
  return read_from(tokens);
}

//! \brief Convert a given cell to a string
//! \param exp The cell to convert
static std::string to_string(const cell_t &exp) {
  if (exp.type == cell_type_e::LIST) {
    std::string s("(");
    for (auto e = exp.list.begin(); e != exp.list.end(); ++e) {
      s += to_string(*e) + ' ';
    }
    if (s[s.size() - 1] == ' ') {
      s.erase(s.size() - 1);
    }
    return s + ')';
  } else if (exp.type == cell_type_e::LAMBDA)
    return "<Lambda>";
  else if (exp.type == cell_type_e::PROC)
    return "<Proc>";
  return exp.val;
}

} // namespace polaris

#endif