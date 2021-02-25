#include <iostream>
#include <filesystem>
#include <stdexcept>
#include "gitletobj.h"
#include "utils.h"
namespace fs = std::filesystem;
namespace utils = gitlet::utils;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using fs::path;
using fs::filesystem_error;
using std::runtime_error;
using namespace gitlet::gitlet_obj;
  
constexpr unsigned int hashStr(const char* s, int off = 0) {
    return !s[off] ? 5831 : (hashStr(s, off+1) * 33) ^ s[off];
}

// check whether arguments are legal
bool isLegal(int argc, char *argv[]) {
    switch(hashStr(argv[1])) {
        case hashStr("init"):
        case hashStr("log"):
        case hashStr("global-log"):
        case hashStr("status"):
            return argc == 2;
        case hashStr("add"):
        case hashStr("commit"):
        case hashStr("rm"):
        case hashStr("find"):
        case hashStr("branch"):
        case hashStr("rm-branch"):
        case hashStr("reset"):
        case hashStr("merge"):
            return argc == 3;
        case hashStr("checkout"):
            return argc >= 2 && argc <= 4;
        default:
            throw runtime_error("No command with that name exists");
    }
}

void checkWorkingDir() {
    if (!fs::exists(".gitlet")) {
        throw runtime_error("Not in an initialized Gitlet directory");
    }
}

// parse command line arguments
void parseArgs(int argc, char *argv[]) {
    if (argc < 2) {
        throw runtime_error("Please enter a command");
    } else {
        if(!isLegal(argc, argv)) {
            throw runtime_error("Incorrect operands");
        } else {
            Gitlet gitlet;
            if (fs::exists(".gitlet")) {
                path dir = gitlet.getDir();
                const auto & entry = fs::directory_iterator(dir);
                string file = entry->path().filename();
                utils::load(gitlet, dir / file);
           }
            switch (hashStr(argv[1])) {
                case hashStr("init"):
                    gitlet.init();       
                    break;
                default:
                    break;
            }
            utils::save(gitlet, gitlet.getDir() / gitlet.getID());
        }
    } 
}

int main(int argc, char *argv[])
{
    try {
        parseArgs(argc, argv);
    } catch (const filesystem_error& e1) {
        cout << e1.what();
    } catch (const runtime_error& e2) {
        cout << e2.what();
    }     
    return 0;
}
