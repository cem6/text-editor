#ifndef FILEHANDLING_H
#define FILEHANDLING_H

#include "settings.h"
#include <fstream>

class Filehandling {
private:
    std::string path;

public:
    std::vector<std::string> textVec;

    Filehandling(std::string s) : path{s} {
        std::cout << "filehandling constructor" << std::endl;

        getFile();
    }

    void getFile() {
        std::ifstream instream(path);
        if (!instream.is_open()) {
            std::cerr << "failed to open file" << std::endl;
            return;
        }

        std::string line;
        while (std::getline(instream, line)) {
            textVec.push_back(line);
        }
        instream.close();

        // segfault when opening empty file
        if (textVec.empty()) textVec.push_back("");
    }

    void updateFile() {
        std::ofstream outstream(path);
        if (!outstream.is_open()) {
            std::cerr << "failed to open file" << std::endl;
            return;
        }
        for (const std::string &line : textVec)
            outstream << line << '\n';
        outstream.close();
    }

};

#endif