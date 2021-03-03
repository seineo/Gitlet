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

bool Init::isLegal(vector<string> args) const { return args.size() == 2; }

void Init::exec() {
    if (fs::exists(".gitlet")) {
        throw runtime_error("A Gitlet version-control system, already exists in "
                            "the current directory");
    }
    string logMessage = "initial commit";
    string branchName = "master";
    Commit initial(logMessage);
    auto &branchCommit = git.getBranchCommit();
    auto &curBranch = git.getCurBranch();
    auto &head = git.getHead();
    branchCommit.insert({branchName, initial.getID()});
    curBranch = branchName;
    head = initial.getID();
    auto &id = git.getID();
    id = utils::sha1({head, curBranch});  // won't change any more
    fs::create_directory(".gitlet");
    fs::create_directory(".gitlet/info");
    fs::create_directory(".gitlet/commit");
    fs::create_directory(".gitlet/blob");
}

bool Add::isLegal(vector<string> args) const {
    return args.size() == 3;  // 还有 args[2]所指文件必须存在
}

void Add::exec() {}

// void Gitlet::init() {
// if (fs::exists(".gitlet")) {
// throw runtime_error("A Gitlet version-control system, already exists in the
// current directory");
// }
// id = utils::sha1({head, curBranch}); // won't change any more
// string logMessage = "initial commit";
// string branchName = "master";
// Commit initial(logMessage);
// branchCommit.insert({branchName, initial.getID()});
// curBranch = branchName;
// head = initial.getID();
// fs::create_directory(".gitlet");
// fs::create_directory(".gitlet/info");
// fs::create_directory(".gitlet/commit");
// fs::create_directory(".gitlet/blob");
// }

Commit::Commit(const string &log) : log(log) {
    timestamp = getEpochTime();
    id = utils::sha1({log, timestamp});
}

Commit::Commit(const string &log,
               const string &timestamp,
               const unordered_map<string, string> &commitBlob,
               const string &parent1,
               const string &parent2)
    : log(log)
    , timestamp(timestamp)
    , commitBlob(commitBlob)
    , parent1(parent1)
    , parent2(parent2) {
    string blobRef;
    for (const auto &i : commitBlob) {
        blobRef.append(i.second);
    }
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
