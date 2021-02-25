#ifndef GITLETOBJ_H
#define GITLETOBJ_H
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>

namespace gitlet {
namespace gitlet_obj {
class GitletObj {
public:
    std::string getID() const {return id;}
    virtual std::filesystem::path getDir() const = 0;
protected:
    GitletObj() = default;
    std::string id;
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & id;
    }

};

BOOST_SERIALIZATION_ASSUME_ABSTRACT(GitletObj);

class Gitlet : public GitletObj {
public:
    Gitlet() = default;
    void init();
    std::filesystem::path getDir() const override {return dir;}
private:
    std::string head; // head pointer
    std::string curBranch; // current branch name
    std::unordered_map<std::string, std::string> branchCommit; // mapping of branch name to commit hash
    std::unordered_set<std::string> removedBlob;  // file name of removed blob
    std::unordered_map<std::string, std::string> stagedBlob; // mapping of file name to blob hash
    static const std::filesystem::path dir;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<GitletObj>(*this);
        ar & head;
        ar & curBranch;
        ar & branchCommit;
        ar & removedBlob;
        ar & stagedBlob;
        // ar & dir;
    }

};

class Commit : public GitletObj {
public:
    Commit() = default;
    Commit(const std::string& log);
    Commit(const std::string& log, const std::string& timestamp, const std::unordered_map<std::string, std::string>& commitBlob, const std::string& parent1, const std::string& parent2 = std::string());
    std::filesystem::path getDir() const override {return dir;}
private:
    std::string log;  // log message of commit
    std::string timestamp;  // timestamp of commit
    std::unordered_map<std::string, std::string> commitBlob; // mapping of file names to blob hash
    std::string parent1; // parent1 hash
    std::string parent2; // parent2 hash
    static const std::filesystem::path dir;

    std::string getCurrentTime() const;
    std::string getEpochTime() const;
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<GitletObj>(*this);
        ar & log & timestamp & commitBlob;
        ar & parent1 & parent2 & dir;
    }
};

class Blob : public GitletObj {
    public:
        Blob() = default;
        Blob(std::string content);
        std::filesystem::path getDir() const override {return dir;}
    private:
        std::string content;
        static const std::filesystem::path dir;
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::base_object<GitletObj>(*this);
            ar & content & dir;
        }
};
}
}

#endif /* ifndef GITLETOBJ_H */
