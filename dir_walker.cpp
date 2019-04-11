#include "dir_walker.h"

#include <cassert>
#include <cstring>
#include <dirent.h>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <wait.h>

static void execute(const std::vector<std::string>& args) {
    std::vector<char*> args_ptrs;
    args_ptrs.reserve(args.size());
    for (const std::string& arg : args) {
        args_ptrs.push_back(const_cast<char*>(arg.data()));
    }

    char** argv = args_ptrs.data();

    switch (pid_t pid = fork()) {
        case -1:
            std::cerr << "Can't fork: " << strerror(errno) << std::endl;
            break;
        case 0:
            if (execve(argv[0], argv, environ) == -1) {
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        default:
            int exit_code;
            if (waitpid(pid, &exit_code, 0) == EXIT_FAILURE) {
                std::cerr << "Execution failed: " << strerror(errno) << std::endl;
            } else {
                std::cout << "Executed. Return code: " << WEXITSTATUS(exit_code) << std::endl;
            }
    }
}

bool DirectoryWalker::IsMeetRequirements(const std::string& fileName, const struct stat& fileStat) {
    if (Config.HasInodeParam && Config.Inode != fileStat.st_ino) {
        return false;
    }
    if (Config.HasNlinksParam && Config.Nlinks != fileStat.st_nlink) {
        return false;
    }
    if (!Config.Name.empty() && Config.Name != fileName) {
        return false;
    }
    if (Config.SzLimit == SizeLimit::EQ && fileStat.st_size != Config.Size ||
        Config.SzLimit == SizeLimit::GT && fileStat.st_size <= Config.Size ||
        Config.SzLimit == SizeLimit::LT && fileStat.st_size >= Config.Size) {
        return false;
    }
    return true;
}

void DirectoryWalker::SetConfig(WalkerConfig&& config) {
    Config = std::move(config);
}

void DirectoryWalker::Do() {
    std::string directory = Config.Directory;
    assert(!directory.empty());
    Do(directory);
}

void DirectoryWalker::Do(const std::string& dirName) {
    auto dirWrapper = opendir(dirName.c_str());
    if (dirWrapper == nullptr) {
        return;
    }
    struct stat fileStat;
    while (auto file = readdir(dirWrapper)) {
        char* fileName = file->d_name;
        if (!fileName || !strcmp(fileName, ".") || !strcmp(fileName, "..")) {
            continue;
        }
        std::string filePath = dirName + "/" + fileName;

        switch (file->d_type) {
            case DT_DIR:
                Do(filePath);
                break;
            case DT_REG:
                if (stat(filePath.c_str(), &fileStat) != 0) {
                    std::cerr << "Can't get info about " << filePath << ": " << strerror(errno) << std::endl;
                    break;
                }
                if (!IsMeetRequirements(fileName, fileStat)) {
                    break;
                }
                if (!Config.Exec.empty()) {
                    execute({Config.Exec, filePath});
                } else {
                    std::cout << filePath << std::endl;
                }
                break;
            default:
                // skip
                break;
        }
    }
    closedir(dirWrapper);
}
