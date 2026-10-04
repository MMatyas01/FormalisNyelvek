#include "dfa.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

void DFAProblem::initialize_parser(cxxopts::Options &options) {
    options.add_options()
        ("check", "Check words in the DFA", cxxopts::value<std::string>());
}

bool DFAProblem::is_chosen_problem(const cxxopts::ParseResult &args) {
    return args.count("check") > 0;
}

int DFAProblem::run(const cxxopts::ParseResult &args) {
    std::string inputFilename = args["input"].as<std::string>();
    std::string outputFilename = args["output"].as<std::string>();
    std::string words = args["check"].as<std::string>();

    std::ifstream inputFile(inputFilename);

    if (!inputFile) {
        std::cerr << "Error opening input file: " << inputFilename << std::endl;
        return 1;
    }

    std::string line;
    std::string state;

    // States
    std::getline(inputFile, line);
    std::stringstream ssStates(line);

    while (ssStates >> state) {
        states.push_back(state);
    }

    // Alphabet
    std::getline(inputFile, line);
    std::stringstream ssAlphabet(line);

    char symbol;

    while (ssAlphabet >> symbol) {
        alphabet.push_back(symbol);
    }

    // Start state
    std::getline(inputFile, line);
    std::stringstream ssStart(line);

    ssStart >> startState;

    // Final states
    std::getline(inputFile, line);
    std::stringstream ssFinal(line);

    while (ssFinal >> state) {
        finalStates.insert(state);
    }

    // Transitions
    std::string source;
    std::string destination;

    while (inputFile >> source >> symbol >> destination) {
        transitions[{source, symbol}] = destination;
    }

    // Open output file
    std::ofstream outputFile(outputFilename);

    if (!outputFile) {
        std::cerr << "Error opening output file: " << outputFilename << std::endl;
        return 1;
    }

    // Check words
    std::stringstream ssWords(words);
    std::string word;

    while (std::getline(ssWords, word, ',')) {
        std::string currentState = startState;
        bool accepted = true;

        for (char symbol : word) {
            auto it = transitions.find({currentState, symbol});

            if (it == transitions.end()) {
                accepted = false;
                break;
            }

            currentState = it->second;
        }

        if (accepted && finalStates.count(currentState) > 0) {
            outputFile << "IGEN\n";
        } else {
            outputFile << "NEM\n";
        }
    }

    return 0;
}