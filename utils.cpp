#include "utils.h"

#include <cryptopp/files.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include <cryptopp/simple.h>
#include <fstream>
#include <stdexcept>
using namespace gitlet::utils;
using fs::file_size;
using fs::path;
using std::ifstream;
using std::initializer_list;
using std::ofstream;
using std::runtime_error;
using std::string;

string sha1(initializer_list<string> il) {
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

string readFile(path file) {
    ifstream is(file);
    string content;
    const auto size = file_size(file);
    content.resize(size, '\0');  // buffer
    is.read(content.data(), size);
    return content;
}

void writeFile(path file, string content) {
    ofstream os(file);
    os << content;
}
