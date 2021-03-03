#ifndef UTILS_H
#define UTILS_H
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <stdexcept>
#include <string>

namespace gitlet {
namespace utils {
// serialize object into file, if cannot open
// the file, throw a runtime_error
template <typename T>
void save(const T &obj, const std::filesystem::path &file) {
    std::ofstream ofs(file, std::ios::binary);
    if (!ofs.is_open()) {
        throw std::runtime_error("cannot open the file");
    }
    boost::archive::binary_oarchive oa(ofs);
    oa << obj;
}

// deserialize object from file, if cannot open
// the file, throw a runtime_error
template <typename T>
void load(T &obj, const std::filesystem::path &file) {
    std::ifstream ifs(file, std::ios::binary);
    if (!ifs.is_open()) {
        throw std::runtime_error("cannot open the file");
    }
    boost::archive::binary_iarchive ia(ifs);
    ia >> obj;
}
// compute hash for list of messages
std::string sha1(std::initializer_list<std::string> il);
// read an entire file into a std::string, if cannot open
// the file, throw a runtime_error
std::string readFile(const std::filesystem::path &file);
// write the content into a file, if cannot open
// the file, throw a runtime_error
void writeFile(const std::filesystem::path &file, std::string content);
}  // namespace utils
}  // namespace gitlet

#endif /* ifndef UTILS_H */
