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
        int c6 = 0, c7 = 0, c8 = 0, c9 = 0, c10 = 0;

        // Szigorú regex a logcat sorok szétválasztására (Dátum, Idő, PID, TID, Szint, Tag, Üzenet)
        std::regex rxLine("^(\\S+)\\s+(\\S+)\\s+(\\d+)\\s+(\\d+)\\s+([IDVEFAW])\\s+([^:]+):(.*)$");
        std::smatch match;

        for (const auto& l : lines) {
            if (std::regex_search(l, match, rxLine)) {
                std::string date = match[1].str();
                std::string time = match[2].str();
                std::string pid = match[3].str();
                std::string level = match[5].str();
                std::string tag = match[6].str();
                std::string msg = match[7].str();

                // 1. Október 15
                if (date.find("10-15") != std::string::npos) c1++;

                // 2. Október 15. 10:18:51
                if (date.find("10-15") != std::string::npos && time.find("10:18:51") == 0) c2++;

                // 3. 51. másodperc
                if (time.find(":51.") != std::string::npos) c3++;

                // 4. lowmemorykiller tag
                if (tag.find("lowmemorykiller") != std::string::npos) c4++;

                // 5. 10:18:51 és 221-es PID
                if (time.find("10:18:51") == 0 && pid == "221") c5++;

                // 6. Összes E sor
                if (level == "E") c6++;

                // 7. W sor PackageManager taggel
                if (level == "W" && tag.find("PackageManager") != std::string::npos) c7++;

                // 8. D sor ExoPlayer taggel
                if (level == "D" && tag.find("ExoPlayer") != std::string::npos) c8++;

                // 9. E vagy W sor, ami .java fájlt tartalmaz
                if ((level == "E" || level == "W") && l.find(".java") != std::string::npos) c9++;

                // 10. Thread szó az üzenetben
                if (msg.find("Thread") != std::string::npos) c10++;
            }
        }
        // 10 szám kiírása egymás alá
        outputFile << c1 << "\n" << c2 << "\n" << c3 << "\n" << c4 << "\n" << c5 << "\n"
                   << c6 << "\n" << c7 << "\n" << c8 << "\n" << c9 << "\n" << c10 << "\n";

    } else if (task == 2) {
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