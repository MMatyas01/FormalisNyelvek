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
    std::string inputFilename  = args["input"].as<std::string>();
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
        int c[10] = {0};

        // 1. Október 15-ei sorok
        std::regex rx1(R"(^10-15\s+)");

        // 2. Október 15. 10:18:51
        std::regex rx2(R"(^10-15\s+10:18:51\.\d{3}\s+)");

        // 3. Bármikor :51 másodperc
        std::regex rx3(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:51\.\d{3}\s+)");

        // 4. lowmemorykiller tag
        std::regex rx4(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d{3}\s+\d+\s+\d+\s+[IDVEFAW]\s+lowmemorykiller:)");

        // 5. 10:18:51-kor, 221-es PID
        std::regex rx5(R"(^10-15\s+10:18:51\.\d{3}\s+221\s+)");

        // 6. Összes E sor
        std::regex rx6(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d{3}\s+\d+\s+\d+\s+E\s+)");

        // 7. W + PackageManager
        std::regex rx7(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d{3}\s+\d+\s+\d+\s+W\s+PackageManager:)");

        // 8. D + ExoPlayer (JAVÍTVA: megengedi, hogy a tag hosszabb legyen, pl. ExoPlayerImpl:)
        std::regex rx8(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d{3}\s+\d+\s+\d+\s+D\s+ExoPlayer[^:]*:)");

        // 9. E/W sor, .java: előfordulással
        std::regex rx9(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d{3}\s+\d+\s+\d+\s+[EW]\s+.*\.java:)");

        // 10. "Thread" szó a message részben (JAVÍTVA: kivettük a \b szóhatárokat)
        std::regex rx10(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d{3}\s+\d+\s+\d+\s+[IDVEFAW]\s+[^:]+:.*Thread)");

        std::regex* rxs[10] = {
            &rx1, &rx2, &rx3, &rx4, &rx5,
            &rx6, &rx7, &rx8, &rx9, &rx10
        };

        for (const auto& l : lines) {
            for (int i = 0; i < 10; ++i) {
                if (std::regex_search(l, *rxs[i])) {
                    c[i]++;
                }
            }
        }

        for (int i = 0; i < 10; ++i) {
            outputFile << c[i] << "\n";
        }

        // === DEBUG (opcionális, törölhető, ha már átment) ===
        std::cerr << "\n[DEBUG] Szamitott ertekek: ";
        for (int i = 0; i < 10; ++i) {
            std::cerr << c[i] << " ";
        }
        std::cerr << "\n[DEBUG] Vart ertekek:      15554 2816 2825 10 1 3196 104 54 645 244\n\n";

    } else if (task == 2) {
        std::regex rxLevelE(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d{3}\s+\d+\s+\d+\s+E\s+)");
        std::regex rxJava(R"(([A-Za-z0-9_$]+\.java))");

        for (const auto& l : lines) {
            if (std::regex_search(l, rxLevelE)) {
                auto begin = std::sregex_iterator(l.begin(), l.end(), rxJava);
                auto end   = std::sregex_iterator();
                for (auto i = begin; i != end; ++i) {
                    outputFile << (*i).str(1) << "\n";
                }
            }
        }

    } else if (task == 3) {
        std::regex rxPID(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d{3}\s+(\d+))");
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
        std::regex rxE(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d{3}\s+(\d+)\s+\d+\s+E\s+)");
        std::map<std::string, int> errCount;

        for (const auto& l : lines) {
            std::smatch match;
            if (std::regex_search(l, match, rxE)) {
                errCount[match.str(1)]++;
            }
        }

        std::string bestPID;
        int maxErr = -1;
        for (const auto& pair : errCount) {
            if (pair.second > maxErr) {
                maxErr  = pair.second;
                bestPID = pair.first;
            }
        }
        if (maxErr != -1) {
            outputFile << "PID=" << bestPID << " (" << maxErr << " errors)\n";
        }

    } else if (task == 5) {
        std::regex rxMainThreadE(R"(^\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d{3}\s+(\d+)\s+\1\s+E\s+)");
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