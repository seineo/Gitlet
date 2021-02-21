#ifndef UTILS_H
#define UTILS_H
#include <filesystem>
#include <initializer_list>
#include <string>
namespace fs = std::filesystem;

namespace gitlet {
namespace utils {
// compute hash for list of messages
std::string sha1(std::initializer_list<std::string> il);
// read an entire file into a std::string, if cannot open
// the file, throw a runtime_error
std::string readFile(fs::path file);
// write the content into a file, if cannot open
// the file, throw a runtime_error
void writeFile(fs::path file, std::string content);
}  // namespace utils
}  // namespace gitlet

#endif /* ifndef UTILS_H */
