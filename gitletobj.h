#ifndef GITLETOBJ_H
#define GITLETOBJ_H
#include <string>
#include <unordered_map>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

namespace gitlet {
namespace gitlet_obj {
class GitletObj {
    friend class boost::serialization::access;
public:
    virtual void serialize() = 0;
    virtual void deserialize(std::string file) = 0;
    std::string getID() const {return id;}
protected:
    GitletObj();
    std::string id;
private:
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};
    
BOOST_SERIALIZATION_ASSUME_ABSTRACT(AbstractStr);

class Commit : public GitletObj {
	friend class boost::serialization::access;
public:
    Commit() = default;
    Commit(const std::string& log);
    Commit(const std::string& log, const std::string& timestamp, const std::unordered_map<std::string, std::string>& commitBlob, const std::string& parent1, const std::string& parent2 = std::string());
    void serialize() override;
    void deserialize(std::string file) override;
private:
    std::string log;  // log message of commit
    std::string timestamp;  // timestamp of commit
    std::unordered_map<std:string, std::string> commitBlob; // mapping of file names to blob hash
    std::string parent1; // parent1 hash
    std::string parent2; // parent2 hash

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
    
    std::string getCurrentTime() const;
    std::string getEpochTime() const;
    
};

class Blob : public GitletObj {
	friend class boost::serialization::access;
    public:
        Blob() = default;
        Blob(std::string content);
        void serialize() override;
        void deserialize(std::string file) override;
    private:
        std::string content;

        template<class Archive>
        void serialize(Archive & ar, const unsigned int version);
};
}
}

#endif /* ifndef GITLETOBJ_H */
