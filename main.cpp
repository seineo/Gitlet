#include "gitletobj.h"
#include "utils.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace fs = std::filesystem;
namespace utils = gitlet::utils;
using fs::filesystem_error;
using fs::path;
using std::cout;
using std::endl;
using std::out_of_range;
using std::runtime_error;
using std::string;
using std::vector;
using namespace gitlet::gitlet_obj;
extern Gitlet git;

// parse command line arguments
void parseArgs(int argc, char *argv[]) {
    if (argc < 2) {
        throw runtime_error("Please enter a command");
    } else {
        if (fs::exists(".gitlet")) {
            path dir = git.getDir();
            const auto &entry = fs::directory_iterator(dir);
            string file = entry->path().filename();
            utils::load(git, dir / file);
        }
        vector<string> args;
        for (int i = 0; i != argc; ++i) {
            args.push_back(argv[i]);
        }
        git.execCommand(args);
        utils::save(git, git.getDir() / git.getID());
    }
}

int main(int argc, char *argv[]) {
    try {
        parseArgs(argc, argv);
        cout << argv[0] << endl;
    } catch (const filesystem_error &e1) {
        cout << e1.what();
    } catch (const out_of_range &e2) {
        cout << "Don't have that command";
    } catch (const runtime_error &e3) {
        cout << e3.what();
    }
    return 0;
}
