#ifndef OS_FIND_DIR_WALKER_H
#define OS_FIND_DIR_WALKER_H

#include <string>

enum class SizeLimit {
    EQ = 0,
    LT,
    GT,
    UNDEFINED
};

struct WalkerConfig {
    std::string Directory;
    std::string Exec;
    std::string Name;
    ino_t Inode;
    nlink_t Nlinks;
    uint64_t Size;

    SizeLimit SzLimit = SizeLimit::UNDEFINED;
    bool HasInodeParam = false;
    bool HasNlinksParam = false;
};

class DirectoryWalker {
public:
    void SetConfig(WalkerConfig&& config);

    void Do();

private:
    WalkerConfig Config;

    void Do(const std::string& dirName);

    bool IsMeetRequirements(const std::string &file, const struct stat& fileStat);
};


#endif //OS_FIND_DIR_WALKER_H
