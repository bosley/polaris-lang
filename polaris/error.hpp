#ifndef POLARIS_ERROR_HPP
#define POLARIS_ERROR_HPP

#include <functional>

namespace polaris {

enum class error_level_e { FATAL, FAILURE };

using error_cb_f = std::function<void(error_level_e, const char *)>;

} // namespace polaris

#endif