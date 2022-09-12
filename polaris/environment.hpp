#ifndef POLARIS_ENVIRONMENT_HPP
#define POLARIS_ENVIRONMENT_HPP

#include "cell.hpp"
#include <memory>
#include <vector>

namespace polaris {

//! \brief Execution environment
class environment_c {
 public:
   //! \brief Construct a base environment
   environment_c();

   //! \brief Construct an environment with an "outer" parent environemnt
   //!        that it can use to locate symbols
   environment_c(std::shared_ptr<environment_c> outer);

   //! \brief Construct an environment wiht specific data baked into it
   //!        from the getgo
   //! \param params Names of incoming data
   //! \param args Value of incoming datas
   environment_c(const cells &params, const cells &args,
                 std::shared_ptr<environment_c> outer);

   //! \brief Find an environment variable given the name
   //!        If the item can not be found in the current environment
   //!        Then the outer environments will be checked - If the item
   //!        does not exist std::exit will be called
   cell_t::map &find(const std::string &var);

   //! \brief Operator [] overload for accessing environment variables
   //! \param var The variable to retrieve
   cell_t &operator[](const std::string &var);

   //! \brief Get accessor for grabbing things through the shared pointer
   //! \param var The variable to retrieve
   cell_t &get(const std::string &value);

 private:
   cell_t::map _env;
   std::shared_ptr<environment_c> _outer;
};

} // namespace polaris

#endif