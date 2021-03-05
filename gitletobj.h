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
#include <cassert>
namespace gitlet {
namespace gitlet_obj {
class GitletObj {
  public:
    void setID(std::string _id) { id = _id; }
    std::string getID() const { return id; }

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

class Gitlet;

class Command {
  public:
    virtual void exec(Gitlet&, const std::vector<std::string>&) = 0;
    virtual bool isLegal(const std::vector<std::string>&) const = 0;
    virtual ~Command() = default;
};

class Init : public Command {
  public:
    void exec(Gitlet&, const std::vector<std::string>&) override;
    bool isLegal(const std::vector<std::string>&) const override;
};

class Add : public Command {
  public:
    void exec(Gitlet&, const std::vector<std::string>&) override;
    bool isLegal(const std::vector<std::string>&) const override;
};

// BOOST_SERIALIZATION_ASSUME_ABSTRACT(GitletObj);

class Gitlet : public GitletObj {
  public:
    Gitlet() {
        ptrCommand.insert({"init", std::unique_ptr<Command>(new Init())});
        ptrCommand.insert({"add", std::unique_ptr<Command>(new Add())});
    }
    void execCommand(const std::vector<std::string>& args) {
        std::string command = args[1];
        if (ptrCommand.find(command) != ptrCommand.end() &&
            ptrCommand.at(command)->isLegal(args)) {
            ptrCommand.at(command)->exec(*this, args);
        } else {
            throw std::runtime_error("command is illegal");
        }
    }
    static std::filesystem::path getDir() { return dir; }
    std::string getHead() const { return head; }
    std::string getCurBranch() const { return curBranch; }
    // get commit hash using branch name, if not exists, return an empty string
    std::string getBranchCommit(std::string branch) {
        if (branchCommit.find(branch) != stagedBlob.end()) {
            return stagedBlob.at(branch);
        } else {
            return {};
        }
    }
    // get blob hash using file name, if not exists, return an empty string
    std::string getStagedBlob(std::string file) {
        if (stagedBlob.find(file) != stagedBlob.end()) {
            return stagedBlob.at(file);
        } else {
            return {};
        }
    }
    void insertBranchCommit(std::string branch, std::string id) {
        branchCommit[branch] = id;
    }
    void eraseBranchCommit(std::string branch) {
        auto iter = branchCommit.find(branch);
        if (iter != branchCommit.end()) {
            branchCommit.erase(iter);
        }
    }
    void insertRemovedBlob(std::string file) { removedBlob.insert(file); }
    void eraseRemovedBlob(std::string file) {
        auto iter = removedBlob.find(file);
        if (iter != removedBlob.end()) {
            removedBlob.erase(iter);
        }
    }
    bool isRemoved(std::string file) {
        return removedBlob.find(file) != removedBlob.end();
    }

    void insertStagedBlob(std::string file, std::string id) {
        stagedBlob[file] = id;
    }
    // erase a staged blob using blob hash
    void eraseStagedBlob(std::string id) {
        for (auto iter = stagedBlob.begin(); iter != stagedBlob.end(); ++iter) {
            if (iter->second == id) {
                stagedBlob.erase(iter);
                break;
            }
        }
    }
    void setHead(std::string h) { head = h; }
    void setCurBranch(std::string cb) { curBranch = cb; }

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
    explicit Commit(const std::string &log);
    explicit Commit(
        const std::string &log,
        const std::string &timestamp,
        const std::unordered_map<std::string, std::string> &commitBlob,
        const std::string &parent1,
        const std::string &parent2 = std::string());
    std::string getLog() const { return log; }
    std::string getTimeStamp() const { return timestamp; }
    bool blobExists(std::string id) {
        for (auto iter = commitBlob.begin(); iter != commitBlob.end(); ++iter) {
            if (iter->second == id) {
                return true;
            }
        }
        return false;
    }
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
