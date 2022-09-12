#include "environment.hpp"

#include <iostream>

namespace polaris {

environment_c::environment_c(error_cb_f cb) : _outer(nullptr), _error_cb(cb) {}

environment_c::environment_c(std::shared_ptr<environment_c> outer)
    : _outer(outer) {}

environment_c::environment_c(const cells &params, const cells &args,
                             std::shared_ptr<environment_c> outer)
    : _outer(outer) {
   auto arg = args.begin();
   for (auto param = params.begin(); param != params.end(); ++param) {
      _env[param->val] = *arg++;
   }
}

cell_t::map &environment_c::find(const std::string &var) {
   if (_env.find(var) != _env.end()) {
      return _env;
   }
   if (_outer) {
      return _outer->find(var);
   }

   std::string err = "Unbound symbol : [" + var + "]";
   _error_cb(error_level_e::FATAL, err.c_str());
   std::exit(1);
}

cell_t &environment_c::operator[](const std::string &var) { return _env[var]; }

cell_t &environment_c::get(const std::string &value) { return _env[value]; }

} // namespace polaris