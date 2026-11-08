#include "rgx.hpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <map>
#include <set>

void RgxProblem::initialize_parser(cxxopts::Options &options) {
    options.add_options()
        ("rgx", "Regex problem task number", cxxopts::value<int>());
}

bool RgxProblem::is_chosen_problem(const cxxopts::ParseResult &args) {
    return args.count("rgx") > 0;
}

int RgxProblem::run(const cxxopts::ParseResult &args) {
    std::string inputFilename = args["input"].as<std::string>();
    std::string outputFilename = args["output"].as<std::string>();
    int task = args["rgx"].as<int>();

    std::ifstream inputFile(inputFilename);
    if (!inputFile) {
        std::cerr << "Error opening input file: " << inputFilename << std::endl;
        return 1;
    }

    std::ofstream outputFile(outputFilename);
    if (!outputFile) {
        std::cerr << "Error opening output file: " << outputFilename << std::endl;
        return 1;
    }

    std::string line;
    std::vector<std::string> lines;
    while (std::getline(inputFile, line)) {
        lines.push_back(line);
    }

    if (task == 1) {
        int c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0;

        // 1. Október 15-én létrejött sorok
        std::regex rx1("^\\s*10-15\\s+");

        // 2. Október 15. 10:18:51 (pont és ezredmásodperc opcionális)
        std::regex rx2("^\\s*10-15\\s+10:18:51(?:\\.\\d+)?\\s+");

        // 3. Bármikor 51 másodperckor
        std::regex rx3("^\\s*\\S+\\s+\\d{1,2}:\\d{2}:51(?:\\.\\d+)?\\s+");

        // 4. lowmemorykiller tag (a szint után jön, kettőspont vagy szóköz követi)
        std::regex rx4("\\s+[IDVEFAW]\\s+lowmemorykiller(?::|\\s)");

        // 5. 10:18:51-kor, 221-es process (PID a 3. oszlopban)
        std::regex rx5("^\\s*\\S+\\s+10:18:51(?:\\.\\d+)?\\s+221\\s+\\d+\\s+");

        for (const auto& l : lines) {
            if (std::regex_search(l, rx1)) c1++;
            if (std::regex_search(l, rx2)) c2++;
            if (std::regex_search(l, rx3)) c3++;
            if (std::regex_search(l, rx4)) c4++;
            if (std::regex_search(l, rx5)) c5++;
        }
        outputFile << c1 << "\n" << c2 << "\n" << c3 << "\n" << c4 << "\n" << c5 << "\n";

    } else if (task == 2) {
        // 2. feladat: Csak a hibasorok (E) .java fájljainak kinyerése
        std::regex rxLevelE("^\\S+\\s+\\S+\\s+\\d+\\s+\\d+\\s+E\\s+");
        std::regex rxJava("([a-zA-Z0-9_]+\\.java)");

        for (const auto& l : lines) {
            if (std::regex_search(l, rxLevelE)) {
                auto begin = std::sregex_iterator(l.begin(), l.end(), rxJava);
                auto end = std::sregex_iterator();
                for (std::sregex_iterator i = begin; i != end; ++i) {
                    outputFile << (*i).str(1) << "\n";
                }
            }
        }

    } else if (task == 3) {
        // 3. feladat: Egyedi PID-ek sorrendben
        std::regex rxPID("^\\S+\\s+\\S+\\s+(\\d+)");
        std::set<std::string> seen;

        for (const auto& l : lines) {
            std::smatch match;
            if (std::regex_search(l, match, rxPID)) {
                std::string pid = match.str(1);
                if (seen.find(pid) == seen.end()) {
                    seen.insert(pid);
                    outputFile << pid << "\n";
                }
            }
        }

    } else if (task == 4) {
        // 4. feladat: A legtöbb E-t tartalmazó PID
        std::regex rxE("^\\S+\\s+\\S+\\s+(\\d+)\\s+\\d+\\s+E\\s+");
        std::map<std::string, int> errCount;

        for (const auto& l : lines) {
            std::smatch match;
            if (std::regex_search(l, match, rxE)) {
                errCount[match.str(1)]++;
            }
        }

        std::string bestPID = "";
        int maxErr = -1;
        for (const auto& pair : errCount) {
            if (pair.second > maxErr) {
                maxErr = pair.second;
                bestPID = pair.first;
            }
        }
        if (maxErr != -1) {
            outputFile << "PID=" << bestPID << " (" << maxErr << " errors)\n";
        }

    } else if (task == 5) {
        // 5. feladat: Fő szálon (PID == TID) történt E-k száma
        std::regex rxMainThreadE("^\\S+\\s+\\S+\\s+(\\d+)\\s+\\1\\s+E\\s+");
        int count = 0;

        for (const auto& l : lines) {
            if (std::regex_search(l, rxMainThreadE)) {
                count++;
            }
        }
        outputFile << count << "\n";
    }

    return 0;
}