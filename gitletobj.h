#ifndef GITLETOBJ_H
#define GITLETOBJ_H
#include <string>
#include <unordered_map>
#include <filesystem>

namespace gitlet {
namespace gitlet_obj {
class GitletObj {
public:
    std::string getID() const {return id;}
protected:
    GitletObj();
    std::string id;
};
    
class Commit : public GitletObj {
public:
    Commit() = default;
    Commit(const std::string& log);
    Commit(const std::string& log, const std::string& timestamp, const std::unordered_map<std::string, std::string>& commitBlob, const std::string& parent1, const std::string& parent2 = std::string());
private:
    std::string log;  // log message of commit
    std::string timestamp;  // timestamp of commit
    std::unordered_map<std:string, std::string> commitBlob; // mapping of file names to blob hash
    std::string parent1; // parent1 hash
    std::string parent2; // parent2 hash
    static std::filesystem::path dir;

    std::string getCurrentTime() const;
    std::string getEpochTime() const;
    
};
std::filesystem::path Commit::dir = ".gitlet/commit";

class Blob : public GitletObj {
    public:
        Blob() = default;
        Blob(std::string content);
    private:
        std::string content;
        static std::filesystem::path dir;
};
std::filesystem::path Blob::dir = ".gitlet/blob";
}
}

#endif /* ifndef GITLETOBJ_H */
