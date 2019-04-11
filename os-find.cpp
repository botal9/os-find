#include "dir_walker.h"

#include <cstring>
#include <iostream>
#include <vector>

WalkerConfig parseArgs(int argc, char* argv[]) {
    if (argc < 2) {
        throw std::invalid_argument("Path argument expected");
    }
    WalkerConfig config;

    config.Directory = argv[1];
    if (config.Directory.back() == '/') {
        config.Directory.pop_back();
    }
    if (argc % 2) {
        throw std::invalid_argument("Wrong arguments number");
    }
    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "-name") == 0) {
            config.Name = argv[++i];
        } else if (strcmp(argv[i], "-exec") == 0) {
            config.Exec = argv[++i];
        } else if (strcmp(argv[i], "-inum") == 0) {
            try {
                config.Inode = std::stoull(argv[++i]);
                config.HasInodeParam = true;
            } catch (std::logic_error& ) {
                throw std::invalid_argument("Wrong -inum argument");
            }
        } else if (strcmp(argv[i], "-size") == 0) {
            try {
                config.Size = std::stoull(argv[++i] + /*skip comparison symbol*/1);
            } catch (std::logic_error& ) {
                throw std::invalid_argument("Wrong -size argument");
            }
            char cmp = argv[i][0];
            switch (cmp) {
                case '+':
                    config.SzLimit = SizeLimit::GT;
                    break;
                case '-':
                    config.SzLimit = SizeLimit::LT;
                    break;
                case '=':
                    config.SzLimit = SizeLimit::EQ;
                    break;
                default:
                    throw std::invalid_argument("Wrong -size argument");
            }
        } else if (strcmp(argv[i], "-nlinks") == 0) {
            try {
                config.Nlinks = std::stoull(argv[++i]);
                config.HasNlinksParam = true;
            } catch (std::logic_error& ) {
                throw std::invalid_argument("Wrong -nlinks argument");
            }
        } else {
            throw std::invalid_argument("Unknown argument: " + std::string(argv[i]));
        }
    }
    return config;
}

int main(int argc, char* argv[]) {
    try {
        WalkerConfig config = parseArgs(argc, argv);

        DirectoryWalker walker;
        walker.SetConfig(std::move(config));
        walker.Do();
    } catch (std::invalid_argument& e) {
        std::cout << e.what() << std::endl << "Usage: os-find Path [-exec path-to-executable] "
                                              "[-inum num] [-name name] [-nlinks num] [-size [-+=]size]" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
