#include "gitletobj.h"

#include "utils.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace gitlet::gitlet_obj;
using std::cout;
using std::ctime;
using std::endl;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::runtime_error;
using std::string;
using std::time_t;
using std::unordered_map;
using std::unordered_set;
using std::vector;

namespace utils = gitlet::utils;
namespace fs = std::filesystem;

const std::filesystem::path Gitlet::dir = ".gitlet/info";
const std::filesystem::path Commit::dir = ".gitlet/commit";
const std::filesystem::path Blob::dir = ".gitlet/blob";

bool Init::isLegal(const vector<string> &args) const {
    if (fs::exists(".gitlet")) {
        throw runtime_error("A Gitlet version-control system, already exists in "
                            "the current directory");
    }
    return args.size() == 2;
}

void Init::exec(Gitlet &git, const vector<string> &args) {
    string logMessage = "initial commit";
    string branchName = "master";
    Commit initial(logMessage);
    git.insertBranchCommit(branchName, initial.getID());
    git.setCurBranch(branchName);
    git.setHead(initial.getID());
    string timestamp = initial.getTimeStamp();
    git.setID(utils::sha1(
        {logMessage, timestamp, branchName}));  // won't change any more
    fs::create_directory(".gitlet");
    fs::create_directory(".gitlet/info");
    fs::create_directory(".gitlet/commit");
    fs::create_directory(".gitlet/blob");
    utils::save(initial, Commit::getDir() / initial.getID());
}

bool Add::isLegal(const vector<string> &args) const {
    if (!fs::exists(".gitlet")) {
        throw runtime_error("Not in an initialized Gitlet directory");
    } else if (args.size() != 3) {
        return false;
    } else if (!fs::exists(args[2])) {
        throw runtime_error("File does not exist.");
    } else {
        return true;
    }
}

void Add::exec(Gitlet &git, const vector<string> &args) {
    string file = args[2];
    string content = utils::readFile(file);
    Blob blob(content);
    string id = blob.getID();
    // if marked removed, remove that mark
    git.eraseRemovedBlob(file);

    // if identical to the current commit, don't add and
    // remove the staged blob
    string head = git.getHead();
    Commit cur;
    utils::load(cur, Commit::getDir() / head);
    if (cur.blobExists(id)) {
        git.eraseStagedBlob(file);
        return;
    }
    // remove previous staged file
    string oldID = git.getStagedBlobID(file);
    if (!oldID.empty()) {
        fs::remove(Blob::getDir() / oldID);
    }
    // save staged file, may overwrite previous entry
    git.insertStagedBlob(file, id);
    utils::save(blob, Blob::getDir() / id);
}

bool CommitCmd::isLegal(const vector<string> &args) const {
    if (!fs::exists(".gitlet")) {
        throw runtime_error("Not in an initialized Gitlet directory");
    }
    if (args.size() == 3) {
        if (args[2].empty()) {
            throw runtime_error("Please enter a commit message");
        } else {
            return true;
        }
    } else {
        return false;
    }
}

void CommitCmd::exec(Gitlet &git, const vector<string> &args) {
    if (git.isStageEmpty() && git.isRemovedEmpty()) {
        throw runtime_error("No changes added to the commit");
    }
    string head = git.getHead();
    Commit cur;
    utils::load(cur, Commit::getDir() / head);
    unordered_map<string, string> stage = git.getStagedBlob();
    unordered_map<string, string> blobs = cur.getCommitBlob();
    unordered_map<string, string> commitBlob;
    for (const auto &i : blobs) {
        if (!git.isRemoved(i.first)) {
            commitBlob.insert(i);
        }
    }
    for (const auto &i : stage) {
        commitBlob[i.first] = i.second;
    }
    Commit newCommit(args[2], commitBlob, head);
    string newHead = newCommit.getID();
    string branch = git.getCurBranch();
    git.setHead(newHead);
    git.insertBranchCommit(branch, newHead);
    git.clearStagedBlob();
    utils::save(newCommit, Commit::getDir() / newHead);
}

