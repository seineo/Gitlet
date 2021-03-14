#include "gitletobj.h"

#include "utils.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <stdexcept>
using namespace gitlet::gitlet_obj;
using std::ctime;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::runtime_error;
using std::string;
using std::time_t;
using std::unordered_map;
using std::vector;

namespace utils = gitlet::utils;
namespace fs = std::filesystem;

const std::filesystem::path Gitlet::dir = ".gitlet/info";
const std::filesystem::path Commit::dir = ".gitlet/commit";
const std::filesystem::path Blob::dir = ".gitlet/blob";

Gitlet git;

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
        return false;
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

bool Rm::isLegal(const std::vector<std::string> &args) const {
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

void Rm::exec(Gitlet &git, const std::vector<std::string> &args) {
    string content = utils::readFile(args[2]);
    string expectedBlobID = utils::sha1({content});
    string actualBlobID = git.getStagedBlobID(args[2]);
    string head = git.getHead();
    Commit cur;
    utils::load(cur, Commit::getDir() / head);
    if (!actualBlobID.empty()) {
        git.eraseStagedBlob(args[2]);
    } else if (cur.blobExists(expectedBlobID)) {
        git.insertRemovedBlob(args[2]);
        fs::remove(args[2]);
    } else {
        throw runtime_error("No reason to remove the file");
    }
}

Commit::Commit(const string &log) : log(log) {
    timestamp = getEpochTime();
    id = utils::sha1({log, timestamp, parent1, parent2});
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
    string timestamp = getCurrentTime();
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
