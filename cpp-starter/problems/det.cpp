
#include "det.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <queue>
#include <set>
#include <map>
#include <vector>
#include <string>

void DetProblem::initialize_parser(cxxopts::Options &options) {
    options.add_options()
        ("det", "Determinize the finite automaton");
}

bool DetProblem::is_chosen_problem(const cxxopts::ParseResult &args) {
    return args.count("det") > 0;
}

int DetProblem::run(const cxxopts::ParseResult &args) {
    std::string inputFilename = args["input"].as<std::string>();
    std::string outputFilename = args["output"].as<std::string>();

    std::ifstream inputFile(inputFilename);

    if (!inputFile) {
        std::cerr << "Error opening input file: "
                  << inputFilename << std::endl;
        return 1;
    }

    states.clear();
    alphabet.clear();
    finalStates.clear();
    transitions.clear();
    startState.clear();

    std::string line;
    std::string state;
    char symbol;

    // Read states
    std::getline(inputFile, line);
    std::stringstream ssStates(line);

    while (ssStates >> state) {
        states.push_back(state);
    }

    // Read alphabet
    std::getline(inputFile, line);
    std::stringstream ssAlphabet(line);

    while (ssAlphabet >> symbol) {
        alphabet.push_back(symbol);
    }

    // Read start state
    std::getline(inputFile, line);
    std::stringstream ssStart(line);
    ssStart >> startState;

    // Read final states
    std::getline(inputFile, line);
    std::stringstream ssFinal(line);

    while (ssFinal >> state) {
        finalStates.insert(state);
    }

    // Read and validate transitions
    std::string source;
    std::string destination;

    while (inputFile >> source >> symbol >> destination) {
        if (std::find(states.begin(), states.end(), source) == states.end() ||
            std::find(states.begin(), states.end(), destination) == states.end() ||
            std::find(alphabet.begin(), alphabet.end(), symbol) == alphabet.end()) {
            std::cerr << "Invalid transition: "
                      << source << " " << symbol << " "
                      << destination << std::endl;
            return 1;
        }

        transitions[{source, symbol}].insert(destination);
    }

    inputFile.close();

    // Each DFA state represents a set of original states.
    using StateSet = std::set<std::string>;

    std::vector<StateSet> dfaStates;
    std::map<StateSet, std::string> stateNames;
    std::map<std::pair<std::string, char>, std::string> dfaTransitions;
    std::set<std::string> dfaFinalStates;
    std::queue<StateSet> pendingStates;

    StateSet initialSet = {startState};

    dfaStates.push_back(initialSet);
    stateNames[initialSet] = "s0";
    pendingStates.push(initialSet);

    // Process every reachable DFA state.
    while (!pendingStates.empty()) {
        StateSet currentSet = pendingStates.front();
        pendingStates.pop();

        std::string currentName = stateNames[currentSet];

        // A DFA state is final if it contains an original final state.
        for (const std::string &current : currentSet) {
            if (finalStates.count(current) > 0) {
                dfaFinalStates.insert(currentName);
                break;
            }
        }

        // Process symbols in the input alphabet order.
        for (char currentSymbol : alphabet) {
            StateSet nextSet;

            for (const std::string &current : currentSet) {
                auto it = transitions.find({current, currentSymbol});

                if (it != transitions.end()) {
                    nextSet.insert(it->second.begin(), it->second.end());
                }
            }

            // No transition: omit this edge.
            if (nextSet.empty()) {
                continue;
            }

            // Give each new state set a unique name.
            if (stateNames.find(nextSet) == stateNames.end()) {
                std::string newName =
                    "s" + std::to_string(dfaStates.size());

                stateNames[nextSet] = newName;
                dfaStates.push_back(nextSet);
                pendingStates.push(nextSet);
            }

            dfaTransitions[{currentName, currentSymbol}] =
                stateNames[nextSet];
        }
    }

    // Write the resulting DFA.
    std::ofstream outputFile(outputFilename);

    if (!outputFile) {
        std::cerr << "Error opening output file: "
                  << outputFilename << std::endl;
        return 1;
    }

    // State names in creation order.
    for (std::size_t i = 0; i < dfaStates.size(); ++i) {
        if (i > 0) {
            outputFile << " ";
        }
        outputFile << "s" << i;
    }
    outputFile << "\n";

    // Alphabet
    for (std::size_t i = 0; i < alphabet.size(); ++i) {
        if (i > 0) {
            outputFile << " ";
        }
        outputFile << alphabet[i];
    }
    outputFile << "\n";

    // Initial state
    outputFile << "s0\n";

    // Final states in state creation order.
    bool firstFinal = true;

    for (std::size_t i = 0; i < dfaStates.size(); ++i) {
        std::string name = "s" + std::to_string(i);

        if (dfaFinalStates.count(name) > 0) {
            if (!firstFinal) {
                outputFile << " ";
            }
            outputFile << name;
            firstFinal = false;
        }
    }
    outputFile << "\n";

    // Transitions: source state, alphabet symbol, destination state.
    for (std::size_t i = 0; i < dfaStates.size(); ++i) {
        std::string sourceName = "s" + std::to_string(i);

        for (char currentSymbol : alphabet) {
            auto it = dfaTransitions.find({sourceName, currentSymbol});

            if (it != dfaTransitions.end()) {
                outputFile << sourceName << " "
                           << currentSymbol << " "
                           << it->second << "\n";
            }
        }
    }

    return 0;
}
