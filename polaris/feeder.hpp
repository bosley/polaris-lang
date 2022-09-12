#ifndef POLARIS_FEEDER_HPP
#define POLARIS_FEEDER_HPP

#include "environment.hpp"
#include "evaluator.hpp"
#include <memory>
#include <string>

namespace polaris {

//! \brief A simple feeder object that makes sure that
//!        messy user input is massaged into something
//!        nice and formal that can be accepted into
//!        the system
class feeder_c {
 public:
   //! \brief Create the feeder
   //! \param evaluator The evaluator to use
   //! \param env The environment to use in execution
   feeder_c(polaris::evaluator_c &evaluator,
            std::shared_ptr<polaris::environment_c> env);

   //! \brief Feed the line into the system.
   //! \returns true iff a statement was submitted
   bool feed(std::string &line, bool print_result = false);

 private:
   polaris::evaluator_c &_eval;
   std::shared_ptr<polaris::environment_c> _env;
   uint64_t _tracker{0};
   std::string _statement;
};

} // namespace polaris

#endif