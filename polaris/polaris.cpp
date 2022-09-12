#include "polaris.hpp"

#include <iostream>
#include <list>
#include <regex>
#include <sstream>
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
            } else if (!std::isspace(str[i]) && str[i] != '(' &&
                       str[i] != ')') {
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

static std::regex is_number("[+-]?([0-9]*[.])?[0-9]+");

// Take a token and convert it into a cell
cell_t atom(const std::string &token) {

   if (std::regex_match(token, is_number)) {
      if (token.find('.') != std::string::npos) {
         return cell_t(cell_type_e::DOUBLE, token);
      }
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

constexpr bool promote_to_double(cell_type_e lhs, cell_type_e rhs) {
   return (lhs == cell_type_e::DOUBLE || rhs == cell_type_e::DOUBLE);
}

void add_globals(std::shared_ptr<environment_c> env, imports_c &imports) {
   env->get("nil") = nil;
   env->get("#f") = false_sym;
   env->get("#t") = true_sym;

   env->get("exit") = cell_t([=](const cells &c) -> cell_t {
      if (!c.empty()) {
         try{
            int n(std::stoi(c[0].val.c_str()));
            std::exit(n);
         } catch (...) {
            env->get_error_cb()(error_level_e::FATAL, "failed to cast return code");
            std::exit(1);
         }
      }
      std::exit(0);
   });

   env->get("print") = cell_t([](const cells &c) -> cell_t {
      std::string result;
      for (auto i = c.begin(); i != c.end(); ++i) {
         result += to_string((*i));
      }
      std::cout << result << std::endl;
      return true_sym;
   });

   env->get("ref") = cell_t([](const cells &c) -> cell_t {
      cell_t result(cell_type_e::LIST);
      for (auto i = c.begin(); i != c.end(); ++i) {
         result.list.push_back(
            cell_t(cell_type_e::STRING, cell_type_to_string((*i).type))
         );
      }
      return result;
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

   env->get("+") = cell_t([=](const cells &c) -> cell_t {
      try {
         double n(std::stod(c[0].val.c_str()));
         bool store_as_double = (c[0].type == cell_type_e::DOUBLE);
         for (auto i = c.begin() + 1; i != c.end(); ++i) {
            n += std::stod(i->val.c_str());
            if (i->type == cell_type_e::DOUBLE) {
               store_as_double = true;
            }
         }
         if (store_as_double) {
            return cell_t(cell_type_e::DOUBLE, std::to_string(n));
         } else {
            return cell_t(cell_type_e::NUMBER,
                          std::to_string(static_cast<long>(n)));
         }
      } catch (const std::invalid_argument &) {
         env->get_error_cb()(error_level_e::FATAL,
                             "invalid argument for numerical conversion");
      } catch (const std::out_of_range &) {
         env->get_error_cb()(error_level_e::FATAL, "out of range");
      }
      std::exit(1);
   });

   env->get("-") = cell_t([=](const cells &c) -> cell_t {
      try {
         double n(std::stod(c[0].val.c_str()));
         bool store_as_double = (c[0].type == cell_type_e::DOUBLE);
         for (auto i = c.begin() + 1; i != c.end(); ++i) {
            n -= std::stod(i->val.c_str());
            if (i->type == cell_type_e::DOUBLE) {
               store_as_double = true;
            }
         }
         if (store_as_double) {
            return cell_t(cell_type_e::DOUBLE, std::to_string(n));
         } else {
            return cell_t(cell_type_e::NUMBER,
                          std::to_string(static_cast<long>(n)));
         }
      } catch (const std::invalid_argument &) {
         env->get_error_cb()(error_level_e::FATAL,
                             "invalid argument for numerical conversion");
      } catch (const std::out_of_range &) {
         env->get_error_cb()(error_level_e::FATAL, "out of range");
      }
      std::exit(1);
   });

   env->get("*") = cell_t([=](const cells &c) -> cell_t {
      try {
         double n(1);
         bool store_as_double = false;
         for (auto i = c.begin(); i != c.end(); ++i) {
            n *= std::stod(i->val.c_str());
            if (i->type == cell_type_e::DOUBLE) {
               store_as_double = true;
            }
         }
         if (store_as_double) {
            return cell_t(cell_type_e::DOUBLE, std::to_string(n));
         } else {
            return cell_t(cell_type_e::NUMBER,
                          std::to_string(static_cast<long>(n)));
         }
      } catch (const std::invalid_argument &) {
         env->get_error_cb()(error_level_e::FATAL,
                             "invalid argument for numerical conversion");
      } catch (const std::out_of_range &) {
         env->get_error_cb()(error_level_e::FATAL, "out of range");
      }
      std::exit(1);
   });

   env->get("/") = cell_t([=](const cells &c) -> cell_t {
      try {
         bool store_as_double = (c[0].type == cell_type_e::DOUBLE);
         double n(std::stod(c[0].val.c_str()));
         for (auto i = c.begin() + 1; i != c.end(); ++i) {
            n /= std::stod(i->val.c_str());
            if (i->type == cell_type_e::DOUBLE) {
               store_as_double = true;
            }
         }
         if (store_as_double) {
            return cell_t(cell_type_e::DOUBLE, std::to_string(n));
         } else {
            return cell_t(cell_type_e::NUMBER,
                          std::to_string(static_cast<long>(n)));
         }
      } catch (const std::invalid_argument &) {
         env->get_error_cb()(error_level_e::FATAL,
                             "invalid argument for numerical conversion");
      } catch (const std::out_of_range &) {
         env->get_error_cb()(error_level_e::FATAL, "out of range");
      }
      std::exit(1);
   });

   env->get(">") = cell_t([=](const cells &c) -> cell_t {
      try {
         double n(std::stod(c[0].val.c_str()));
         for (auto i = c.begin() + 1; i != c.end(); ++i) {
            if (n <= std::stod(i->val.c_str())) {
               return false_sym;
            }
         }
         return true_sym;
      } catch (const std::invalid_argument &) {
         env->get_error_cb()(error_level_e::FATAL,
                             "invalid argument for numerical conversion");
      } catch (const std::out_of_range &) {
         env->get_error_cb()(error_level_e::FATAL, "out of range");
      }
      std::exit(1);
   });

   env->get("<") = cell_t([=](const cells &c) -> cell_t {
      try {
         double n(std::stod(c[0].val.c_str()));
         for (auto i = c.begin() + 1; i != c.end(); ++i) {
            if (n >= std::stod(i->val.c_str())) {
               return false_sym;
            }
         }
         return true_sym;
      } catch (const std::invalid_argument &) {
         env->get_error_cb()(error_level_e::FATAL,
                             "invalid argument for numerical conversion");
      } catch (const std::out_of_range &) {
         env->get_error_cb()(error_level_e::FATAL, "out of range");
      }
      std::exit(1);
   });

   env->get("<=") = cell_t([=](const cells &c) -> cell_t {
      try {
         double n(std::stod(c[0].val.c_str()));
         for (auto i = c.begin() + 1; i != c.end(); ++i) {
            if (n > std::stod(i->val.c_str())) {
               return false_sym;
            }
         }
         return true_sym;
      } catch (const std::invalid_argument &) {
         env->get_error_cb()(error_level_e::FATAL,
                             "invalid argument for numerical conversion");
      } catch (const std::out_of_range &) {
         env->get_error_cb()(error_level_e::FATAL, "out of range");
      }
      std::exit(1);
   });

   env->get(">=") = cell_t([=](const cells &c) -> cell_t {
      try {
         double n(std::stod(c[0].val.c_str()));
         for (auto i = c.begin() + 1; i != c.end(); ++i) {
            if (n < std::stod(i->val.c_str())) {
               return false_sym;
            }
         }
         return true_sym;
      } catch (const std::invalid_argument &) {
         env->get_error_cb()(error_level_e::FATAL,
                             "invalid argument for numerical conversion");
      } catch (const std::out_of_range &) {
         env->get_error_cb()(error_level_e::FATAL, "out of range");
      }
      std::exit(1);
   });
}

} // namespace polaris