#ifndef POLARIS_EVALUATOR_HPP
#define POLARIS_EVALUATOR_HPP

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "fwd.hpp"

namespace polaris {

//! \brief Evaluator
class evaluator_c {
public:
  //! \brief Construct the base evaluator with the standard
  //!         callable symbols baked into it
  evaluator_c();

  //! \brief Evaluate a cell given and environment
  //! \param x The cell to evaluate
  //! \param env The environment to use in the evaluation
  cell_t evaluate(cell_t x, std::shared_ptr<environment_c> env);

private:
  std::unordered_map<std::string, std::function<cell_t(
                                      cell_t, std::shared_ptr<environment_c>)>>
      _callable_symbol_table;
};
} // namespace polaris

#endif