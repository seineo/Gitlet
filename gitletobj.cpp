#include "gitletobj.h"
#include "utils.h"
#include <fstream>
using namespace gitlet::gitlet_obj;
using std::string;
using std::unordered_map;
using std::ifstream;
using std::ofstream;
using std::ios;
namespace utils = gitlet::utils;

Commit::Commit(const string& log):log(log) {
    timestamp = getEpochTime();
    id = utils::sha1({log, timestamp});
}

Commit::Commit(const string& log, const string& timestamp, const unordered_map<string, string>& commitBlob, const string& parent1, const string& parent2):log(log), timestamp(timestamp), commitBlob(commitBlob), parent1(parent1), parent2(parent2) {
    string blobRef;
    for (const auto& i : commitBlob) {
        blobRef.append(i.second);
    }
    id = utils::sha1({log, timestamp, blobRef, parent1, parent2});
}

Blob::Blob(string content):content(content) {
    id = utils::sha1({content});
}