bool Rm::isLegal(const vector<string> &args) const {
    if (!fs::exists(".gitlet")) {
        throw runtime_error("Not in an initialized Gitlet directory");
    } else if (args.size() != 3) {
        return false;
    } else if (!fs::exists(args[2])) {
        return false;
    } else {
        return true;
    }
}

void Rm::exec(Gitlet &git, const vector<string> &args) {
    string content = utils::readFile(args[2]);
    string expectedBlobID = utils::sha1({content});
    string actualBlobID = git.getStagedBlobID(args[2]);
    string head = git.getHead();
    Commit cur;
    utils::load(cur, Commit::getDir() / head);
    if (actualBlobID.empty() && !cur.blobExists(expectedBlobID)) {
        throw runtime_error("No reason to remove the file");
    } else {
        if (!actualBlobID.empty()) {
            git.eraseStagedBlob(args[2]);
        }
        if (cur.blobExists(expectedBlobID)) {
            git.insertRemovedBlob(args[2]);
            fs::remove(args[2]);
        }
    }
}

void AbstractLog::printLog(const string &id) {
    Commit cur;
    utils::load(cur, Commit::getDir() / id);
    string par1 = cur.getParent1();
    string par2 = cur.getParent2();
    cout << "===" << endl;
    cout << "commit " << id << endl;
    if (!par2.empty()) {
        cout << "Merge: " << par1.substr(0, 6) << " " << par2.substr(0, 6)
             << endl;
    }
    cout << "Date: " << cur.getTimeStamp() << endl;
    cout << cur.getLog() << endl;
    cout << endl;
}

bool Log::isLegal(const vector<string> &args) const {
    if (!fs::exists(".gitlet")) {
        throw runtime_error("Not in an initialized Gitlet directory");
    }
    return args.size() == 2;
}

void Log::exec(Gitlet &git, const vector<string> &args) {
    string id = git.getHead();
    Commit cur;
    while (!id.empty()) {
        printLog(id);
        utils::load(cur, Commit::getDir() / id);
        id = cur.getParent1();
    }
}

void GlobalLog::addCommits(const string &id) { commits.insert(id); }

bool GlobalLog::isVisited(const string &id) { return commits.count(id); }

bool GlobalLog::isLegal(const vector<string> &args) const {
    if (!fs::exists(".gitlet")) {
        throw runtime_error("Not in an initialized Gitlet directory");
    }
    return args.size() == 2;
}

void GlobalLog::exec(Gitlet &git, const vector<string> &args) {
    string id;
    unordered_map<string, string> branchCommit = git.getBranchCommit();
    Commit cur;
    for (const auto &i : branchCommit) {
        id = i.second;
        while (!id.empty()) {
            if (!isVisited(id)) {
                addCommits(id);
                printLog(id);
                utils::load(cur, Commit::getDir() / id);
                id = cur.getParent1();
            } else {
                break;
            }
        }
    }
}

vector<string> Status::toVector(const unordered_map<string, string> &m) {
    vector<string> v;
    for (const auto &i : m) {
        v.push_back(i.first);
    }
    return v;
}

vector<string> Status::toVector(const unordered_set<string> &s) {
    vector<string> v;
    for (const auto &i : s) {
        v.push_back(i);
    }
    return v;
}

bool Status::isLegal(const std::vector<std::string> &args) const {
    if (!fs::exists(".gitlet")) {
        throw runtime_error("Not in an initialized Gitlet directory");
    }
    return args.size() == 2;
}

Commit::Commit(const string &log) : log(log) {
    timestamp = getEpochTime();
    id = utils::sha1({log, timestamp, parent1, parent2});
}

