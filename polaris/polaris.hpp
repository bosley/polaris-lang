#ifndef POLARIS_LANG_HPP
#define POLARIS_LANG_HPP

#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

namespace polaris {
namespace {
bool is_digit(const char c) {
  return std::isdigit(static_cast<unsigned char>(c)) != 0;
}
} // namespace

enum class cell_type_e { SYMBOL, NUMBER, LIST, PROC, LAMBDA, STRING };

class environment_c;

struct cell_t {
  typedef cell_t (*proc_type)(const std::vector<cell_t> &);
  typedef std::vector<cell_t>::const_iterator iter;
  typedef std::unordered_map<std::string, cell_t> map;
  cell_type_e type{cell_type_e::SYMBOL};
  std::string val;
  std::vector<cell_t> list;
  proc_type proc;
  environment_c *env{nullptr};
  cell_t(cell_type_e type = cell_type_e::SYMBOL) : type(type) {}
  cell_t(cell_type_e type, const std::string &val) : type(type), val(val) {}
  cell_t(proc_type proc) : type(cell_type_e::PROC), proc(proc) {}
};

typedef std::vector<cell_t> cells;

const cell_t false_sym(cell_type_e::SYMBOL, "#f");
const cell_t true_sym(cell_type_e::SYMBOL, "#t");
const cell_t nil(cell_type_e::SYMBOL, "nil");

class environment_c {
public:
  environment_c() {}
  environment_c(environment_c &outer) : _outer(&outer) {}
  environment_c(const cells &params, const cells &args, environment_c *outer)
      : _outer(outer) {
    auto arg = args.begin();
    for (auto param = params.begin(); param != params.end(); ++param) {
      _env[param->val] = *arg++;
    }
  }

  cell_t::map &find(const std::string &var) {
    if (_env.find(var) != _env.end()) {
      return _env;
    }
    if (_outer) {
      return _outer->find(var);
    }
    std::cerr << "Unbound symbol : " << var << std::endl;
    std::exit(EXIT_FAILURE);
  }

