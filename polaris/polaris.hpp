#ifndef POLARIS_LANG_HPP
#define POLARIS_LANG_HPP

#include <string>
#include <memory>

#include "cell.hpp"
#include "evaluator.hpp"
#include "environment.hpp"
#include "imports.hpp"

namespace polaris {

//! \brief Take a given string and process it down
//!        down to the cell_t level
//! \param s The string to process in
extern cell_t read(const std::string &str);

//! \brief Convert a given cell to a string
//! \param exp The cell to convert
extern std::string to_string(const cell_t &exp);

//! \brief Add the basic sumbols to a given environment
//! \param env The environment to load the symbols into
extern void add_globals(std::shared_ptr<environment_c> env,
                        imports_c &imports);

} // namespace polaris

#endif