void Status::exec(Gitlet &git, const std::vector<std::string> &args) {
    // print branches
    unordered_map<string, string> branchCommit = git.getBranchCommit();
    vector<string> bc = toVector(branchCommit);
    sort(bc.begin(), bc.end());
    string curBranch = git.getCurBranch();
    cout << "=== Branches ===" << endl;
    for (const auto &i : bc) {
        if (i == curBranch) {
            cout << "*";
        }
        cout << i << endl;
    }
    cout << endl;
    // print staged files
    cout << "=== Staged Files" << endl;
    unordered_map<string, string> stagedBlob = git.getStagedBlob();
    vector<string> sb = toVector(stagedBlob);
    sort(sb.begin(), sb.end());
    for (const auto &i : sb) {
        cout << i << endl;
    }
    cout << endl;
    // print removed files
    unordered_set<string> removedBlob = git.getRemovedBlob();
    vector<string> rb = toVector(removedBlob);
    sort(rb.begin(), rb.end());
    for (const auto &i : rb) {
        cout << i << endl;
    }
    // print modified but not staged files
    cout << "=== Modifications Not Staged For Commit ===" << endl;
    string deleted = " (deleted)";
    string modified = " (modified)";
    vector<string> modifiedNotStaged;
    // staged but modified or deleted
    for (const auto &i : stagedBlob) {
        if (!fs::exists(i.first)) {  // deleted
            modifiedNotStaged.push_back(i.first + deleted);
        } else {
            Blob blob(utils::readFile(i.first));
            if (blob.getID() != i.second) {
                modifiedNotStaged.push_back(i.first + modified);
            }
        }
    }
    // tracked but modified & not staged  or deleted
    Commit cur;
    string head = git.getHead();
    utils::load(cur, Commit::getDir() / head);
    unordered_map<string, string> commitBlob = cur.getCommitBlob();
    for (const auto &i : commitBlob) {
        if (!fs::exists(i.first)) {  // deleted
            modifiedNotStaged.push_back(i.first + deleted);
        } else {
            Blob blob(utils::readFile(i.first));
            if (blob.getID() != commitBlob[i.first] &&
                stagedBlob.find(i.first) == stagedBlob.end()) {
                modifiedNotStaged.push_back(i.first + modified);
            }
        }
    }
    sort(modifiedNotStaged.begin(), modifiedNotStaged.end());
    for (const auto &i : modifiedNotStaged) {
        cout << i << endl;
    }
    cout << endl;
    // print untracked files
    cout << "=== Untracked Files ===" << endl;
    vector<string> untracked;
    for (auto &iter : fs::directory_iterator(".")) {
        if (fs::is_regular_file(iter.path())) {
            string file = iter.path().filename();
            if (stagedBlob.find(file) == stagedBlob.end() &&
                commitBlob.find(file) == commitBlob.end()) {
                untracked.push_back(file);
            }
        }
    }
    sort(untracked.begin(), untracked.end());
    for (const auto &i : untracked) {
        cout << i << endl;
    }
}

bool Checkout::isLegal(const vector<string> &args) const {
    if (!fs::exists(".gitlet")) {
        throw runtime_error("Not in an initialized Gitlet directory");
    }
    int sz = args.size();
    if (sz == 4) {
        return args[2] == "--";
    } else if (sz == 5) {
        return args[3] == "--" && args[2].size() >= 6 && args[2].size() <= 40;
    }
    return sz == 3;
}

void Checkout::exec(Gitlet &git, const vector<string> &args) {
    int sz = args.size();
    string id;
    if (sz == 3) {  // with branch name
        // check untracked files
        for (auto &iter : fs::directory_iterator(".")) {
            string file = iter.path().filename();
            if (fs::is_regular_file(file) && isUntracked(git, file)) {
                throw runtime_error("Threr is an untracked file in the way; "
                                    "delete it or add it first");
            }
        }
        // check branch name
        string branchName = args[sz - 1];
        if (branchName == git.getCurBranch()) {
            throw runtime_error("No need to checkout the current branch.");
        }
        id = git.getBranchCommitID(branchName);
        if (id.empty()) {
            throw runtime_error("No such branch exists");
        }
        git.clearStagedBlob();         // clear the staging area
        git.setCurBranch(branchName);  // update current branch
        if (id == git.getHead()) {     // if it's head commit, then no need to
                                       // change files
            return;
        }
        git.setHead(id);  // update head ref
        takeCommitFiles(id);
    } else {  // with commit id
        // check whether the given file is untracked
        string file = args[sz - 1];
        if (fs::is_regular_file(file) && isUntracked(git, file)) {
            throw runtime_error("Threr is an untracked file in the way; delete "
                                "it or add it first");
        }
        id = (sz == 4 ? git.getHead() : args[2]);
        if (id.size() < 40) {
            id = getTotalID(id);
        }
        takeCommitFile(id, file);
    }
}