  cell_t &operator[](const std::string &var) { return _env[var]; }

private:
  cell_t::map _env;
  environment_c *_outer{nullptr};
};

cell_t proc_print(const cells &c) {
  std::string result;
  for (auto i = c.begin(); i != c.end(); ++i) {
    result += i->val;
    result += ", ";
  }
  if (result.size() > 0) {
    result = result.substr(0, result.size() - 2);
  }
  return cell_t(cell_type_e::STRING, result);
}

cell_t proc_add(const cells &c) {
  long n(atol(c[0].val.c_str()));
  for (auto i = c.begin() + 1; i != c.end(); ++i) {
    n += atol(i->val.c_str());
  }
  return cell_t(cell_type_e::NUMBER, std::to_string(n));
}

cell_t proc_sub(const cells &c) {
  long n(atol(c[0].val.c_str()));
  for (auto i = c.begin() + 1; i != c.end(); ++i) {
    n -= atol(i->val.c_str());
  }
  return cell_t(cell_type_e::NUMBER, std::to_string(n));
}

cell_t proc_mul(const cells &c) {
  long n(1);
  for (auto i = c.begin(); i != c.end(); ++i) {
    n *= atol(i->val.c_str());
  }
  return cell_t(cell_type_e::NUMBER, std::to_string(n));
}

cell_t proc_div(const cells &c) {
  long n(atol(c[0].val.c_str()));
  for (auto i = c.begin() + 1; i != c.end(); ++i) {
    n /= atol(i->val.c_str());
  }
  return cell_t(cell_type_e::NUMBER, std::to_string(n));
}

cell_t proc_greater(const cells &c) {
  long n(atol(c[0].val.c_str()));
  for (auto i = c.begin() + 1; i != c.end(); ++i) {
    if (n <= atol(i->val.c_str())) {
      return false_sym;
    }
  }
  return true_sym;
}

cell_t proc_less(const cells &c) {
  long n(atol(c[0].val.c_str()));
  for (auto i = c.begin() + 1; i != c.end(); ++i) {
    if (n >= atol(i->val.c_str())) {
      return false_sym;
    }
  }
  return true_sym;
}

cell_t proc_less_equal(const cells &c) {
  long n(atol(c[0].val.c_str()));
  for (auto i = c.begin() + 1; i != c.end(); ++i) {
    if (n > atol(i->val.c_str())) {
      return false_sym;
    }
  }
  return true_sym;
}

cell_t proc_length(const cells &c) {
  return cell_t(cell_type_e::NUMBER, std::to_string(c[0].list.size()));
}
cell_t proc_nullp(const cells &c) {
  return c[0].list.empty() ? true_sym : false_sym;
}

cell_t proc_car(const cells &c) { return c[0].list[0]; }

cell_t proc_cdr(const cells &c) {
  if (c[0].list.size() < 2) {
    return nil;
  }
  cell_t result(c[0]);
  result.list.erase(result.list.begin());
  return result;
}

cell_t proc_append(const cells &c) {
  cell_t result(cell_type_e::LIST);
  result.list = c[0].list;
  for (auto i = c[1].list.begin(); i != c[1].list.end(); ++i) {
    result.list.push_back(*i);
  }
  return result;
}

cell_t proc_cons(const cells &c) {
  cell_t result(cell_type_e::LIST);
  result.list.push_back(c[0]);
  for (auto i = c[1].list.begin(); i != c[1].list.end(); ++i) {
    result.list.push_back(*i);
  }
  return result;
}

cell_t proc_list(const cells &c) {
  cell_t result(cell_type_e::LIST);
  result.list = c;
  return result;
}

void add_globals(environment_c &env) {
  env["nil"] = nil;
  env["#f"] = false_sym;
  env["#t"] = true_sym;
  env["print"] = cell_t(&proc_print);
  env["append"] = cell_t(&proc_append);
  env["car"] = cell_t(&proc_car);
  env["cdr"] = cell_t(&proc_cdr);
  env["cons"] = cell_t(&proc_cons);
  env["length"] = cell_t(&proc_length);
  env["list"] = cell_t(&proc_list);
  env["null?"] = cell_t(&proc_nullp);
  env["+"] = cell_t(&proc_add);
  env["-"] = cell_t(&proc_sub);
  env["*"] = cell_t(&proc_mul);
  env["/"] = cell_t(&proc_div);
  env[">"] = cell_t(&proc_greater);
  env["<"] = cell_t(&proc_less);
  env["<="] = cell_t(&proc_less_equal);
}

cell_t eval(cell_t x, environment_c *env) {
  if (x.type == cell_type_e::SYMBOL) {
    return env->find(x.val)[x.val];
  }
  if (x.type == cell_type_e::NUMBER) {
    return x;
  }
  if (x.type == cell_type_e::STRING) {
    return x;
  }
  if (x.list.empty()) {
    return nil;
  }
  if (x.list[0].type == cell_type_e::SYMBOL) {
    if (x.list[0].val == "quote") {
      return x.list[1];
    }
    if (x.list[0].val == "if") {
      // (if test conseq [alt])
      return eval(eval(x.list[1], env).val == "#f"
                      ? (x.list.size() < 4 ? nil : x.list[3])
                      : x.list[2],
                  env);
    }
    if (x.list[0].val == "set!") {
      // (set! var exp)
      return env->find(x.list[1].val)[x.list[1].val] = eval(x.list[2], env);
    }
    if (x.list[0].val == "define") {
      // (define var exp)
      return (*env)[x.list[1].val] = eval(x.list[2], env);
    }
    if (x.list[0].val == "lambda") { // (lambda (var*) exp)
      x.type = cell_type_e::LAMBDA;
      // keep a reference to the environment that exists now (when the
      // lambda is being defined) because that's the outer environment
      // we'll need to use when the lambda is executed
      x.env = env;
      return x;
    }
    if (x.list[0].val == "begin") { // (begin exp*)
      for (size_t i = 1; i < x.list.size() - 1; ++i) {
        eval(x.list[i], env);
      }
      return eval(x.list[x.list.size() - 1], env);
    }
  }
  // (proc exp*)
  cell_t proc(eval(x.list[0], env));
  cells exps;
  for (cell_t::iter exp = x.list.begin() + 1; exp != x.list.end(); ++exp)
    exps.push_back(eval(*exp, env));
  if (proc.type == cell_type_e::LAMBDA) {
    // Create an environment for the execution of this lambda function
    // where the outer environment is the one that existed* at the time
    // the lambda was defined and the new inner associations are the
    // parameter names with the given arguments.
    // *Although the environmet existed at the time the lambda was defined
    // it wasn't necessarily complete - it may have subsequently had
    // more symbols defined in that environment.
    return eval(/*body*/ proc.list[2],
                new environment_c(/*parms*/ proc.list[1].list, /*args*/ exps,
                                  proc.env));
  } else if (proc.type == cell_type_e::PROC)
    return proc.proc(exps);

  std::cerr << "Not a function\n";
  std::exit(EXIT_FAILURE);
}

// Parse
std::list<std::string> tokenize(const std::string &str) {
  std::list<std::string> tokens;
  const char *s = str.c_str();
  while (*s) {
    while (*s == ' ') {
      ++s;
    }
    if (*s == '(' || *s == ')') {
      tokens.push_back(*s++ == '(' ? "(" : ")");
    } else {
      const char *t = s;
      bool in_str {false};
      bool consume {true};
      while (*t && consume) {
        if (*t == '"') {
          in_str = !in_str;
        }
        if(in_str) {
          ++t;
        }
        else if(*t != ' ' && *t != '(' && *t != ')') {
          ++t;
        }
        else {
          consume = false;
        }
      }
      tokens.push_back(std::string(s, t));
      s = t;
    }
  }
  return tokens;
}

cell_t atom(const std::string &token) {
  if (is_digit(token[0]) || (token[0] == '-' && is_digit(token[1]))) {
    return cell_t(cell_type_e::NUMBER, token);
  }

  if (token.starts_with('"') && token.ends_with('"')) {
    auto trimmed = token.substr(1, token.size()-2);
    return cell_t(cell_type_e::STRING, trimmed);
  }

  return cell_t(cell_type_e::SYMBOL, token);
}

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

cell_t read(const std::string &s) {
  std::list<std::string> tokens(tokenize(s));
  return read_from(tokens);
}

std::string to_string(const cell_t &exp) {
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