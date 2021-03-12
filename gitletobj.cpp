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

bool Init::isLegal(const vector<string> &args) const { return args.size() == 2; }

void Init::exec(Gitlet &git, const vector<string> &args) {
    if (fs::exists(".gitlet")) {
        throw runtime_error("A Gitlet version-control system, already exists in "
                            "the current directory");
    }
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
    if (args.size() != 3) {
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
        string stagedID = git.getStagedBlob(file);
        if (!stagedID.empty()) {
            git.eraseStagedBlob(id);
        }
        return;
    }
    // remove previous staged file
    string oldID = git.getStagedBlob(file);
    if (!oldID.empty()) {
        fs::remove(Blob::getDir() / oldID);
    }
    // save staged file, may overwrite previous entry
    git.insertStagedBlob(file, id);
    utils::save(blob, Blob::getDir() / id);
}

Commit::Commit(const string &log) : log(log) {
    timestamp = getEpochTime();
    id = utils::sha1({log, timestamp, parent1, parent2});
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
