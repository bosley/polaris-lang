#include "polaris.hpp"

#include <iostream>
#include <list>
#include <string>
#include <vector>

namespace polaris {

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

// Check if something is a digit
bool is_digit(const char c) {
  return std::isdigit(static_cast<unsigned char>(c)) != 0;
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

void add_globals(std::shared_ptr<environment_c> env, imports_c &imports) {
  env->get("nil") = nil;
  env->get("#f") = false_sym;
  env->get("#t") = true_sym;

  env->get("print") = cell_t([](const cells &c) -> cell_t {
    
    std::string result;
    for (auto i = c.begin(); i != c.end(); ++i) {
      result += to_string((*i));
      result += ", ";
    }
    if (result.size() > 0) {
      result = result.substr(0, result.size() - 2);
    }
    std::cout << result << std::endl;
    return true_sym;
  });

  env->get("import") = cell_t([&](const cells &c) -> cell_t {
    if (c.empty()) {
      std::cerr << "Malformed import statement" << std::endl;
      std::exit(EXIT_FAILURE);
    }

    for (auto i : c) {
      imports.import(i.val);
    }

    return true_sym;
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

  env->get("eq") = cell_t([](const cells &c) -> cell_t {
    bool equal{false};
    for (auto i = c.begin() + 1; i != c.end(); ++i) {
      equal = (c[0].type == i->type && c[0].val == i->val);
    }
    return equal ? true_sym : false_sym;
  });

  env->get("neq") = cell_t([](const cells &c) -> cell_t {
    bool equal{false};
    for (auto i = c.begin() + 1; i != c.end(); ++i) {
      equal = (c[0].type == i->type && c[0].val == i->val);
    }
    return equal ? false_sym : true_sym;
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

} // namespace polaris