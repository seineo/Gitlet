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

CommandExecutor ce;

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

Gitlet setUp() {
    clearGitlet();
    Gitlet test;
    vector<string> args{"./unittest", "init"};
    ce.execCommand(test, args);
    return test;  // move ?
}

// test for the init command
void testInit() {
    cout << "start to test init" << endl;
    // there is no ".gitlet"
    Gitlet test = setUp();
    assert(fs::exists(".gitlet"));
    assert(fs::exists(".gitlet/info"));
    assert(fs::exists(".gitlet/commit"));
    assert(fs::exists(".gitlet/blob"));
    string head = test.getHead();
    assert(fs::exists(Commit::getDir() / head));
    // there is a ".gitlet"
    string expected = "A Gitlet version-control system, already exists in the "
                      "current directory";
    vector<string> args{"./unittest", "init"};
    ASSERT_THROW(ce.execCommand(test, args), runtime_error, expected);
    // check number of files or directories
    assert(clearGitlet() == 5);  // 4 directories, 1 gitlet data
    cout << "test init successfully" << endl;
}

// test for the add command
// add new file and modify content
void testAdd01() {
    cout << "start to test add 01" << endl;
    // set up
    Gitlet test = setUp();
    // add new file
    string testFile = "test.txt";
    string oldContent = "hello";
    string newContent = oldContent + "!";
    utils::writeFile(testFile, oldContent);
    vector<string> args = {"./unittest", "add", testFile};
    ce.execCommand(test, args);
    // deserialize the blob and compare the content
    string blobID = test.getStagedBlob(testFile);
    assert(!blobID.empty());
    fs::path file = Blob::getDir() / blobID;
    assert(fs::exists(file));
    Blob testBlob;
    utils::load(testBlob, file);
    assert(testBlob.getContent() == utils::readFile(testFile));
    // modify content
    utils::writeFile(testFile, newContent);
    ce.execCommand(test, args);
    string newBlobID = test.getStagedBlob(testFile);
    assert(!newBlobID.empty());
    fs::path newFile = Blob::getDir() / newBlobID;
    assert(file != newFile);  // file names aren't equal
    assert(!fs::exists(file));
    assert(fs::exists(newFile));
    utils::load(testBlob, newFile);
    assert(testBlob.getContent() == utils::readFile(testFile));
    // tear down
    assert(clearGitlet() == 6);  // 4 directories, 1 gitlet data, 1 blob
    assert(fs::remove(testFile));
    cout << "test add 01 successfully" << endl;
}

// test for add command
// file content is identical to the current commit
void testAdd02() {
    cout << "start to test add 02" << endl;
    // set up
    Gitlet test = setUp();
    string log = "test";
    string timestamp = "20:00";
    unordered_map<string, string> commitBlob;
    string parent = "par";
    string testFile = "test.txt";
    string content = "hello";
    utils::writeFile(testFile, content);
    Blob blob(content);
    string blobID = blob.getID();
    utils::save(blob, Blob::getDir() / blobID);
    commitBlob.insert({testFile, blobID});
    Commit c(log, timestamp, commitBlob, parent);
    utils::save(c, Commit::getDir() / c.getID());
    test.setHead(c.getID());
    test.insertStagedBlob(testFile, blobID);
    // run test
    vector<string> args = {"./unittest", "add", testFile};
    ce.execCommand(test, args);
    assert(test.getStagedBlob(testFile).empty());
    assert(!fs::is_empty(Blob::getDir()));
    // tear down
    assert(clearGitlet() == 7);  // 4 directories, 1 gitlet data, 1 commit and 1 blob
    assert(fs::remove(testFile));
    cout << "test add 02 successfully" << endl;
}

// test for add command
// marked removed
void testAdd03() {
    cout << "start to test add 03" << endl;
    // set up
    Gitlet test = setUp();
    string testFile = "test.txt";
    string content = "hello";
    utils::writeFile(testFile, content);
    test.insertRemovedBlob(testFile);
    // run test
    assert(test.isRemoved(testFile));
    vector<string> args = {"./unittest", "add", testFile};
    ce.execCommand(test, args);
    assert(!test.isRemoved(testFile));
    // tear down
    assert(clearGitlet() == 6);  // 4 directories, 1 gitlet data, 1 blob
    assert(fs::remove(testFile));
    cout << "test add 03 successfully" << endl;
}

// test for commit command
// staged files
void testCommit01() {
    cout << "start to test commit 01" << endl;
    // set up
    Gitlet test = setUp();
    string testFile = "test.txt";
    string content = "hello";
    utils::writeFile(testFile, content);
    vector<string> args = {"./unittest", "add", testFile};
    ce.execCommand(test, args);
    string blobID = test.getStagedBlob(testFile);
    assert(!blobID.empty());
    string oldHead = test.getHead();
    // run test
    string log = "add a test file";
    args = {"./unittest", "commit", log};
    ce.execCommand(test, args);
    string newHead = test.getHead();
    Commit cur;
    fs::path cPath = Commit::getDir() / newHead;
    assert(fs::exists(cPath));
    utils::load(cur, cPath);
    assert(cur.blobExists(blobID));
    assert(test.isStageEmpty());
    assert(cur.getParent1() == oldHead);
    assert(cur.getLog() == log);
    // tear down
    assert(clearGitlet() == 7); // 4 directories, 1 gitlet data, 1 blob and 1 commit
    assert(fs::remove(testFile));
    cout << "test commit 01 successfully" << endl;
}

// test for commit 
//

int main() {
    testInit();
    testAdd01();
    testAdd02();
    testAdd03();
    return 0;
}
