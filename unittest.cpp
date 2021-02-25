#include "gitletobj.h"
#include "utils.h"
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
namespace utils = gitlet::utils;
namespace fs = std::filesystem;
using std::string;
using std::cout;
using std::endl;
using std::runtime_error;
using namespace gitlet::gitlet_obj;

// capture the stdout output of a command
// throw a runtime_error if popen() fails
string getOutput(const char *command) {
    std::array<char, 128> buffer;
    std::string result;

    FILE *pipe = popen(command, "r");
    if (!pipe) {
        throw runtime_error("popen() failed");
    }
    while (fgets(buffer.data(), 128, pipe) != NULL) {
        result += buffer.data();
    }
    pclose(pipe);
    return result;
}

// test for the init command
void testInit() {
    cout << "start to test init" << endl;
    // remove ".gitlet" directory if it exists
    if (fs::exists(".gitlet"))
        fs::remove_all(".gitlet");
    // if there is no ".gitlet"
    const char *command = "./main init";
    system(command);
    assert(fs::exists(".gitlet"));
    assert(fs::exists(".gitlet/info"));
    assert(fs::exists(".gitlet/commit"));
    assert(fs::exists(".gitlet/blob"));
    // if there is a ".gitlet"
    string expected = "A Gitlet version-control system, already exists in the current directory";
    try {
        string actual = getOutput(command);
        assert(expected == actual);
    } catch (const runtime_error& e) {
        cout << e.what() << endl;
    }
    cout << "test init successfully" << endl;
}

int main() {
    testInit();
}
