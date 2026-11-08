#include "mini.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <queue>
#include <set>
#include <map>
#include <vector>
#include <string>
#include <climits>

void MiniProblem::initialize_parser(cxxopts::Options &options) {
    options.add_options()
        ("mini", "Minimize the finite automaton");
}

bool MiniProblem::is_chosen_problem(const cxxopts::ParseResult &args) {
    return args.count("mini") > 0;
}

// Segédfüggvény az állapotok sorszámának kinyeréséhez (pl. "q5" -> 5)
int MiniProblem::extract_number(const std::string& s) {
    std::string numStr = "";
    for (char c : s) {
        if (std::isdigit(c)) numStr += c;
    }
    if (numStr.empty()) return 0;
    return std::stoi(numStr);
}

int MiniProblem::run(const cxxopts::ParseResult &args) {
    std::string inputFilename = args["input"].as<std::string>();
    std::string outputFilename = args["output"].as<std::string>();

    std::ifstream inputFile(inputFilename);
    if (!inputFile) {
        std::cerr << "Error opening input file: " << inputFilename << std::endl;
        return 1;
    }

    states.clear();
    alphabet.clear();
    finalStates.clear();
    transitions.clear();
    startState.clear();

    std::string line, state;
    char symbol;

    // Beolvasás
    std::getline(inputFile, line);
    std::stringstream ssStates(line);
    while (ssStates >> state) states.push_back(state);

    std::getline(inputFile, line);
    std::stringstream ssAlphabet(line);
    while (ssAlphabet >> symbol) alphabet.push_back(symbol);

    std::getline(inputFile, line);
    std::stringstream ssStart(line);
    ssStart >> startState;

    std::getline(inputFile, line);
    std::stringstream ssFinal(line);
    while (ssFinal >> state) finalStates.insert(state);

    std::string source, destination;
    while (inputFile >> source >> symbol >> destination) {
        if (std::find(states.begin(), states.end(), source) == states.end() ||
            std::find(states.begin(), states.end(), destination) == states.end() ||
            std::find(alphabet.begin(), alphabet.end(), symbol) == alphabet.end()) {
            std::cerr << "Invalid transition: " << source << " " << symbol << " " << destination << std::endl;
            return 1;
        }
        transitions[{source, symbol}] = destination;
    }
    inputFile.close();

    // 1. Elérhetetlen állapotok kiszűrése (BFS a kezdőállapotból)
    std::set<std::string> reachable;
    std::queue<std::string> q;
    q.push(startState);
    reachable.insert(startState);

    while (!q.empty()) {
        std::string curr = q.front();
        q.pop();
        for (char sym : alphabet) {
            auto it = transitions.find({curr, sym});
            if (it != transitions.end()) {
                std::string dest = it->second;
                if (reachable.find(dest) == reachable.end()) {
                    reachable.insert(dest);
                    q.push(dest);
                }
            }
        }
    }

    // Csak az elérhető állapotokat tartjuk meg
    std::vector<std::string> activeStates;
    for (const auto& s : states) {
        if (reachable.count(s)) activeStates.push_back(s);
    }

    // 2. Kezdeti partíció: Végállapotok és Nem-végállapotok
    std::set<std::string> p_final, p_non_final;
    for (const auto& s : activeStates) {
        if (finalStates.count(s)) p_final.insert(s);
        else p_non_final.insert(s);
    }

    std::vector<std::set<std::string>> partitions;
    if (!p_final.empty()) partitions.push_back(p_final);
    if (!p_non_final.empty()) partitions.push_back(p_non_final);

    // 3. Partíció finomítás (Moore algoritmus)
    bool changed = true;
    while (changed) {
        changed = false;
        std::vector<std::set<std::string>> new_partitions;

        for (const auto& part : partitions) {
            // Csoportosítás az átmenetek alapján
            std::map<std::vector<int>, std::set<std::string>> signature_groups;

            for (const auto& state : part) {
                std::vector<int> signature;
                for (char sym : alphabet) {
                    auto it = transitions.find({state, sym});
                    if (it != transitions.end()) {
                        std::string dest = it->second;
                        // Megkeressük, hogy a célállapot melyik partícióban van
                        for (size_t i = 0; i < partitions.size(); ++i) {
                            if (partitions[i].count(dest)) {
                                signature.push_back(i);
                                break;
                            }
                        }
                    } else {
                        signature.push_back(-1); // Nincs átmenet
                    }
                }
                signature_groups[signature].insert(state);
            }

            // Ha egy partíció szétesik több csoportra, akkor változás történt
            for (auto& pair : signature_groups) {
                new_partitions.push_back(pair.second);
            }
            if (signature_groups.size() > 1) changed = true;
        }
        partitions = new_partitions;
    }

    // 4. Új állapotnevek kiosztása (a legkisebb számú állapot alapján)
    std::map<std::string, std::string> newStateNames;
    std::vector<std::pair<int, std::set<std::string>>> namedPartitions;

    for (const auto& part : partitions) {
        int min_num = INT_MAX;
        for (const auto& s : part) {
            int num = extract_number(s);
            if (num < min_num) min_num = num;
        }
        namedPartitions.push_back({min_num, part});
    }

    // Rendezzük a partíciókat a legkisebb számuk alapján
    std::sort(namedPartitions.begin(), namedPartitions.end());

    for (const auto& p : namedPartitions) {
        std::string newName = "s" + std::to_string(p.first);
        for (const auto& oldState : p.second) {
            newStateNames[oldState] = newName;
        }
    }

    // 5. Kiírás
    std::ofstream outputFile(outputFilename);
    if (!outputFile) {
        std::cerr << "Error opening output file: " << outputFilename << std::endl;
        return 1;
    }

    // Állapotok kiírása
    for (size_t i = 0; i < namedPartitions.size(); ++i) {
        if (i > 0) outputFile << " ";
        outputFile << "s" << namedPartitions[i].first;
    }
    outputFile << "\n";

    // Ábécé kiírása
    for (size_t i = 0; i < alphabet.size(); ++i) {
        if (i > 0) outputFile << " ";
        outputFile << alphabet[i];
    }
    outputFile << "\n";

    // Kezdőállapot
    outputFile << newStateNames[startState] << "\n";

    // Végállapotok
    std::set<std::string> newFinalStates;
    for (const auto& part : namedPartitions) {
        bool isFinal = false;
        for (const auto& s : part.second) {
            if (finalStates.count(s)) {
                isFinal = true;
                break;
            }
        }
        if (isFinal) {
            newFinalStates.insert("s" + std::to_string(part.first));
        }
    }

    bool firstFinal = true;
    for (const auto& fs : newFinalStates) {
        if (!firstFinal) outputFile << " ";
        outputFile << fs;
        firstFinal = false;
    }
    outputFile << "\n";

    // Átmenetek kiírása (csak létező átmenetek, ábécé sorrendben)
    for (const auto& p : namedPartitions) {
        std::string sourceName = "s" + std::to_string(p.first);
        std::string representative = *p.second.begin(); // Egy képviselő az eredeti partícióból

        for (char sym : alphabet) {
            auto it = transitions.find({representative, sym});
            if (it != transitions.end()) {
                std::string destName = newStateNames[it->second];
                outputFile << sourceName << " " << sym << " " << destName << "\n";
            }
        }
    }

    return 0;
}