// given a shortened commit id, return a total id
string Checkout::getTotalID(string id) {
    fs::path dir = Commit::getDir();
    int sz = id.size();
    string file;
    for (auto &iter : fs::directory_iterator(dir)) {
        file = iter.path().filename();
        if (file.substr(0, sz) == id) {
            return file;
        }
    }
    throw runtime_error("No commit with that id exists.");
}

// check whether the given regular file is untracked
bool Checkout::isUntracked(Gitlet &git, string file) {
    Commit cur;
    string head = git.getHead();
    utils::load(cur, Commit::getDir() / head);
    unordered_map<string, string> stagedBlob = git.getStagedBlob();
    unordered_map<string, string> commitBlob = cur.getCommitBlob();
    if (stagedBlob.find(file) == stagedBlob.end() &&
        commitBlob.find(file) == commitBlob.end()) {
        return true;
    }
    return false;
}

// given commit id, clear the current files and take the version of files that
// exist in the given commit id
void Checkout::takeCommitFiles(string id) {
    Commit c;
    utils::load(c, Commit::getDir() / id);
    unordered_map<string, string> commitBlob = c.getCommitBlob();
    Blob blob;
    // clear files in current working directory
    for (auto &iter : fs::directory_iterator(".")) {
        if (fs::is_regular_file(iter.path())) {
            fs::remove(iter.path());
        }
    }
    // take the files in the given commit
    for (const auto &i : commitBlob) {
        utils::load(blob, Blob::getDir() / i.second);
        utils::writeFile(i.first, blob.getContent());
    }
}

void Checkout::takeCommitFile(string id, string file) {
    fs::path cpath = Commit::getDir() / id;
    if (!fs::exists(cpath)) {
        throw runtime_error("No commit with that id exists.");
    }
    Commit c;
    utils::load(c, cpath);
    Blob blob;
    string blobID = c.getBlobID(file);
    if (blobID.empty()) {
        throw runtime_error("File does not exist in that commit.");
    }
    utils::load(blob, Blob::getDir() / blobID);
    utils::writeFile(file, blob.getContent());
}

bool Branch::isLegal(const vector<string> &args) const {
    if (!fs::exists(".gitlet")) {
        throw runtime_error("Not in an initialized Gitlet directory");
    }
    return args.size() == 3;
}

void Branch::exec(Gitlet &git, const vector<string> &args) {
    if (!git.getBranchCommitID(args[2]).empty()) {
        throw runtime_error("A branch with that name already exists.");
    }
    git.insertBranchCommit(args[2], git.getHead());
}

Commit::Commit(const string &log,
               const unordered_map<string, string> &commitBlob,
               const string &parent1,
               const string &parent2)
    : log(log), commitBlob(commitBlob), parent1(parent1), parent2(parent2) {
    string blobRef;
    for (const auto &i : commitBlob) {
        blobRef.append(i.second);
    }
    timestamp = getCurrentTime();
    id = utils::sha1({log, timestamp, blobRef, parent1, parent2});
}

string Commit::getCurrentTime() const {
    const auto curTime =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    return string(ctime(&curTime));
}

string Commit::getEpochTime() const {
    const auto p0 = std::chrono::time_point<std::chrono::system_clock>{};
    time_t epoch_time = std::chrono::system_clock::to_time_t(p0);
    return string(ctime(&epoch_time));
}

Blob::Blob(string content) : content(content) { id = utils::sha1({content}); }
