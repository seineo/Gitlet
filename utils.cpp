#include "utils.h"

#include <cryptopp/files.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include <cryptopp/simple.h>
#include <fstream>
#include <stdexcept>
namespace fs = std::filesystem;
namespace utils = gitlet::utils;
using fs::file_size;
using fs::path;
using std::ifstream;
using std::initializer_list;
using std::ofstream;
using std::runtime_error;
using std::string;

string utils::sha1(initializer_list<string> il) {
    using namespace CryptoPP;
    string message, id, digest;
    for (const auto &str : il) {
        message.append(str);
    }
    SHA1 hash;
    hash.Update((const byte *)message.data(), message.size());
    digest.resize(hash.DigestSize());
    hash.Final((byte *)&digest[0]);
    HexEncoder encoder(new StringSink(id));
    StringSource(digest, true, new Redirector(encoder));
    return id;
}

string utils::readFile(const path& file) {
    ifstream is(file);
    if (!is.is_open()) {
        throw runtime_error("cannot open the file");
    }
    string content;
    const auto size = file_size(file);
    content.resize(size, '\0');  // buffer
    is.read(content.data(), size);
    return content;
}

void utils::writeFile(const path& file, string content) {
    ofstream os(file);
    if (!os.is_open()) {
        throw runtime_error("cannot open the file");
    }
    os << content;
}
