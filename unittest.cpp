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
using std::uintmax_t;
using namespace gitlet::gitlet_obj;

// assertion to check that expcetion is thrown
#define ASSERT_THROW(expression, exceptionType, expected) {\
    bool exceptionThrown = false;\
    try {\
        expression;\
    } catch (const exceptionType& e) {\
       exceptionThrown = true;\
       string actual = e.what();\
       assert(expected == actual);\
    } catch (...) {\
       exceptionThrown = true;\
    }\
    assert(exceptionThrown);\
}


// clear .gitlet directory, return the number of files or directory deleted
uintmax_t clearGitlet() {
    uintmax_t n = 0;
    if (fs::exists(".gitlet")) {
        n = fs::remove_all(".gitlet");
    }
    return n;
    
}

// test for the init command
void testInit() {
    cout << "start to test init" << endl;
    // remove ".gitlet" directory if it exists
    clearGitlet();
    // if there is no ".gitlet"
    Gitlet test;
    test.init();
    assert(fs::exists(".gitlet"));
    assert(fs::exists(".gitlet/info"));
    assert(fs::exists(".gitlet/commit"));
    assert(fs::exists(".gitlet/blob"));
    // if there is a ".gitlet"
    string expected = "A Gitlet version-control system, already exists in the current directory";
    ASSERT_THROW(test.init(), runtime_error, expected);
    // check number of files or directories
    assert(clearGitlet() == 4);
    cout << "test init successfully" << endl;
}

int main() {
    testInit();
}
