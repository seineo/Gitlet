#ifndef GITLETOBJ_H
#define GITLETOBJ_H
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace gitlet {
namespace gitlet_obj {
class GitletObj {
  public:
    std::string &getID() { return id; }

  protected:
    GitletObj() = default;
    std::string id;

  private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &id;
    }
};

class Command {
  public:
    virtual void exec() = 0;
    virtual bool isLegal(std::vector<std::string>) const = 0;
};

class Init : public Command {
  public:
    void exec() override;
    bool isLegal(std::vector<std::string>) const override;
};

class Add : public Command {
  public:
    void exec() override;
    bool isLegal(std::vector<std::string>) const override;
};

// BOOST_SERIALIZATION_ASSUME_ABSTRACT(GitletObj);

class Gitlet : public GitletObj {
  public:
    Gitlet() {
        ptrCommand.insert({"init", std::unique_ptr<Command>(new Init())});
        ptrCommand.insert({"add", std::unique_ptr<Command>(new Add())});
    }
    void execCommand(std::vector<std::string> args) {
        std::string command = args[1];
        if (ptrCommand.at(command)->isLegal(args)) {
            ptrCommand.at(command)->exec();
        } else {
            throw std::runtime_error("command is illegal");
        }
    }
    void add(std::string file);
    static std::filesystem::path getDir() { return dir; }
    std::string &getHead() { return head; }
    std::string &getCurBranch() { return curBranch; }
    std::unordered_map<std::string, std::string> &getBranchCommit() {
        return branchCommit;
    }
    std::unordered_set<std::string> &getRemovedBlob() { return removedBlob; }
    std::unordered_map<std::string, std::string> &getStagedBlob() {
        return stagedBlob;
    }
    void setHead(std::string h) { head = h; }
    void setCurBranch(std::string cb) { curBranch = cb; }
    void setBranchCommit(std::unordered_map<std::string, std::string> bc) {
        branchCommit = bc;
    }
    void setRemovedBlob(std::unordered_set<std::string> rb) { removedBlob = rb; }

  private:
    std::string head;       // head pointer
    std::string curBranch;  // current branch name
    std::unordered_map<std::string, std::string>
        branchCommit;  // mapping of branch name to commit hash
    std::unordered_set<std::string> removedBlob;  // file name of removed blob
    std::unordered_map<std::string, std::string>
        stagedBlob;  // mapping of file name to blob hash
    std::unordered_map<std::string, std::unique_ptr<Command>> ptrCommand;

    static const std::filesystem::path dir;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<GitletObj>(*this);
        ar &head;
        ar &curBranch;
        ar &branchCommit;
        ar &removedBlob;
        ar &stagedBlob;
    }
};

class Commit : public GitletObj {
  public:
    Commit() = default;
    Commit(const std::string &log);
    Commit(const std::string &log,
           const std::string &timestamp,
           const std::unordered_map<std::string, std::string> &commitBlob,
           const std::string &parent1,
           const std::string &parent2 = std::string());
    static std::filesystem::path getDir() { return dir; }

  private:
    std::string log;        // log message of commit
    std::string timestamp;  // timestamp of commit
    std::unordered_map<std::string, std::string>
        commitBlob;       // mapping of file names to blob hash
    std::string parent1;  // parent1 hash
    std::string parent2;  // parent2 hash
    static const std::filesystem::path dir;

    std::string getCurrentTime() const;
    std::string getEpochTime() const;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<GitletObj>(*this);
        ar &log &timestamp &commitBlob;
        ar &parent1 &parent2;
    }
};

class Blob : public GitletObj {
  public:
    Blob() = default;
    Blob(std::string content);
    std::string getContent() const { return content; }
    static std::filesystem::path getDir() { return dir; }

  private:
    std::string content;
    static const std::filesystem::path dir;
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<GitletObj>(*this);
        ar &content;
    }
};
}  // namespace gitlet_obj
}  // namespace gitlet

#endif /* ifndef GITLETOBJ_H */
