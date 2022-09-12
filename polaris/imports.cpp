#include "imports.hpp"

#include "evaluator.hpp"
#include "feeder.hpp"
#include "polaris.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace polaris {

imports_c::imports_c(evaluator_c &eval,
                     std::shared_ptr<environment_c> environment,
                     const std::vector<std::string> &include_directories)
    : _evaluator(eval), _environment(environment),
      _include_directories(include_directories) {}

void imports_c::import(const std::string &file) {

   std::size_t idx = 0;
   std::filesystem::path file_path(file);
   while (!std::filesystem::is_regular_file(file_path)) {

      if (idx >= _include_directories.size()) {
         std::cerr << "File not found : " << file << std::endl;
         std::exit(EXIT_FAILURE);
      }

      file_path = _include_directories[idx];
      file_path /= file;
      idx++;
   }

   if (_imported.contains(file_path.string())) {
      return;
   }

   read_file(file_path.string());
   _imported.insert(file_path.string());
}

void imports_c::read_file(const std::string &path) {

   std::fstream fs;
   fs.open(path, std::fstream::in);
   if (!fs.is_open()) {
      std::cerr << "Unable to open file : " << path << std::endl;
      std::exit(EXIT_FAILURE);
   }

   std::string line;
   feeder_c feeder(_evaluator, _environment);
   while (std::getline(fs, line)) {
      feeder.feed(line);
   }
}

} // namespace polaris