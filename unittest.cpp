#include "gitletobj.h"
#include "utils.h"

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
namespace utils = gitlet::utils;
namespace fs = std::filesystem;
using std::cout;
using std::endl;
using std::ofstream;
using std::runtime_error;
using std::string;
using std::uintmax_t;
using std::unordered_map;
using std::vector;
using namespace gitlet::gitlet_obj;

// assertion to check that expcetion is thrown
#define ASSERT_THROW(expression, exceptionType, expected)                       \
    {                                                                           \
        bool exceptionThrown = false;                                           \
        try {                                                                   \
            expression;                                                         \
        } catch (const exceptionType &e) {                                      \
            exceptionThrown = true;                                             \
            string actual = e.what();                                           \
            assert(expected == actual);                                         \
        } catch (...) {                                                         \
            exceptionThrown = true;                                             \
        }                                                                       \
        assert(exceptionThrown);                                                \
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
    // there is no ".gitlet"
    Gitlet test;
    vector<string> args{"./unittest", "init"};
    test.execCommand(args);
    assert(fs::exists(".gitlet"));
    assert(fs::exists(".gitlet/info"));
    assert(fs::exists(".gitlet/commit"));
    assert(fs::exists(".gitlet/blob"));
    // there is a ".gitlet"
    string expected = "A Gitlet version-control system, already exists in the "
                      "current directory";
    ASSERT_THROW(test.execCommand(args), runtime_error, expected);
    // check number of files or directories
    assert(clearGitlet() == 4);
    cout << "test init successfully" << endl;
}

// test for the add command
void testAdd() {
    cout << "start to test add" << endl;
    clearGitlet();
    Gitlet test;
    vector<string> args{"./unittest", "init"};
    test.execCommand(args);  // initialize
    // add new file
    string testFile = "test.txt";
    string oldContent = "hello";
    string newContent = oldContent + "!";
    ofstream ofs(testFile);
    if (!ofs.is_open()) {
        throw runtime_error("cannot open test file");
    }
    ofs << oldContent;
    args = {"./unittest", "add", testFile};
    test.execCommand(args);
    unordered_map<string, string> stagedBlob = test.getStagedBlob();
    assert(stagedBlob.find(testFile) != stagedBlob.end());
    assert(stagedBlob.size() == 1);
    // deserialize the blob and compare the content
    fs::path file = Blob::getDir() / stagedBlob[testFile];
    assert(fs::exists(file));
    Blob testBlob;
    utils::load(testBlob, file);
    assert(testBlob.getContent() == utils::readFile(file));
    // modify content
    ofs << newContent;
    test.execCommand(args);
    fs::path newFile = Blob::getDir() / stagedBlob[testFile];
    assert(file != newFile);  // file names aren't equal
    utils::load(testBlob, newFile);
    assert(testBlob.getContent() == utils::readFile(newFile));
}

int main() {
    testInit();
    // testAdd();
    return 0;
}
