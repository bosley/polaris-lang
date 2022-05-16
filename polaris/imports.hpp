#ifndef POLARIS_IMPORTS_HPP
#define POLARIS_IMPORTS_HPP

#include "fwd.hpp"

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace polaris
{

//! \brief Import helper
class imports_c {
public:
  //! \brief Construct the importer with the evaluator that
  // will be used to
  imports_c(evaluator_c &eval, std::shared_ptr<environment_c> environment,
            const std::vector<std::string> &include_directories);

  //! \brief Attempt to import a file - If its already imported nothing will
  //! happen,
  //!        if the file can't be found as-is it the include directories will be
  //!        checked iteratively. The first file found matching the name will be
  //!        imported and added to the set of imported files
  //! \param file The file to import
  void import(const std::string &file);

private:
  evaluator_c &_evaluator;
  std::shared_ptr<environment_c> _environment;
  std::set<std::string> _imported;
  std::vector<std::string> _include_directories;

  void read_file(const std::string &path);
};

}

#endif