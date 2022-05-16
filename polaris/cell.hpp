#ifndef POLARIS_CELL_HPP
#define POLARIS_CELL_HPP

#include "fwd.hpp"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace polaris {

//! \brief General cell types
enum class cell_type_e { SYMBOL, NUMBER, LIST, PROC, LAMBDA, STRING };

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

using cells = std::vector<cell_t>; //! Shorthand for vector of cells
const cell_t false_sym(cell_type_e::SYMBOL, "#f"); //! Cell for "FALSE"
const cell_t true_sym(cell_type_e::SYMBOL, "#t");  //! Cell for "TRUE"
const cell_t nil(cell_type_e::SYMBOL, "nil");      //! Cell for "NIL"

} // namespace polaris

#endif