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

template<class Archive>
void GitletObj::serialize(Archive & ar, const unsigned int version) {
    ar & id;
}

Commit::Commit(const string& log):log(log) {
    timestamp = getEpochTime();
    id = utils::sha1({log, timestamp});
}

Commit::Commit(const string& log, const string& timestamp, const unordered_map<string, string>& commitBlob, const string& parent1, const string& parent2):log(log), timestamp(timestamp), commitBlob(commitBlob), parent1(parent1), parent2(parent2) {}

template<class Archive>
void Commit::serialize(Archive & ar, const unsigned int version) {
    ar & boost::serialization::base_object<GitletObj>(*this);
    ar & log;
    ar & timestamp;
    ar & commitBlob;
    ar & parent1;
    ar & parent2;
}

void Commit::serialize() {
     string file = id;
     ofstream ofs(file, ios::binary);
     boost::archive::binary_oarchive oa(ofs);
     oa << (*this);
}

void Commit::deserialize(string file) {
     ifstream ifs(file, ios::binary);
     boost::archive::binary_iarchive ia(ifs);
     ia >> (*this);
}


void Blob::serialize() {
     string file = id;
     ofstream ofs(file, ios::binary);
     boost::archive::binary_oarchive oa(ofs);
     oa << (*this);
}

void Blob::deserialize(string file) {
     ifstream ifs(file, ios::binary);
     boost::archive::binary_iarchive ia(ifs);
     ia >> (*this);
}

